#!/bin/bash
# Simple build script, ideal for users who intend to build the server and run
# it out of the box immediately. Always performs a full, clean build.

# Error handling.
error_level=0
inc_error(){
	error_level=$(expr $error_level + 1)
}

# Used to rebuild web/index.html, if it's not present.
create_default_doc(){
cat > "web/index.html" << EOF
!# Welcome, traveler!
!# HTTP headers can be sent to clients here.
!# Leave comments by starting with a #.
!Cache-Control: no-cache
<!DOCTYPE html>
<html><head>
	<meta charset=UTF-8>
	<title>Welcome to Kraknet!</title>
</head><body>
	<h2>It works!</h2>
	<p>The server is running, but no content is available yet. The current time is <????time:time>.</p>
	<p><i>Kraknet Site System &copy;2012-<????time:year></i></p>
	<textarea><????sys:env></textarea>
</body></html>
EOF
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
	create_default_doc
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
