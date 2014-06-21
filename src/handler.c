#define _GNU_SOURCE

#include <ctype.h>
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

#include "http11.h"
#include "generic.h"
#include "conf.h"

#include "handler.h"

static void log_response(char *remote_addr, char *user_identifier, char *user_name, char *date, char *request, int http_response, int bytes){
	char str_http_response[16];
	char str_bytes[256];

	if(http_response != -1)
		sprintf(str_http_response, "%d", http_response);
	else strcpy(str_http_response, "-");

	if(bytes != -1)
		sprintf(str_bytes, "%d", bytes);
	else strcpy(str_bytes, "-");

	error_code(0, "--%s %s %s %s \"%s\" %s %s",
		remote_addr,
		user_identifier,
		user_name,
		date,
		request,
		str_http_response,
		str_bytes
	);
}


// Valid strings are [a-zA-Z-]. Others are set null at *str.
static char * envstring(char *str){
	char *s;
	for(s = str; *s; s++){
		if(*s == '-')
			*s = '_';
		else if((*s >= 'a') && (*s <= 'z'))
			*s -= 'a' - 'A';
		else if((*s < 'A') || (*s > 'Z')){
			*str = 0;
			break;
		}
	}

	return str;
}

// Destroys the original string.
static void sethttpenv(char *str){
	char *a, *b;

	a = str;
	b = strchr(str, ':');

	if(!a || !b)
		return;

	*b = 0;
	if(!*envstring(a))
		return;

	a = b+1;
	while(isspace(*a))
		a++;
	if(!*a)
		return;

	// Exceptions to HTTP_ prepend.
	if(
		(strcmp(str, "CONTENT_LENGTH")) &&
		(strcmp(str, "CONTENT_TYPE")) &&
		(strcmp(str, "CONNECTION_MODE")) &&
		(strcmp(str, "IF_MODIFIED_SINCE"))
	){
		b = calloc(6 + strlen(str), sizeof(char));
		sprintf(b, "HTTP_%s", str);
		str = b;
	}

	setenv(str, a, 1);
	if(b == str)
		free(b);
}


int handle_connection(FILE *request_stream, struct sockaddr_in socket_addr_client, int port){
	char *s, *str = NULL;
	size_t n;

	char **v;
	char *method, *uri, *http_standard, *query;

	int post_length;
	char *post_raw_data, *request_original;
	char *a, *web_root, *cmp_web_root, buf[64];

	int skiplog = 0;
	pid_t pid = 0;

	// Hold the door open.
	alarm(60);

	// Receive HTTP header.
	fflush(request_stream);
	do {
		if(getline(&str, &n, request_stream) <= 0)
			return 1;
		sanitize_str(str);
	}	while(!*str);

	// Clear timeout.
	alarm(0);

	// Preserve request string.
	request_original = calloc(1 + strlen(str), sizeof(char));
	strcpy(request_original, str);
	
	// Parse Header.
	v = chop_words(str);
	method = v[1];
	uri = v[2];
	http_standard = v[3];

	if(!method || !uri || !http_standard)
		http_default_error(request_stream, 400, "Bad Request");
	else if(!(pid = fork())){
		// Set HTTP headers to appropriate environment variables.
		while(getline(&str, &n, request_stream) > 0){
			sanitize_str(str);

			if(!*str)
				break;
			else sethttpenv(str);
		}

		// Calibrate path to account for domain-based sites.
		calibrate_path();

		// Clean up POST data.
		if(!strcasecmp(method, "POST")){
			if((s = getenv("CONTENT_LENGTH")))
				post_length = atoi(s);
			if(post_length > 0){
				post_raw_data = calloc(post_length + 1, sizeof(char));
				*(post_raw_data + post_length) = 0;
				fread(post_raw_data, sizeof(char), post_length, request_stream);
			}
		}

		// Break the QUERY_STRING off of the URI.
		setenv("REQUEST_URI", uri, 1);
		query = NULL;
		for(a = uri; *a; a++){
			if(*a == '?'){
				query = calloc(strlen(a), sizeof(char));
				strcpy(query, a + 1);
				break;
			}
		}
		*a = 0;
		unescape_url(uri);

		// Warning for a 301 redirect later.
		if(*(uri + strlen(uri) - 1) != '/')
			setenv("kws_pot_err", "dirnotdir", 1);

		/*	Web directory protection stops the client from accessing
		 *	files with a realpath outside of $web_root. This
		 *	includes symlinks. */
		web_root = getenv("web_root");
		sprintf(str, "%s%s", web_root, uri);
		if((a = getenv("web_dir_protection")) && strcasecmp(a, "no"))
			cmp_web_root = realpath(str, NULL);
		else cmp_web_root = web_root;

		if(!cmp_web_root) // Can't happen.
			http_default_error(request_stream, 404, "File not found.");
		else if(cmp_web_root == strstr(cmp_web_root, web_root)){
			setenv("SERVER_PROTOCOL", http_standard, 1);
			sprintf(buf, "%d", port);
			setenv("SERVER_PORT", buf, 1);
			setenv("REQUEST_METHOD", method, 1);
			// Skipping PATH_INFO and PATH_TRANSLATED
			setenv("SCRIPT_NAME", uri, 1);
			setenv("QUERY_STRING", (query ? query : ""), 1);
			setenv("REMOTE_ADDR", (char*)inet_ntoa(socket_addr_client.sin_addr), 1);
			// Skipping CONTENT_TYPE and CONTENT_LENGTH

			free(query);

			if(!strcasecmp(method, "HEAD"))
				skiplog = http_request(request_stream, str, HEAD, NULL);
			else if(!strcasecmp(method, "GET"))
				skiplog = http_request(request_stream, str, GET, NULL);
			else if(!strcasecmp(method, "POST")){
				skiplog = http_request(request_stream, str, POST, post_raw_data);
				free(post_raw_data);
			} else http_default_error(request_stream, 501, "Method Not Implemented.");

		} else http_default_error(request_stream, 401, "Permission denied.");
		
		// Log this request.
		if(!skiplog)
			log_response(
				(char*)inet_ntoa(socket_addr_client.sin_addr), // Remote address
				"-", // User identifier
				"-", // User name
				"-", // Formated date (e.g. [10/Oct/2000:13:55:35 -0700])
				request_original, // Request (e.g. "GET / HTTP/1.1")
				-1, // Reponse code (e.g. 200)
				-1 // Response bytes
			);

		// End of handler process.
		kws_fclose(&request_stream);

		exit(0);
	}
	if(pid > 0)
		waitpid(pid, NULL, 0);
	free(str);
	free(*v);
	free(v);
	free(request_original);

	// Handle another request if connection was keep-alive.
	return ((s = getenv("CONNECTION_MODE")) && !strcasecmp(s, "keep-alive"))?0:-1;
}
