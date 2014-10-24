#!/bin/bash

if [[ "$HTTP_REFERER" == *$SCRIPT_NAME ]]; then
	printf "/"
else
	printf "$HTTP_REFERER"
fi
