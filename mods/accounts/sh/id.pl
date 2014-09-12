#!/usr/bin/perl
use strict;
use DBI;

my $dbh = DBI->connect('dbi:mysql:kws', 'kraknet', '') or &fail("could not access DB");

chomp(my $auth = qx/mod_find accounts:auth/);
if($? != 0){ die "Bad Auth" }
$auth =~ s/^OK (.*)$/\1/ or die "Can't grep username";

my $sql = qq/SELECT id_user FROM users WHERE name = ?;/;
my $sth = $dbh->prepare($sql);
$sth->execute($auth);

my @row = $sth->fetchrow_array();
if($row[0]){
	my $id_user = $row[0];

	printf "$id_user";
	exit 0
} else {
	die "Can't select id_user"
}
