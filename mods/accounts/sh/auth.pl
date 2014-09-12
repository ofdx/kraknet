#!/usr/bin/perl
# Mike Perron (2013)
#
# Verify that a given session is valid.

use strict;
use DBI;

sub fail {
	printf "NO $_[0]";
	exit 0
}

my $dbh = DBI->connect('dbi:mysql:kws', 'kraknet', '') or &fail("could not access DB");

my $buffer = $ENV{HTTP_COOKIE};
my %cookies;
if(length($buffer) > 0){
	my @pairs = split(/[;&]/, $buffer);
	foreach my $pair (@pairs){
		my ($name, $value) = split(/=/, $pair);
		$name =~ s/^\s+//;
		$value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
		chomp($cookies{$name} = $value);
	}
}

my $sid = $cookies{knetsid};
if(length($sid) > 0){
	my $sql = qq{SELECT users.name FROM users LEFT JOIN sids ON users.id_user = sids.id_user WHERE sids.id_session = ?;};
	my $sth = $dbh->prepare($sql);
	$sth->execute($sid);

	my @row = $sth->fetchrow_array();
	my $name = $row[0];

	if(length($name) > 0){
		printf "OK $name";
	} else {
		&fail("Unauthorized");
	}
} else {
	&fail("Bad Sid");
}

exit 0
