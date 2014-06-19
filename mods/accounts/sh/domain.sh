#!/bin/bash

domainfile="cookiedomain"

if [ -e "$domainfile" ]; then
	domain=$(cat "$domainfile")
	printf "$domain"
fi
