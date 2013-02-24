/*
	http11
	Written by Mike Perron (2012-2013)

	Functions for handling the basics of HTTP/1.1.
*/
#ifndef KWS_HTTP_11
#define KWS_HTTP_11

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "generic.h"
#include "utc_date.h"

#define GET 	0
#define POST	1
#define HEAD	2
#define KWS_SERVER_NAME "krakws (dev)"

extern void http_request(FILE *stream, char *uri, int method);
extern void http_default_error(FILE *stream, int code, const char *optional_msg);
extern void http_date(FILE *stream,int offset_sec);
extern void http_redirect(FILE *stream, int code, const char *uri_moved);

#endif
