/*
	kraknet
	Written by Mike Perron (2012-2013)

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

	This is where the magic happens. kraknet parses HTML documents looking for
	hint tags of the format <????module:script [args]>. When found, it looks for
	an info.txt file at $mod_root/module/info.txt. In that file there will be a
	link from the script name to the actual path, which is executed and the
	results piped through where the hint tag was in the HTML document.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "generic.h"
#include "http11.h"

// If this value is true, print out HTTP headers.
char print_headers = 1;

void serve(FILE *r){
	char *str, *s, *seek;
	char *buf, *b, *b_r;
	size_t b_size, n;

	buf = calloc(256, sizeof(char));
	b = b_r = calloc(b_size = 256, sizeof(char));
	while(getline(&b_r, &b_size, r) != -1){
		if(*b_r != '!')
			break;
		if(b_r[1] == '#')
			continue;

		if(print_headers)
			fputs(b_r + 1, stdout);
	}	
	if(print_headers)
		fputs("\r\n", stdout);

	str = calloc(n = 256, sizeof(char));
	do{	b = b_r;
		while(*b){
			if(!(seek = strstr(b, "<????")))
				break;
			else {
				*seek = 0;
				fputs(b, stdout);
				b = seek + 5;
				if(!(seek = strstr(b, ">")))
					break;
				*seek = 0;

				// Find and execute script.
				if((s = strchr(b, ' ')))
					*(s++) = 0;
				mod_find_ps(b, s, NULL);

				b = seek + 1;
			}
		}
		fputs(b, stdout);
	}	while(getline(&b_r, &b_size, r) != -1);

	free(b_r);
	free(buf);
	free(str);
}

int main(int argc, char **argv){
	char *home_dir, *buf = calloc(256, sizeof(char)), *s, *str;
	FILE *r;

	// If this is an "inline" rendering, HTTP headers are probably not wanted.
	if(argv[0] && !strcmp(argv[0], "kraknet_inline"))
		print_headers = 0;

	// Find server's HTML MIME type. (conf/mime)
	if(print_headers){
		str = get_mime_type("index.html");
		printf("Content-Type: %s\r\n", str);
		free(str);
	}

	if(!(home_dir = getenv("mod_root"))){
		printf("\nBad configuration\n");
		return error_code(-1, "Missing environment variable $mod_root.");
	}

	if(!(r = fopen(argv[1], "r")))
		return error_code(-1, "Could not open file \"%s\"", argv[1]);


	//Begin accounts magic
	setenv("kraknet_user_auth", "NO", 1);
	if(getenv("enable_accounts") && !strcmp(getenv("enable_accounts"), "true") && !mod_find_p("accounts", "auth", NULL, &buf)){
		sanitize_str(buf);
		if(!strncmp(buf, "OK", 2)){
			setenv("kraknet_user", buf + 3, 1);
			setenv("kraknet_user_ip", (s = getenv("HTTP_X_FORWARDED_FOR"))?s:getenv("REMOTE_ADDR"), 1);
			setenv("kraknet_user_auth", "OK", 1);
		}
	}
	free(buf);
	//End accounts magic

	// Serve the first file listed.
	serve(r);
	fclose(r);

	// Serve additional files
	while(argc > 2){
		argc--;
		argv++;

		if(!(r = fopen(argv[1], "r")))
			return error_code(-1, "Could not open file \"%s\"", argv[1]);
		serve(r);
		fclose(r);
	}

	return 0;
}
