/*
	http11
	Written by Mike Perron (2012-2013)

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

	Functions for handling the basics of HTTP/1.1.
*/
#ifndef KWS_HTTP_11
#define KWS_HTTP_11

#include "generic.h"

#define GET 	0
#define POST	1
#define HEAD	2
#define KWS_SERVER_NAME "krakws/1.0 (Linux)"

typedef struct http_loggable {
	int skiplog; // If this is set, the request will not be logged.
	int code; // HTTP Status Code
	long bytes; // Count of bytes written by the request.
} http_loggable;

extern char *get_mime_type(char *filename);
extern http_loggable http_request(FILE *stream, char *uri, int method, char *post_raw_data);
extern void http_default_error(FILE *stream, int code, const char *optional_msg);
extern char *http_date(time_t offset_sec);
extern void http_redirect(FILE *stream, int code, const char *uri_moved);

#endif
