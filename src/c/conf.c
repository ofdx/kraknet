/*
	conf
	Written by Mike Perron (2013)

	Functions for handling configuration for Kraknet.
*/
#include "conf.h"

/*	Adjust paths to be relative to the root of the server's data. */
void set_path(char *dest,char *src){
	static char *root=NULL;
	if(!dest){
		root=src;
		return;
	}

	if(*src!='/')
		sprintf(dest,"%s/%s",root,src);
	else strcpy(dest,src);
}

/*	Read conf the main server configuration file and handle all the details. */
int set_env_from_conf(){
	static char *str=NULL;
	FILE *conf;
	char *a;

	// Static buffer to translate relative paths on.
	if(!str)
		str=calloc(1024,sizeof(char));

	/*	All other directories are assumed to be derived from here if they don't
		begin with a / */
	if(!(a=getenv("server_home")))
		return -1;
	if(!(a=realpath(a,NULL))){
		fprintf(stderr,"Server path is inaccessible. (%s)\n",getenv("server_home"));
		return -1;
	}
	set_path(NULL,a);

	if(!(conf=get_conf_stream("serv","r")))
		return -1;

	// SERVER_NAME
	if(!(a=get_conf_line_s(conf,"server_name",SEEK_RESET_OK)))
		a="krakws.local";
	setenv("SERVER_NAME",a,1);

	// web_user_name
	if(a=get_conf_line_s(conf,"web_user_name",SEEK_RESET_OK))
		setenv("web_user_name",a,1);

	// web_root and DOCUMENT_ROOT
	if(!(a=get_conf_line_s(conf,"web_root",SEEK_RESET_OK))){
		fprintf(stderr,"web_root not set.\n");
		return -1;
	}
	set_path(str,a);

	// Remove symlinks
	if(!(str=realpath(str,NULL))){
		fprintf(stderr,"web_root path inaccessible.\n");
		return -1;
	}
	setenv("web_root",str,1);
	setenv("DOCUMENT_ROOT",str,1);

	// mod_root
	if(!(a=get_conf_line_s(conf,"mod_root",SEEK_RESET_OK))){
		fprintf(stderr,"mod_root not set.\n");
		return -1;
	}
	set_path(str,a);
	setenv("mod_root",str,1);

	// tmp_ws
	if(!(a=get_conf_line_s(conf,"tmp_ws",SEEK_RESET_OK))){
		fprintf(stderr,"tmp_ws not set.\n");
		return -1;
	}
	set_path(str,a);
	setenv("tmp_ws",str,1);

	// Create temp directory.
	mkdir(str,776);
	switch(errno){
		case 0: case EEXIST:
			break;
		default:
			fprintf(stderr,"Could not create temp directory.\n");
			return -1;
	}

	// use_web_dir_protection
	if(!(a=get_conf_line_s(conf,"web_dir_protection",SEEK_RESET_OK)))
		setenv("web_dir_protection","yes",1);
	else setenv("web_dir_protection",a,1);

	fclose(conf);
	return 0;
}
