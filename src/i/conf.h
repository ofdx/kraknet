/*
	conf
	Written by Mike Perron (2013)

	Functions for handling configuration for Kraknet.
*/

#ifndef KRAKNET_CONF_H
#define KRAKNET_CONF_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "generic.h"

extern void set_path(char *dest,char *src);
extern int set_env_from_conf();

#endif
