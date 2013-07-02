#!/bin/bash
# Simple build script, ideal for users who intend to build the server and run
# it out of the box immediately. Always performs a full, clean build.

# Error handling.
error_level=0
inc_error(){
	error_level=$(expr $error_level + 1)
}

cat << EOF
This script will rebuild the entire site system. If you want to do something
else, you should look at the Makefile in /src/

Press [RETURN] to begin or Ctrl-C to cancel.
EOF
read buf

# Dependency checks.
# This could be refactored to use a dependency list from a file.
oifs=$IFS
IFS=$'\n'
for dep in $(cat .depends); do
	_c=$(expr substr "$dep" 1 1)
	if [ "$_c" != "#" ]; then
		if ! which $dep > /dev/null; then
			echo "You are missing $dep."
			inc_error
		fi
	fi
done
IFS=$oifs
cat << EOF
Predependencies: $error_level errors.

EOF
if [ "$error_level" -gt "0" ]; then
	exit
fi

# Start build process.
error_level=0
opwd=$PWD
cd src

echo "-- Deleting old binaries."
make clean

echo "-- Rebuilding..."
if ! make; then
	inc_error
fi

cd $opwd

# Build web/index.html if it doesn't exist.
if ! stat "web/index.html" > /dev/null 2>&1; then
	echo "-- No web/index.html, adding default."
	mkdir -p web
	cp .default/index.html web/
fi

# conf/serv
if ! stat "conf/serv" > /dev/null 2>&1; then
	echo "-- No conf/serv, adding default."
	mkdir -p conf
	cp .default/serv conf/
fi

# init_ws
if ! stat "init_ws" > /dev/null 2>&1; then
	echo "-- No init_ws, adding default."
	mkdir -p conf
	cp .default/init_ws ./
fi

# Error report.
cat << EOF

Done. ($error_level build errors)
EOF

if [ "$error_level" -eq "0" ]; then
cat << EOF
Build completed successfully. You can start the server now with
	./init_ws start
EOF
fi
