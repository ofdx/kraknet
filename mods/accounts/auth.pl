#!/usr/bin/perl
# Mike Perron (2013)
#
# Verify that a given session is valid.

my $homepath=`mod_home accounts`;
my $database="accounts.db";

sub fail {
	printf "NO $_[0]";
	exit 0
}

chomp($homepath);
chdir($homepath) or &fail("Error");

my $buffer=$ENV{"HTTP_COOKIE"};
my %cookies;
if(length($buffer)>0){
	my @pairs=split(/[;&]/, $buffer);
	foreach my $pair (@pairs){
		my ($name, $value) = split(/=/, $pair);
		$name =~ s/^\s+//;
		$value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
		chomp($cookies{$name} = $value);
	}
}

my $sid=$cookies{"knetsid"};
if(length($sid)>0){
	my $sql="SELECT users.name FROM users LEFT JOIN sids ON users.id_user = sids.id_user WHERE sids.id_session='$sid';";
	my $name=qx{sqlite3 "$database" "$sql"};
	if(length($name)>0){
		printf "OK $name";
	} else {
		&fail("Unauthorized");
	}
} else {
	&fail("Bad Sid");
}

exit 0
