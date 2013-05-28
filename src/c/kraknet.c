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

int main(int argc, char **argv){
	char *home_dir, *web_root;
	char *buf, *b, *b_r;
	char *mod, *script;

	FILE *r, *lookup, *pipe;
	char *seek, *root, *str;
	size_t b_size=256, n;

	char headers_closed=0;

	printf("Content-Type: text/html\r\n");

	if(!(home_dir=getenv("mod_root"))){
		printf("\nBad configuration\n");
		return error_code(-1, "Missing environment variable $mod_root.");
	}

	if(!(r=fopen(argv[1], "r")))
		return error_code(-1, "Could not open file \"%s\"", argv[1]);

	buf=calloc(256, sizeof(char));

	//Begin accounts magic
	setenv("kraknet_user_auth", "NO", 1);
	if(pipe=popen("mod_find accounts:auth", "r")){
		fgets(buf, 256, pipe);
		sanitize_str(buf);

		if(!strncmp(buf, "OK", 2)){
			setenv("kraknet_user", buf+3, 1);
			setenv("kraknet_user_ip", getenv("REMOTE_ADDR"), 1);
			setenv("kraknet_user_auth", "OK", 1);
		} else printf(
			"Set-Cookie: sid=deleted; Expires=Thu, 01 Jan 1970 00:00:01 GMT; Path=/\r\n"
		);
		pclose(pipe);
	}
	//End accounts magic

	b=b_r=calloc(b_size=256, sizeof(char));
	while(getline(&b_r, &b_size, r)!=-1){
		if(*b_r!='!')
			break;
		if(*(b_r+1)=='#')
			continue;
		fputs(b_r+1, stdout);
	}	
	fputs("\r\n", stdout);

	str=calloc(n=256, sizeof(char));
	do{	b=b_r;
		while(*b){
			if(!(seek=strstr(b, "<????")))
				break;
			else {
				*seek=0;
				fputs(b, stdout);
				b=seek+5;
				if(!(seek=strstr(b, ">")))
					break;
				*seek=0;
				sprintf(buf, "mod_find %s", b);

				if(!(pipe=popen(buf, "r")))
					break;

				while(getline(&str, &n, pipe)!=-1)
					fputs(str, stdout);
				pclose(pipe);

				b=seek+1;
			}
		}
		fputs(b, stdout);
	}	while(getline(&b_r, &b_size, r)!=-1);
	fclose(r);
	return 0;
}
