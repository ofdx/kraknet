/*
	krakws
	Written by Mike Perron (2012-2013)

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

	krakws is the actual web server portion of Kraknet. It manages HTTP and CGI
	interactions and allows Kraknet to be run without bootstrapping into Apache
	with a bash CGI script.

	TODO: Data for POST should be cached to a timestamped file at $tmp_ws/ and
	then passed with a pipe into the script.
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
void error(const char *msg);

int main(int argc, char**argv){
	int sockfd,sockfd_client,client_length;
	struct sockaddr_in socket_addr_client;
	struct sockaddr_in socket_addr;
	int port=KWS_DEFAULT_PORT;

	FILE *client_stream=NULL,*content;
	struct passwd *gpasswd;
	struct stat sbuf;
	pid_t pid;

	int status,c,optval=2;

	char *buf=calloc(256,sizeof(char));
	char *web_root,*cmp_web_root,*date;
	char *method,*uri,*http_standard;
	char *a,*b,*query=NULL;
	char *str,*s,**v;
	size_t n;

	// Reads server config file and sets environment variables.
	if(set_env_from_conf()){
		fprintf(stderr,"Configuration problem, bailing out.\n");
		return -1;
	}

	// Get a port number from argv if provided.
	if(argc>1 && !(port=atoi(*(argv+1))))
		port=KWS_DEFAULT_PORT;

	// Tell CGI scripts about the server they're running on.
	setenv("SERVER_SOFTWARE",KWS_SERVER_NAME,1);
	setenv("GATEWAY_INTERFACE","CGI/1.1",1);
	// SERVER_NAME is set by init_ws


	// Fork the process and start the server.
	if(!(pid=fork())){
		// Acquire INET socket.
		if((sockfd=socket(AF_INET, SOCK_STREAM,0))==-1)
			error("Socket not got.");

		memset(&socket_addr,0,sizeof(socket_addr));
		socket_addr.sin_family=AF_INET;
		socket_addr.sin_port=htons(port);
		socket_addr.sin_addr.s_addr=htonl(INADDR_ANY);

		/*	SO_REUSEADDR is set so that we don't need to wait for all the
		 *	existing connections to close when restarting the server. */
		setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));


		if(bind(sockfd,(struct sockaddr*)&socket_addr,sizeof(socket_addr))==-1)
			error("Couldn't bind.");

		if(listen(sockfd,64)==-1)
			error("Deaf.");


		/*	root privileges should be dropped after this point. The server has
		 *	been initialized and bound its socket, and will now run as whatever
		 *	$web_user_name is set as in the init_ws config file. */
		if(str=getenv("web_user_name")){
			if(gpasswd=getpwnam(str)){
				if(setuid(gpasswd->pw_uid))
					fprintf(stderr,"Warning: You don't have permission to setuid to %s.\n",gpasswd->pw_name);
			} else fprintf(stderr,"Warning: No user named %s!\n",str);
			unsetenv("web_user_name");
		} else {
			gpasswd=getpwuid(getuid());
			fprintf(stderr,"Warning: $web_user_name not set! Running as %s.\n",gpasswd->pw_name);
		}

		// Let init handle the children.
		signal(SIGCHLD,SIG_IGN);

		str=calloc(n=256,sizeof(char));
		while(state){
			client_length=sizeof(socket_addr_client);
			memset(&socket_addr_client,0,client_length);
			sockfd_client=accept(sockfd,(struct sockaddr*)&socket_addr_client,&client_length);

			if(set_env_from_conf()){
				fprintf(stderr,"General Configuration issue.\nServer stopping.\n");
				exit(1);
			}

			// Fork if a client connection accepts successfully.
			if(sockfd_client<0)
				printf("No connection.\n");
			else if(!(pid=fork())){
				close(sockfd);

				// Bind a file stream to the socket. This makes everything easy.
				if(!(client_stream=fdopen(sockfd_client,"w+")))
					error("Couldn't get a stream.");

				// TODO: Parse header here (vectorized to $v);
				getline(&str,&n,client_stream);
				v=chop_words(str);
				method=*(v+1);
				uri=*(v+2);
				http_standard=*(v+3);

				if(!method||!uri||!http_standard)
					http_default_error(client_stream,400,"Bad Request");
				else {
					while(getline(&str,&n,client_stream)!=-1){
						if(str==strcasestr(str,"Cookie: ")){
							sanitize_str(str+8);
							setenv("HTTP_COOKIE",str+8,1);
						}
						else if(str==strcasestr(str,"Referer: ")){
							sanitize_str(str+9);
							setenv("HTTP_REFERER",str+9,1);
						}
						else if(str==strcasestr(str,"Content-length: ")){
							sanitize_str(str+16);
							setenv("CONTENT_LENGTH",str+16,1);
						}
						else if(str==strcasestr(str,"Content-type: ")){
							sanitize_str(str+14);
							setenv("CONTENT_TYPE",str+14,1);
						}
						else if(str==strstr(str,"\r\n"))break;
					}

					// Break the QUERY_STRING off of the URI.
					for(a=uri;*a;a++)if(*a=='?'){
						query=calloc(strlen(a),sizeof(char));
						strcpy(query,a+1);
						break;
					}
					*a=0;
					unescape_url(uri);

					// Warning for a 301 redirect later.
					if(*(uri+strlen(uri)-1)!='/')
						setenv("kws_pot_err","dirnotdir",1);

					// TODO: Improve security of this filter.
					for(a=uri;*a;a++)if(*a==';'||*a=='`'||*a=='&'||*a=='|')*a=' ';

					/*	Web directory protection stops the client from accessing
					 *	files with a realpath outside of $web_root. This
					 *	includes symlinks. */
					web_root=getenv("web_root");
					sprintf(str,"%s%s",web_root,uri);
					if((a=getenv("web_dir_protection"))&&strcasecmp(a,"no"))
						cmp_web_root=realpath(str,NULL);
					else cmp_web_root=web_root;

					if(!cmp_web_root) // Can't happen.
						http_default_error(client_stream,404,"File not found.");
					else if(cmp_web_root==strstr(cmp_web_root,web_root)){
						setenv("SERVER_PROTOCOL",http_standard,1);
						sprintf(buf,"%d",port);
						setenv("SERVER_PORT",buf,1);
						setenv("REQUEST_METHOD",method,1);
						// Skipping PATH_INFO and PATH_TRANSLATED
						setenv("SCRIPT_NAME",uri,1);
						setenv("QUERY_STRING",(query)?query:"",1);
						setenv("REMOTE_ADDR",(char*)inet_ntoa(socket_addr_client.sin_addr),1);
						// Skipping CONTENT_TYPE and CONTENT_LENGTH

						if(!strcasecmp(method,"HEAD"))
							http_request(client_stream,str,HEAD);
						else if(!strcasecmp(method,"GET"))
							http_request(client_stream,str,GET);
						else if(!strcasecmp(method,"POST"))
							http_request(client_stream,str,POST);
						else http_default_error(client_stream,501,"Method Not Implemented.");

					} else http_default_error(client_stream,401,"Permission denied.");
				}

				free(*v);
				fclose(client_stream);
				close(sockfd_client);

				// End of child process
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
		printf("Server started on port %d. [%d]\n",port,pid);
	else printf("Failed\n");
	return 0;
}


void error(const char *msg){
	fprintf(stderr,"ERROR: %s\n",msg);
	exit(-1);
}
