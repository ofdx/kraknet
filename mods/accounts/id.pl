#!/usr/bin/perl
use strict;

my $database="accounts.db";

chomp(my $homepath=`mod_home accounts`);
chdir($homepath) or die "Can't get home";

chomp(my $auth=qx/mod_find accounts:auth/);
if($?!=0){ die "Bad Auth" }
$auth=~s/^OK (.*)$/\1/ or die "Can't grep username";

my $sql = qq/SELECT id_user FROM users WHERE name='$auth';/;
my $id_user = qx/sqlite3 "$database" "$sql"/;
if($?!=0){ die "Can't select id_user" }

printf "$id_user";
exit 0
