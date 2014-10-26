/*
	generic
	Written by Mike Perron (2012-2013)

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

	General functions to patch things together for krakws.

	unescape_url and x2c are borrowed from the NCSA HTTPD server example
	CGI application.
*/
#define _GNU_SOURCE

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utc_date.h"

#include "generic.h"

/*	Splits words at spaces and tabs. This function creates a copy of the
	original string. The first word may be the same as the second, and should
	only be used to free memory. The last pointer will be NULL, signifying the
	end of the string. */
char **chop_words(const char *src){
	char **v = NULL, *str, *r;
	int count = 2;

	if(!src)
		return NULL;

	str = calloc(1 + strlen(src), sizeof(char));
	*(v = malloc(count * sizeof(char**))) = str;
	strcpy(str, src);

	*(v + count - 1) = strtok_r(str, WORDS_DELIMINATOR, &r);
	do v = realloc(v, ++count * sizeof(char**));
	while((*(v + count - 1) = strtok_r(NULL, WORDS_DELIMINATOR, &r)));

	return v;
}

/* These two functions are borrowed directly from the NCSA HTTPD example CGI
   application. */
static char x2c(char *what){
	register char digit;
	digit=(what[0]>='A'?((what[0]&0xdf)-'A')+10:(what[0]-'0'));
	digit*=16;
	digit+=(what[1]>='A'?((what[1]&0xdf)-'A')+10:(what[1]-'0'));
	return digit;
}
void unescape_url(char *url){
	register int x, y;
	for(x=0, y=0;url[y];++x, ++y){
		if((url[x]=url[y])=='%'){
			url[x]=x2c(&url[y+1]);
			y+=2;
		}
	}
	url[x]=0;
}
/* End NCSA code */

void sanitize_str(char *str){
	if((str = strpbrk(str, "\r\n")))
		*str = 0;
}

void unquote_str(char *str){
	size_t l;

	if(!str || ((l = strlen(str)) < 2))
		return;

	if(*str == '\"')
		memmove(str, str + 1, l--);

	if(str[--l] == '\"')
		str[l] = 0;
}

/*	Opens a stream to $conf_dir/cname */
FILE *get_conf_stream(char *cname, const char *mode){
	FILE *stream;
	char *str, *a;

	if(!(a=getenv("conf_dir")))
		return NULL;

	str = calloc(strlen(cname) + strlen(a) + 2, sizeof(char));
	sprintf(str, "%s/%s", a, cname);

	stream = fopen(str, mode);
	free(str);

	return stream;
}

/*	If you only need to read one value from a configuration file, this function
 *	is better than get_conf_line_s, because it handles the stream itself. */
char *get_conf_line(char *fname, char *value){
	char *return_string;
	FILE *stream;

	if(!(stream = fopen(fname, "r")))
		return NULL;

	return_string = get_conf_line_s(stream, value, SEEK_FORWARD_ONLY);

	fclose(stream);
	return return_string;
}

/*	This function returns a pointer to a statically allocated buffer containing
 *	the arguments (following a '=') from a line in a stream. There are two mode
 *	possibilities: SEEK_FORWARD_ONLY will search forward from the file pointers
 *	current position; SEEK_RESET_OK allows the function to search the remainder
 *	of the file, reset the pointer to the beginning, and search from wherever it
 *	started. SEEK_RESET_OK is optimal for reading actual config files multiple
 *	times. Do not use on non-file streams. */
char *get_conf_line_s(FILE *stream, char *value, enum SEEK_MODE mode){
	static char *str = NULL;
	static size_t n;

	char *return_string = NULL;
	char *a, *s;
	ssize_t r;

	long initial_file_pos = ftell(stream);

	do {
		r = getline(&str, &n, stream);

		// EOF handling and looping.
		if(feof(stream) || (r < 0)){
			// initial_file_pos is false (0) if file started from the beginning.
			// No need to rewind in that case.
			if((mode == SEEK_RESET_OK) && initial_file_pos){
				rewind(stream);
				mode = SEEK_POST_REWIND;
			} else break;
		}

		// Break if we seek past the point we started.
		if((mode == SEEK_POST_REWIND) && (ftell(stream) >= initial_file_pos))
			break;

		// Skip comments
		if(*str == '#')
			continue;
		sanitize_str(str);

		// Skip indentation
		for(s = str;(*s == ' ') || (*s == '\t'); s++);
		a = s;

		// Match needle to the beginning of the line.
		if(a == strcasestr(a, value)){
			for(s = a; *s; s++)
				if(*s == '=')
					break;
			a = (*s) ? s + 1 : s;

			return_string = str;
			strcpy(str, a);
		}
	}	while(!return_string);

	unquote_str(return_string);

	return return_string;
}

/*	Configure debug output stream. */
FILE *mod_debug_stream(enum debug_stream_op op, FILE *stream){
	static FILE *dbg_stream = NULL;

	switch(op){
		case SET:
			dbg_stream = stream;
		case GET:
		default:
			if(!dbg_stream)
				dbg_stream = stderr;
			return dbg_stream;
	}
}

/*	Prints the message with a timestamp to stderr. Returns the code value
	specified. */
int error_code(int code, const char *msg, ...){
	FILE *stream = mod_debug_stream(GET, NULL);
	va_list va;

	va_start(va, msg);
	if(msg == strstr(msg, "--")){
		vfprintf(stream, msg + 2, va);
	} else {
		fprintf(stream, "%s [kraknet]: ", post_time("%Y/%m/%d %H:%M:%S.", 1));
		vfprintf(stream, msg, va);
	}
	fputc('\n', stream);
	va_end(va);

	fflush(stream);
	return code;
}


// Return path to module's home.
char *mod_home(char *mod){
	static char p[1024];
	char *home_dir;

	if(!(home_dir = getenv("mod_root")))
		return error_code(0, "Missing environment variable $mod_root."), NULL;
	sprintf(p, "%s/%s", home_dir, mod);
	return p;
}


// Find a module, run it, and push the result text to stdout.
int mod_find_p(char *mod, char *script, char *args, char **ret){
	char *home_dir;
	char *str, *s;
	char pwd[2048];
	size_t n = 256;
	FILE *pipe;
	int c,p=0;

	if(!(home_dir = getenv("mod_root")))
		return error_code(-1, "Missing environment variable $mod_root.");

	str = calloc(256 + n, sizeof(char));
	sprintf(str, "%s/%s/info.txt", home_dir, mod);
	if((s = get_conf_line(str, script))){
		unquote_str(script = s);

		// Set working directory for the module.
		getcwd(pwd, sizeof(pwd));
		sprintf(str, "%s/%s", home_dir, mod);
		if(chdir(str))
			return error_code(1, "Module missing. (%s)", mod);

		// Run script.
		sprintf(str, "./%s %s", script, args?args:"");
		if((pipe = popen(str, "r"))){
			while((c = getc(pipe)) != EOF){
				if(ret){
					if(p > n){
						n += 256;
						str = realloc(str, n * sizeof(char));
					}
					str[p++] = c;
				} else fputc(c, stdout);
			}
			str[p]=0;
			pclose(pipe);
		}

		// Restore previous working directory.
		chdir(pwd);

		if(!ret)
			free(str);
		else *ret = str;
	} else return error_code(1, "No script found. (%s:%s)", mod, script);

	return 0;
}
int mod_find(char *mod, char *script, char *args){
	return mod_find_p(mod, script, args, NULL);
}
int mod_find_ps(char *mod_script, char *args, char **ret){
	char *mod, *script;
	char *s, *str;
	int code;

	str = mod_script;
	if(!(s = strstr(str, ":")))
		return error_code(-1, "Bad request format for %s.", str);
	*s = 0;

	mod = calloc(1 + strlen(str), sizeof(char));
	strcpy(mod, str);

	script = calloc(1 + strlen(++s), sizeof(char));
	strcpy(script, s);

	// Fix original data.
	*(s - 1) = ':';

	code = mod_find_p(mod, script, args, ret);
	free(mod);
	free(script);

	return code;
}

int kws_fclose(FILE **stream){
	FILE *f = *stream;

	*stream = NULL;
	return f ? fclose(f) : 0;
}
