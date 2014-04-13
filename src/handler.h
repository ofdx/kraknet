/*
	handler
	Written by Mike Perron (2013)

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef KRAKNET_HANDLER_H
#define KRAKNET_HANDLER_H

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "http11.h"
#include "generic.h"
#include "conf.h"

extern int handle_connection(FILE *request_stream, struct sockaddr_in socket_addr_client, int port);

#endif
