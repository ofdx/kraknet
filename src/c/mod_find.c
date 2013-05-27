/*
	mod_find
	Written by Mike Perron (2012-2013)

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

	Dumps out the script for a given module, provided as an argument of the
	form mod:script [arguments list].
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "generic.h"

int main(int argc,char **argv){
	char *mod,*script,*args=NULL;
	char *home_dir;

	char *s,*str,**p;
	size_t n=0;

	FILE *pipe;
	int c;

	if(argc<2)
		return error_code(-1, "Too few paremeters for %s.",*argv);

	if(!(home_dir=getenv("mod_root")))
		return error_code(-1, "Missing environment variable $mod_root.");

	//Look for a colon between module and script
	str=*(argv+1);
	if(!(s=strstr(str,":")))
		return error_code(-1, "Bad request format for %s.",*argv);
	*s=0;

	mod=calloc(32,sizeof(char));
	strcpy(mod,str);

	script=calloc(64,sizeof(char));
	strcpy(script,++s);

	// Flatten arguments to a single string.
	if(argc>2){
		p=argv+2;
		do{	n+=256;
			args=realloc(args,n*sizeof(char));
			if(n==256)
				strcpy(args,*p); 
			else {
				strcat(args," ");
				strcat(args,*p);
			}
		}	while(*(++p));
	}

	str=calloc(256+n,sizeof(char));
	sprintf(str,"%s/%s/info.txt",home_dir,mod);

	if(s=get_conf_line(str,script)){
		unquote_str(script=s);
		sprintf(str,"%s/%s/%s %s",home_dir,mod,script,args?args:"");
		if(pipe=popen(str,"r")){
			while(1){
				c=getc(pipe);
				if(feof(pipe))
					break;
				fputc(c,stdout);
			}	pclose(pipe);
		}
	} else return error_code(1, "No script found. (%s:%s)", mod, script);

	return 0;
}
