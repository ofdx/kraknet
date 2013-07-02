#!/bin/sh
# Reports host name.

if [ -z "$REQUEST_HOST" ]; then
	echo -n "Kraknet"
else
	echo -n "$REQUEST_HOST"
fi
