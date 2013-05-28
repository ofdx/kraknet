/*
	generic
	Written by Mike Perron (2012-2013)

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

	General functions to patch things together for krakws.
*/

#ifndef KWS_GENERIC_F
#define KWS_GENERIC_F

#define _GNU_SOURCE
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utc_date.h"

enum SEEK_MODE {
	SEEK_FORWARD_ONLY,
	SEEK_RESET_OK,
	SEEK_POST_REWIND // Do not use.
};

#define WORDS_DELIMINATOR " \t\r\n"

// String functions
extern char **chop_words(const char *src);
extern void unescape_url(char *url);
extern void sanitize_str(char *str);
extern void unquote_str(char *str);

// Config file parsing
extern FILE *get_conf_stream(char *cname, const char *mode);
extern char *get_conf_line(char *fname, char *value);
extern char *get_conf_line_s(FILE *stream, char *value, enum SEEK_MODE mode);

// Write an error to stderr.
extern int error_code(int code, const char *msg, ...);
#endif
