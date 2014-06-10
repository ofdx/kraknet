/*
	conf
	Written by Mike Perron (2013)

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

	Functions for handling configuration for Kraknet.
*/
#include "conf.h"

void change_root(const char *path){
	setenv("web_root", path, 1);
	setenv("DOCUMENT_ROOT", path, 1);
	chdir(path);
}

/*	Adjust paths to be relative to the root of the server's data. */
void set_path(char **dest, char *src){
	char *root = getenv("server_home");

	*dest = calloc(2048 + strlen(root) + strlen(src), sizeof(char));

	if(*src != '/')
		sprintf(*dest, "%s/%s", root, src);
	else strcpy(*dest, src);
}

/*	Read conf the main server configuration file and handle all the details. */
int set_env_from_conf(){
	char *str = NULL;
	FILE *conf, *stream;
	char *a;

	/*	All other directories are assumed to be derived from here if they don't
		begin with a / */
	if(!(a = getenv("server_home")))
		return error_code(-1, "server_home not set");

	if(!(a = realpath(a, NULL)))
		return error_code(-1, "Server path is inaccessible. (%s)", getenv("server_home"));
	free(a);

	if(!(conf = get_conf_stream("serv", "r")))
		return error_code(-1, "Can't read serv config.");

	// SERVER_NAME
	if(!(a = get_conf_line_s(conf, "server_name", SEEK_RESET_OK)))
		a = "krakws.local";
	setenv("SERVER_NAME", a, 1);

	// web_user_name
	if(a = get_conf_line_s(conf, "web_user_name", SEEK_RESET_OK))
		setenv("web_user_name", a, 1);

	// web_root and DOCUMENT_ROOT
	if(!(a = get_conf_line_s(conf, "web_root", SEEK_RESET_OK)))
		return error_code(-1, "web_root not set.");
	set_path(&str, a);
	a = str;

	// Remove symlinks
	if(!(str = realpath(str, NULL)))
		return error_code(-1, "web_root path inaccessible.");
	change_root(str);
	free(str);
	free(a);

	// mod_root
	if(!(a = get_conf_line_s(conf, "mod_root", SEEK_RESET_OK)))
		return error_code(-1, "mod_root not set.");
	set_path(&str, a);
	setenv("mod_root", str, 1);
	free(str);

	// tmp_ws
	if(!(a = get_conf_line_s(conf, "tmp_ws", SEEK_RESET_OK)))
		return error_code(-1, "tmp_ws not set.");
	set_path(&str, a);
	setenv("tmp_ws", str, 1);

	// Create temp directory.
	mkdir(str, 0777);
	switch(errno){
		case 0: case EEXIST:
			chmod(str, 0777);
			break;
		default:
			return error_code(-1, "Could not create temp directory.");
	}

	free(str);

	// log_root
	if(!(a = get_conf_line_s(conf, "log_root", SEEK_RESET_OK)))
		error_code(0, "log_root not set in conf/serv, logging to stderr...");
	else {
		set_path(&str, a);
		setenv("log_root", str, 1);

		mkdir(str, 0777);
		switch(errno){
			case 0: case EEXIST:
				strcat(str, "/server.log");

				// mod_debug_stream has a static pointer to hold this fopen.
				stream = mod_debug_stream(GET, NULL);
				if(!stream || (stream == stderr))
					mod_debug_stream(SET, fopen(str, "a"));
				if(mod_debug_stream(GET, NULL) == stderr)
					error_code(0, "Could not create log file, logging to stderr...");
				break;
			default:
				error_code(0, "Could not create log directory, logging to stderr...");
		}
		free(str);
	}

	// use_web_dir_protection
	if(!(a = get_conf_line_s(conf, "web_dir_protection", SEEK_RESET_OK)))
		setenv("web_dir_protection", "yes", 1);
	else setenv("web_dir_protection", a, 1);

	// Frees the buffer.
	fclose(conf);
	return 0;
}

/*	Take ownership of log files if possible. */
int change_log_owner(uid_t uid, gid_t gid){
	struct dirent *d;
	struct stat s;
	FILE *conf;
	DIR *dir;

	int e = 0;
	char *a, *b;

	// Find out where logs are supposed to be.
	if(!(conf = get_conf_stream("serv", "r")))
		return error_code(1, "Could not open conf/serv.");
	if(!(a = get_conf_line_s(conf, "log_root", SEEK_RESET_OK)))
		return error_code(0, "No logging directory set.");
	fclose(conf);

	// Log directory must exist to change its owner.
	if(stat(a, &s))
		return 0;
	if(chown(a, uid, gid))
		return error_code(2, "Could not change owner of \"%s\".", a);
	if(!(dir = opendir(a)))
		return error_code(0, "Warning: Could not access directory structure for \"%s\".", a);

	// Own each file in the logs directory.
	b = calloc(256 + strlen(a), sizeof(char));
	while(d = readdir(dir)){
		if(!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
			continue;
		sprintf(b, "%s/%s", a, d->d_name);
		e += chown(b, uid, gid);
	}
	free(b);
	closedir(dir);	

	return e ? error_code(0, "Warning: %d files could not be chowned.", e) : 0;
}

void calibrate_path(){
	char *host, *server, *path;
	struct stat s;

	server = getenv("server_home");
	host = getenv("HTTP_HOST");

	if(server && *server){
		if(host && *host){
			path = calloc(strlen(server) + strlen(host) + 32, sizeof(char));
			sprintf(path, "%s/domains/%s", server, host);

			if(!stat(path, &s))
				change_root(path);

			free(path);
		} else change_root(server);
	}
}
