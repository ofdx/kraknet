/*
	handler
	Written by Mike Perron (2013)

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef KRAKNET_HANDLER_H
#define KRAKNET_HANDLER_H

extern int handle_connection(FILE *request_stream, struct sockaddr_in socket_addr_client, int port);

#endif
