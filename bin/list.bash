#!/bin/bash
# Bash directory listing script for Krakws.
# Temporary solution.
#
# Mike Perron (2013)

# Find the present working directory.
DIR=$(dirname "$SCRIPT_NAME")
WORKING=$web_root
if [ "$DIR" == "/" ]; then
	DIRF="$DIR"
else
	DIRF="$DIR/"
fi

# HTML preamble.
cat << EOF
Content-Type: Text/html

<!DOCTYPE html>
<html><head>
	<meta charset=UTF-8>
	<title>Index of $DIRF</title>
	<style type=text/css>
		body { font-family: sans-serif; }
		td { padding-right: 5px; }
		th { text-align: left; }
		.size { text-align: right; font-family: monospace; }
		tr:nth-child(even) { background-color:#e0e0e0; }
	</style>
</head><body>
	<h2>Files located in $DIRF</h2>
	<table>
		<tr>
			<th>Name</th><th>Size (Bytes)</th><th>Description</th>
		</tr>
EOF

# If the directory is the root dir, just clear it.
# No other dir looks like /\/$/
if [ "$DIR" == "/" ];  then
	DIR=""
fi

# Loop to generate each row.
IFS=$'\n'
for FILE in $(ls -a "$WORKING/$DIR"); do
SIZE=$(stat --printf="%s" "$WORKING/$DIR/$FILE")
MIME=$(file -b "$WORKING/$DIR/$FILE")
if [ "$MIME" == "directory" ]; then
	FILEF="$FILE/"
	case "$FILE" in
		..)	SIZE=""
			MIME="<b>Move Up</b>"
		;;
		.)	MIME="<b>Present Directory</b>"	;&
		*)	SIZE="$(du -sh '$WORKING/$DIR/$FILE/' | awk '{print $1}')" ;;
	esac 
else
	FILEF="$FILE"
fi
cat << EOF
		<tr>
			<td><a href="$DIR/$FILE">$FILEF</a></td>
			<td class=size>$SIZE</td>
			<td>$MIME</td>
		</tr>
EOF
done
# End of loop.

# Close the table and HTML document.
cat << EOF
	</table>
</body></html>
EOF
