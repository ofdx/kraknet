/*
	conf
	Written by Mike Perron (2013)

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

	Functions for handling configuration for Kraknet.
*/

#ifndef KRAKNET_CONF_H
#define KRAKNET_CONF_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "generic.h"

extern void set_path(char *dest, char *src);
extern int set_env_from_conf();

extern int change_log_owner(uid_t uid, gid_t gid);

#endif
