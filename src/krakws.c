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
#include <sys/wait.h>

#include "conf.h"
#include "generic.h"
#include "http11.h"
#include "handler.h"

#define KWS_DEFAULT_PORT 9001

void request_timeout(int);
FILE **request_stream = NULL;

/*
 * Write the process ID to a file, which the init_ws script is expecting to
 * find it in later.
 */
void pid_write(int port, int pid){
	char path[256];
	FILE *out = NULL;

	sprintf(path, "/tmp/krakws_%d.pid", port);
	if((out = fopen(path, "w"))){
		fprintf(out, "%d", pid);
		fclose(out);
	}
}

int main(int argc, char**argv){
	int sockfd, sockfd_client, client_length;
	struct sockaddr_in socket_addr_client;
	struct sockaddr_in socket_addr;
	int port = KWS_DEFAULT_PORT;

	struct passwd *gpasswd = NULL;
	pid_t pid;

	int optval = 2;
	char *str;

	// Reads server config file and sets environment variables.
	if(set_env_from_conf())
		return error_code(-1, "Configuration problem, bailing out.");

	// Log startup
	error_code(0, "--");
	error_code(0, "Server is starting up.");

	// Get a port number from argv if provided.
	if((argc > 1) && !(port = atoi(argv[1])))
		port = KWS_DEFAULT_PORT;

	// Tell CGI scripts about the server they're running on.
	setenv("SERVER_SOFTWARE", KWS_SERVER_NAME, 1);
	setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
	// SERVER_NAME is set by init_ws

	// Figure out what user we should run as.
	if((str = getenv("web_user_name")) && (*str)){
		if(!(gpasswd = getpwnam(str))){
			error_code(0, "Warning: No user named %s!", str);
			gpasswd = NULL;
		}

		unsetenv("web_user_name");
	} else {
		gpasswd = getpwuid(getuid());
		error_code(0, "Warning: $web_user_name not set! Running as %s.", gpasswd->pw_name);
		gpasswd = NULL;
	}

	// Fork the process and start the server.
	if(!(pid = fork())){
		// Acquire INET socket.
		if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			return error_code(1, "Socket not got.");

		memset(&socket_addr, 0, sizeof(socket_addr));
		socket_addr.sin_family = AF_INET;
		socket_addr.sin_port = htons(port);
		socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);

		/*	SO_REUSEADDR is set so that we don't need to wait for all the
		 *	existing connections to close when restarting the server. */
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

		if(bind(sockfd, (struct sockaddr*)&socket_addr, sizeof(socket_addr)) == -1)
			return error_code(1, "Couldn't bind.");

		if(listen(sockfd, 64) == -1)
			return error_code(1, "Deaf.");

		/*	root privileges should be dropped after this point. The server has
		 *	been initialized and bound its socket, and will now run as whatever
		 *	$web_user_name is set as in the init_ws config file. */
		if(gpasswd){
			// Fix log file ownership.
			if(change_log_owner(gpasswd->pw_uid, gpasswd->pw_gid))
				error_code(0, "Warning: Could not take ownership of log files.");

			// Change users.
			if(setuid(gpasswd->pw_uid))
				error_code(0, "Warning: You don't have permission to setuid to %s.", gpasswd->pw_name);
		}

		// Let init handle the children.
		signal(SIGCHLD, SIG_IGN);

		request_stream = malloc(sizeof(FILE*));

		while(1){
			client_length = sizeof(socket_addr_client);
			memset(&socket_addr_client, 0, client_length);
			sockfd_client = accept(sockfd, (struct sockaddr*) &socket_addr_client, (socklen_t*) &client_length);

			if(set_env_from_conf())
				return error_code(1, "General Configuration issue. ** Server stopping. **");

			// Fork if a client connection accepts successfully.
			if(sockfd_client < 0)
				printf("No connection.\n");
			else if(!(pid = fork())){
				close(sockfd);

				// Bind a file stream to the socket. This makes everything easy.
				if(!(*request_stream = fdopen(sockfd_client, "w+")))
					return error_code(1, "Couldn't get a stream.");

				// Timeout handler
				signal(SIGALRM, request_timeout);
				signal(SIGUSR1, request_timeout);

				// Handle this connection, close stream when done.
				while(!handle_connection(*request_stream, socket_addr_client, port));

				// End of child process
				kws_fclose(request_stream);
				KWS_FREE(request_stream);
				close(sockfd_client);
				exit(0);
			}

			// Close handle to the child's socket in parent.
			close(sockfd_client);
		}

		// End of server process
		exit(0);
	}

	// PID info returned when starting server.
	if(pid > 0){
		printf("Server started on port %d. [%d]\n", port, pid);

		// Write the PID out to a file, so the init_ws script can kill it later on stop.
		pid_write(port, pid);
	} else printf("Failed\n");

	return 0;
}


/*	If the user is taking too long to make another request, we'll time out
	and kill the handler. */
void request_timeout(int i){
	if(request_stream)
		kws_fclose(request_stream);
}
