#!/bin/sh
# Simple build script, ideal for users who intend to build the server and run
# it out of the box immediately. Always performs a full, clean build.

error_level=0
inc_error(){
	error_level=`expr $error_level + 1`
}

cat << EOF
This script will rebuild the entire site system. If you want to do something
else, you should look at the Makefile in /src/

Press [RETURN] to begin or Ctrl-C to cancel.
EOF
read buf

# Dependency checks.
# This could be refactored to use a dependency list from a file.
if ! which gcc > /dev/null; then
	echo "You are missing gcc."
	inc_error
fi
if ! which make > /dev/null; then
	echo "You are missing make."
	inc_error
fi
cat << EOF
Predependencies: $error_level errors.

EOF
if [ "$error_level" -gt "0" ]; then
	exit
fi

# Start build process.
error_level=0
cd src

echo "-- Deleting old binaries."
make clean

echo "-- Rebuilding..."
if ! make; then
	inc_error
fi

# Error report.
cat << EOF

Done. ($error_level build errors)
EOF
