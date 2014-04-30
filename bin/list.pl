#!/usr/bin/perl -w
# Perl directory listing script for Krakws.
# Slightly less temporary solution.
#
# Mike Perron (2013)

use File::stat;
use Fcntl ':mode';

$working = $ENV{'web_root'};

# Find the present working directory.
$dir = `dirname "$ENV{'SCRIPT_NAME'}"`;
chomp $dir;
$dirf = ($dir eq "/") ? "$dir" : "$dir/";

# If the directory is the root dir, just clear it.
# No other directory looks like /\/$/
$dir = "" if($dir eq "/");

# Loop to generate each row.
$dirname = "$working/$dir";
opendir my($dh), $dirname or die "Couldn't read directory. ($dirname): $!";
my @file = readdir $dh;
@file = sort @file;
closedir $dh;

my $filecount = @file;


my $sort = 'name';
my $sort_dir = 'asc';
my $showdesc = 0;

# Get QUERY_STRING
my $buffer=$ENV{"QUERY_STRING"};
my %queryvals;
if(length($buffer)>0){
	my @pairs=split(/[;&]/, $buffer);
	foreach my $pair (@pairs){
		my ($name, $value) = split(/=/, $pair);
		$name =~ s/^\s+//;
		$value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
		chomp($queryvals{$name} = $value);
	}
}

if($queryvals{'sort'}){ $sort = $queryvals{'sort'} }
if($queryvals{'dir'}){ $sort_dir = $queryvals{'dir'} }
if($queryvals{'desc'}){ $showdesc = $queryvals{'desc'} }


my $index = 0;
my @entries = [];

my %colclass = (
	name => 'name',
	size => 'size',
	date => 'date',
	mime => 'mime',
);

foreach my $name (@file){
	my $mime = '';
	if($showdesc != 0){ $mime = `file -b "$working/$dir/$name"` }
	my $sb = stat("$working/$dir/$name") or next;
	my $size = $sb->size;
	chomp $mime;

	my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime($sb->mtime);
	$year += 1900;
	$modified = sprintf("%04d/%02d/%02d&nbsp;%02d:%02d:%02d", $year, $mon, $mday, $hour, $min, $sec);


	if(S_ISDIR($sb->mode)){
		$filef = "$name/";
		$size = "";
		if($name eq ".."){
			$mime = "<b>Move Up</b>";
		} elsif($name eq "."){
			next;
		}
	} else {
		$filef = "$name";
	}

	$entries[$index++] = {
		path => $name,
		name => $filef,
		size => $size,
		modified => $modified,
		mime => $mime
	};
}

my %headerlabels = (
	name => "Name",
	size => "Size&nbsp;(bytes)",
	date => "Last&nbsp;Modified",
	mime => "Description"
);
my %headers = (
	name => "<a href=\"?sort=name&dir=desc&desc=$showdesc\">$headerlabels{name}</a>",
	size => "<a href=\"?sort=size&dir=desc&desc=$showdesc\">$headerlabels{size}</a>",
	date => "<a href=\"?sort=date&dir=desc&desc=$showdesc\">$headerlabels{date}</a>",
	mime => "<a href=\"?sort=mime&dir=desc&desc=$showdesc\">$headerlabels{mime}</a>"
);
$colclass{$sort} .= ' sortby';
my @entries_sorted = @entries;
if($sort eq "name"){
	if($sort_dir eq "desc"){
		@entries_sorted = sort { $b->{name} cmp $a->{name} } @entries;
		$headers{name} = "<a href=\"?sort=name&dir=asc&desc=$showdesc\">$headerlabels{name}</a>";
	}
} else {
	$headers{name} = "<a href=\"?sort=name&dir=asc&desc=$showdesc\">$headerlabels{name}</a>";
		
   	if($sort eq "size"){
		if($sort_dir eq "asc"){
			@entries_sorted = sort { $a->{size} <=> $b->{size} } @entries;
		} else {
			$headers{size} = "<a href=\"?sort=size&dir=asc&desc=$showdesc\">$headerlabels{size}</a>";
			@entries_sorted = sort { $b->{size} <=> $a->{size} } @entries;
		}
	} elsif($sort eq "date"){
		if($sort_dir eq "asc"){
			@entries_sorted = sort { $a->{modified} cmp $b->{modified} } @entries;
		} else {
			$headers{date} = "<a href=\"?sort=date&dir=asc&desc=$showdesc\">$headerlabels{date}</a>";
			@entries_sorted = sort { $b->{modified} cmp $a->{modified} } @entries;
		}
	} elsif($sort eq "mime"){
		if($sort_dir eq "asc"){
			@entries_sorted = sort { $b->{mime} cmp $a->{mime} } @entries;
		} else {
			$headers{mime} = "<a href=\"?sort=mime&dir=asc&desc=$showdesc\">$headerlabels{mime}</a>";
			@entries_sorted = sort { $a->{mime} cmp $b->{mime} } @entries;
		}
	}
}

my $showdescslink;
if($showdesc != 0){
	$showdescslink = "<a href=\"?sort=$sort&dir=$sort_dir&desc=0\">Hide Descriptions (fast)</a>";
} else {
	$showdescslink = "<a href=\"?sort=$sort&dir=$sort_dir&desc=1\">Show Descriptions (slow)</a>";
}

print<<EOF;
Content-Type: Text/html

<!DOCTYPE html>
<html><head>
	<meta charset=UTF-8>
	<title>Index of $dirf</title>
	<style type=text/css>
		body { font-family: sans-serif; }
		table { border-spacing: 0px; }
		th { text-align: left; border-bottom: solid 1px black; padding: 0 1em; }
		td { padding-right: 1em; }
		.size { text-align: right; font-family: monospace; }
		.date { font-family: monospace; }
		tr:nth-child(even) { background-color:#e0e0f0; }
		tr:hover td { background-color: #d0d0e0; }
		td.name { padding: 0.2em 0.2em; }
	</style>
</head><body>
	<h2>$filecount files located in $dirf</h2>
	<p>$showdescslink</p>
	<table>
		<tr>
			<th>$headers{name}</th>
			<th>$headers{size}</th>
			<th>$headers{date}</th>
EOF
if($showdesc != 0){
	print<<EOF;
			<th>$headers{mime}</th>
EOF
}
print<<EOF;
		</tr>
EOF

foreach my $ref (@entries_sorted){
	print <<EOF;
		<tr>
			<td class=\"$colclass{name}\"><a href="$dir/$ref->{'path'}">$ref->{'name'}</a></td>
			<td class=\"$colclass{size}\">$ref->{'size'}</td>
			<td class=\"$colclass{date}\">$ref->{'modified'}</td>
EOF
	if($showdesc != 0){
		print<<EOF;
			<td class=\"$colclass{mime}\">$ref->{'mime'}</td>
EOF
	}
	print <<EOF;
		</td>
EOF
}

print <<EOF;
	</table>
</body></html>
EOF

exit 0
