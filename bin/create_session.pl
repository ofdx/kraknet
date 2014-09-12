#!/usr/bin/perl
# Mike Perron (2014)
#
# Create a random session ID for a given user and return that value on stdout.
# SIDS are 64 characters, base-52 numbers encoded as [a-zA-Z]. This should give
# an extremely small chance for collision.

use strict;
use DBI;

my $user = $ARGV[0];
die "No user specified" if(!$user);

my $dbh = DBI->connect('dbi:mysql:kws', 'kraknet', '') or die "Could not access DB";

my $sql = qq{SELECT id_user FROM users WHERE name = ?;};
my $sth = $dbh->prepare($sql);
$sth->execute($user);

my @row = $sth->fetchrow_array();
my $uid = $row[0];
die "Bad username" if(!$uid);

$sql = qq{SELECT users.name FROM users LEFT JOIN sids ON users.id_user = sids.id_user WHERE sids.id_session = ?;};
$sth = $dbh->prepare($sql);

while(1){
	my $sid = qx{tr -dc "[:alpha:]" < /dev/urandom | head -c 64};
	$sth->execute($sid);

	if(!$sth->fetchrow_array()){
		$sql = qq{INSERT INTO sids(id_session, id_user) values(?, ?);};
		$sth = $dbh->prepare($sql);
		$sth->execute($sid, $uid);

		print "$sid";
		exit 0
	}
}
