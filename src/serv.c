/*
	krakws
	Written by Mike Perron (2012-2013)

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

	krakws is the actual web server portion of Kraknet. It manages HTTP and CGI
	interactions and allows Kraknet to be run without bootstrapping into Apache
	with a bash CGI script.
*/
#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "http11.h"
#include "generic.h"
#include "conf.h"

#define KWS_DEFAULT_PORT 9001

char state=1;
void request_timeout(int);
FILE *request_stream=NULL;

int main(int argc, char**argv){
	int sockfd, sockfd_client, client_length;
	struct sockaddr_in socket_addr_client;
	struct sockaddr_in socket_addr;
	int port=KWS_DEFAULT_PORT;

	struct passwd *gpasswd=NULL;
	FILE *client_stream=NULL;
	pid_t pid;

	int optval=2;
	int request_count=0;

	char *web_root, *cmp_web_root;
	char *buf=calloc(256, sizeof(char));
	char *method, *uri, *http_standard;
	char *a, *query=NULL;
	char *str, *s, **v;
	size_t n;

	// MT POST band-aid
	size_t post_length;
	char *post_raw_data;

	// Reads server config file and sets environment variables.
	if(set_env_from_conf())
		return error_code(-1, "Configuration problem, bailing out.");

	// Log startup
	error_code(0, "--");
	error_code(0, "Server is starting up.");

	// Get a port number from argv if provided.
	if(argc>1 && !(port=atoi(*(argv+1))))
		port=KWS_DEFAULT_PORT;

	// Tell CGI scripts about the server they're running on.
	setenv("SERVER_SOFTWARE", KWS_SERVER_NAME, 1);
	setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
	// SERVER_NAME is set by init_ws

	// Figure out what user we should run as.
	if((str=getenv("web_user_name"))&&(*str)){
		if(!(gpasswd=getpwnam(str))){
			error_code(0, "Warning: No user named %s!", str);
			gpasswd=NULL;
		}
		unsetenv("web_user_name");
	} else {
		gpasswd=getpwuid(getuid());
		error_code(0, "Warning: $web_user_name not set! Running as %s.", gpasswd->pw_name);
		gpasswd=NULL;
	}

	// Fork the process and start the server.
	if(!(pid=fork())){
		// Acquire INET socket.
		if((sockfd=socket(AF_INET, SOCK_STREAM, 0))==-1)
			return error_code(1, "Socket not got.");

		memset(&socket_addr, 0, sizeof(socket_addr));
		socket_addr.sin_family=AF_INET;
		socket_addr.sin_port=htons(port);
		socket_addr.sin_addr.s_addr=htonl(INADDR_ANY);

		/*	SO_REUSEADDR is set so that we don't need to wait for all the
		 *	existing connections to close when restarting the server. */
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));


		if(bind(sockfd, (struct sockaddr*)&socket_addr, sizeof(socket_addr))==-1)
			return error_code(1, "Couldn't bind.");

		if(listen(sockfd, 64)==-1)
			return error_code(1, "Deaf.");


		/*	root privileges should be dropped after this point. The server has
		 *	been initialized and bound its socket, and will now run as whatever
		 *	$web_user_name is set as in the init_ws config file. */
		if(gpasswd){
			// Fix log file ownership.
			if(change_log_owner(gpasswd->pw_uid, gpasswd->pw_gid))
				error_code(0, "Warning: Could not take ownership of log files.");

			// Change groups.
			if(setgid(gpasswd->pw_gid))
				error_code(0, "Warning: You don't have permission to setgid to %d.", gpasswd->pw_gid);

			// Change users.
			if(setuid(gpasswd->pw_uid))
				error_code(0, "Warning: You don't have permission to setuid to %s.", gpasswd->pw_name);
		}


		// Let init handle the children.
		signal(SIGCHLD, SIG_IGN);

		str=calloc(n=256, sizeof(char));
		while(state){
			client_length=sizeof(socket_addr_client);
			memset(&socket_addr_client, 0, client_length);
			sockfd_client=accept(sockfd, (struct sockaddr*)&socket_addr_client, &client_length);

			if(set_env_from_conf())
				return error_code(1, "General Configuration issue. ** Server stopping. **");

			// Fork if a client connection accepts successfully.
			if(sockfd_client<0)
				printf("No connection.\n");
			else if(!(pid=fork())){
				close(sockfd);

				// Bind a file stream to the socket. This makes everything easy.
				if(!(client_stream=fdopen(sockfd_client, "w+")))
					return error_code(1, "Couldn't get a stream.");
				request_stream=client_stream;

				// Timeout handler
				signal(SIGALRM, request_timeout);
				signal(SIGUSR1, request_timeout);

				do{ /* Loop to process multiple requests on the same TCP connection. */
					unsetenv("HTTP_COOKIE");
					unsetenv("HTTP_REFERER");
					unsetenv("CONTENT_LENGTH");
					unsetenv("CONTENT_TYPE");
					unsetenv("CONNECTION_MODE");
					unsetenv("REQUEST_HOST");
					unsetenv("IF_MODIFIED_SINCE");

					// Hold the door open.
					alarm(60);

					// Receive HTTP header.
					fflush(client_stream);
					do{	if(getline(&str, &n, client_stream)<=0)
							goto end_of_stream;
						sanitize_str(str);
					}	while(!*str);

					// Clear timeout.
					alarm(0);
					
					// Parse Header.
					push_str(str);
					v=chop_words(str);
					method=*(v+1);
					uri=*(v+2);
					http_standard=*(v+3);

					// Increment for each request handled.
					request_count++;

					if(!method||!uri||!http_standard)
						http_default_error(client_stream, 400, "Bad Request");
					else {
						while(getline(&str, &n, client_stream)>0){
							sanitize_str(str);

							// These need to be unset for each request.
							if(!*str)
								break;
							else if(str==strcasestr(str, "Cookie: "))
								setenv("HTTP_COOKIE", str+8, 1);
							else if(str==strcasestr(str, "Referer: "))
								setenv("HTTP_REFERER", str+9, 1);
							else if(str==strcasestr(str, "Content-length: "))
								setenv("CONTENT_LENGTH", str+16, 1);
							else if(str==strcasestr(str, "Content-type: "))
								setenv("CONTENT_TYPE", str+14, 1);
							else if(str==strcasestr(str, "Connection: "))
								setenv("CONNECTION_MODE", str+12, 1);
							else if(str==strcasestr(str, "Host: "))
								setenv("REQUEST_HOST", str+6, 1);
							else if(str==strcasestr(str, "If-Modified-Since: "))
								setenv("IF_MODIFIED_SINCE", str+19, 1);
						}

						// Clean up POST data.
						if(!strcasecmp(method, "POST")){
							if(s=getenv("CONTENT_LENGTH"))
								post_length=atoi(s);
							if(post_length>0){
								post_raw_data=calloc(post_length+1, sizeof(char));
								*(post_raw_data+post_length)=0;
								fread(post_raw_data, sizeof(char), post_length, client_stream);
							}
						}

						// Request handler child starts here.
						if(!(pid=fork())){
							// Break the QUERY_STRING off of the URI.
							for(a=uri;*a;a++)
								if(*a=='?'){
									query=calloc(strlen(a), sizeof(char));
									strcpy(query, a+1);
									break;
								}
							*a=0;
							unescape_url(uri);

							// Warning for a 301 redirect later.
							if(*(uri+strlen(uri)-1)!='/')
								setenv("kws_pot_err", "dirnotdir", 1);

							// TODO: Improve security of this filter.
							// FIXME: Causes some files to be unreachable.
							for(a=uri;*a;a++)
								if(*a==';'||*a=='`'||*a=='&'||*a=='|')
									*a=' ';

							/*	Web directory protection stops the client from accessing
							 *	files with a realpath outside of $web_root. This
							 *	includes symlinks. */
							web_root=getenv("web_root");
							sprintf(str, "%s%s", web_root, uri);
							if((a=getenv("web_dir_protection"))&&strcasecmp(a, "no"))
								cmp_web_root=realpath(str, NULL);
							else cmp_web_root=web_root;

							if(!cmp_web_root) // Can't happen.
								http_default_error(client_stream, 404, "File not found.");
							else if(cmp_web_root==strstr(cmp_web_root, web_root)){
								setenv("SERVER_PROTOCOL", http_standard, 1);
								sprintf(buf, "%d", port);
								setenv("SERVER_PORT", buf, 1);
								setenv("REQUEST_METHOD", method, 1);
								// Skipping PATH_INFO and PATH_TRANSLATED
								setenv("SCRIPT_NAME", uri, 1);
								setenv("QUERY_STRING", (query)?query:"", 1);
								setenv("REMOTE_ADDR", (char*)inet_ntoa(socket_addr_client.sin_addr), 1);
								// Skipping CONTENT_TYPE and CONTENT_LENGTH

								if(!strcasecmp(method, "HEAD"))
									http_request(client_stream, str, HEAD, NULL);
								else if(!strcasecmp(method, "GET"))
									http_request(client_stream, str, GET, NULL);
								else if(!strcasecmp(method, "POST"))
									http_request(client_stream, str, POST, post_raw_data);
								else http_default_error(client_stream, 501, "Method Not Implemented.");

							} else http_default_error(client_stream, 401, "Permission denied.");
						
							fclose(client_stream);
							close(sockfd_client);

							// End of handler process.
							exit(0);
						}
						waitpid(pid, NULL, 0);
					}
					free(*v);

					// Handle another request if connection was keep-alive.
				}	while((s=getenv("CONNECTION_MODE")) && !strcasecmp(s, "keep-alive"));
				fclose(client_stream);

				// Dump a list of handled requests to the log file.
end_of_stream:
				error_code(0, "Handled %d request%s for %s", request_count, (request_count==1)?"":"s", (char*)inet_ntoa(socket_addr_client.sin_addr));
				while(s=pop_str()){
					error_code(0, "--\t%s", s);
					free(s);
				}

				// End of child process
				close(sockfd_client);
				exit(0);
			}

			// Close handle to the child's socket in parent.
			close(sockfd_client);
		}

		// End of server process
		close(sockfd);
		exit(0);
	}

	// PID info returned when starting server.
	if(pid>0)
		printf("Server started on port %d. [%d]\n", port, pid);
	else printf("Failed\n");
	return 0;
}


/*	If the user is taking too long to make another request, we'll time out
	and kill the handler. */
void request_timeout(int i){
	fclose(request_stream);
}
