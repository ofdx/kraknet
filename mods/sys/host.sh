#!/bin/sh
# Reports host name.

if [ -z "$HTTP_HOST" ]; then
	echo -n "Kraknet"
else
	echo -n "$HTTP_HOST"
fi
