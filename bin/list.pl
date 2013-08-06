#!/usr/bin/perl -w
# Perl directory listing script for Krakws.
# Slightly less temporary solution.
#
# Mike Perron (2013)

use File::stat;
use Switch;

$working=$ENV{'web_root'};

# Find the present working directory.
$dir=`dirname "$ENV{'SCRIPT_NAME'}"`;
chomp $dir;

if($dir eq "/"){
	$dirf="$dir";
} else {
	$dirf="$dir/";
}

print<<EOF;
Content-Type: Text/html

<!DOCTYPE html>
<html><head>
	<meta charset=UTF-8>
	<title>Index of $dirf</title>
	<style type=text/css>
		body { font-family: sans-serif; }
		td { padding-right: 5px; }
		th { text-align: left; }
		.size { text-align: right; font-family: monospace; }
		tr:nth-child(even) { background-color:#e0e0e0; }
	</style>
</head><body>
	<h2>Files located in $dirf</h2>
	<table>
		<tr>
			<th>Name</th>
			<th>Size (Bytes)</th>
			<th>Last Modified</th>
			<th>Description</th>
		</tr>
EOF

# If the directory is the root dir, just clear it.
# No other directory looks like /\/$/
$dir="" if($dir eq "/");

# Loop to generate each row.
$dirname="$working/$dir";
opendir my($dh), $dirname or die "Couldn't read directory. ($dirname): $!";
my @file=readdir $dh;
closedir $dh;
@file=sort @file;

foreach (@file){
	$sb=stat("$working/$dir/$_");
	$size=$sb->size;
	$mime=`file -b "$working/$dir/$_"`;
	chomp $mime;

	my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime($sb->mtime);
	$year+=1900;
	$modified="$year/$mon/$mday $hour:$min:$sec";


	if($mime eq "directory"){
		$filef="$_/";
		$size="";
		switch($_){
			case ".." { $mime="<b>Move Up</b>"; }
			case "." { $mime="<b>Present Directory</b>"; }
		}
	} else {
		$filef="$_";
	}

print <<EOF;
		<tr>
			<td><a href="$dir/$_">$filef</a></td>
			<td class=size>$size</td>
			<td>$modified</td>
			<td>$mime</td>
		</tr>
EOF
}

print <<EOF;
	</table>
</body></html>
EOF
