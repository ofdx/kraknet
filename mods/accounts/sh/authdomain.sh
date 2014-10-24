#!/bin/bash

domainfile="authdomain"

if [ -e "$domainfile" ]; then
	domain=$(cat "$domainfile")
	printf "$domain"
fi
