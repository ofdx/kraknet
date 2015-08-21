#!/usr/bin/perl
use strict;
use DBI;

my $dbh = DBI->connect('dbi:mysql:kws', 'kraknet', '') or &fail("could not access DB");

my $user = $ARGV[0];
my $sql = qq/SELECT pretty FROM users WHERE name = ?;/;
my $sth = $dbh->prepare($sql);
$sth->execute($user);

my @row = $sth->fetchrow_array();
if($row[0]){
	my $pretty = $row[0];

	printf "$pretty";
	exit 0
}
