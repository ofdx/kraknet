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

int handle_connection(FILE *request_stream, struct sockaddr_in socket_addr_client, int port){
	char *s, *str = NULL;
	size_t n;

	char **v;
	char *method, *uri, *http_standard, *query;

	int post_length;
	char *post_raw_data, *request_original;
	char *a, *web_root, *cmp_web_root, buf[64];

	pid_t pid;

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
	else {
		while(getline(&str, &n, request_stream)>0){
			sanitize_str(str);

			// These need to be unset for each request.
			if(!*str)
				break;
			else if(str == strcasestr(str, "Cookie: "))
				setenv("HTTP_COOKIE", str + 8, 1);
			else if(str == strcasestr(str, "Referer: "))
				setenv("HTTP_REFERER", str + 9, 1);
			else if(str == strcasestr(str, "Content-length: "))
				setenv("CONTENT_LENGTH", str + 16, 1);
			else if(str == strcasestr(str, "Content-type: "))
				setenv("CONTENT_TYPE", str + 14, 1);
			else if(str == strcasestr(str, "Connection: "))
				setenv("CONNECTION_MODE", str + 12, 1);
			else if(str == strcasestr(str, "Host: "))
				setenv("REQUEST_HOST", str + 6, 1);
			else if(str == strcasestr(str, "If-Modified-Since: "))
				setenv("IF_MODIFIED_SINCE", str + 19, 1);
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

		// Request handler child starts here.
		if(!(pid = fork())){
			// Break the QUERY_STRING off of the URI.
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
				setenv("QUERY_STRING", (query)?query:"", 1);
				setenv("REMOTE_ADDR", (char*)inet_ntoa(socket_addr_client.sin_addr), 1);
				// Skipping CONTENT_TYPE and CONTENT_LENGTH

				if(!strcasecmp(method, "HEAD"))
					http_request(request_stream, str, HEAD, NULL);
				else if(!strcasecmp(method, "GET"))
					http_request(request_stream, str, GET, NULL);
				else if(!strcasecmp(method, "POST"))
					http_request(request_stream, str, POST, post_raw_data);
				else http_default_error(request_stream, 501, "Method Not Implemented.");

			} else http_default_error(request_stream, 401, "Permission denied.");
			
			// End of handler process.
			kws_fclose(&request_stream);

			exit(0);
		}
		waitpid(pid, NULL, 0);
	}
	free(*v);

	// Log this request.
	log_response(
		(char*)inet_ntoa(socket_addr_client.sin_addr), // Remote address
		"-", // User identifier
		"-", // User name
		"-", // Formated date (e.g. [10/Oct/2000:13:55:35 -0700])
		request_original, // Request (e.g. "GET / HTTP/1.1")
		-1, // Reponse code (e.g. 200)
		-1 // Response bytes
	);

	free(request_original);

	// Handle another request if connection was keep-alive.
	return ((s = getenv("CONNECTION_MODE")) && !strcasecmp(s, "keep-alive"))?0:-1;
}
