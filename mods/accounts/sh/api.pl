#!/usr/bin/perl
# api
# mperron (2015)
#
# Multi-functional script which provides a JSON-based stream API for the
# accounts management system.

use strict;
use JSON;
use DBI;

my $dbh = DBI->connect('dbi:mysql:kws', 'kraknet', '') or &fail("could not access DB");

my $postdata = <STDIN>;
my $postvalues = decode_json $postdata;
my $okay = 0;
my $error;
my $redir;

# Store the response object here. It's what gets converted to JSON and dumped out.
my $response;

my $op = $postvalues->{op};

# Rough estimate of acceptable domain names. Probably fine.
my $domain = $postvalues->{domain};
if(!($domain =~ /^[a-zA-Z0-9.-]{3,}$/)){
	$domain = "";
}

# What should we do?
if($op eq "login"){
	&login();
} elsif($op eq "register"){
	&register();
} elsif($op eq "logout"){
	&logout();
}

# Dump out JSON text.
print encode_json $response;

exit 0;

sub logout {
	my $cookie = $ENV{HTTP_COOKIE};
	my %cookies;
	if(length($cookie) > 0){
		my @pairs = split(/[;&]/, $cookie);
		foreach my $pair (@pairs){
			my ($name, $value) = split(/=/, $pair);
			$name =~ s/^\s+//;
			$value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
			chomp($cookies{$name} = $value);
		}
	}

	my $sid = $cookies{knetsid};
	if(!($sid =~ /^[a-zA-Z]*$/)){
		$response->{success} = 0;
		$response->{message} = "Bad Session ID";
		return
	}

	my $sql = qq/DELETE FROM sids WHERE id_session = ?;/;
	my $sth = $dbh->prepare($sql);

	my $logstat = "Failed";
	if(length($sid) > 0){
		$sth->execute($sid);
		if(!$?){ $logstat = "Successful" }	
	}

	my $ref = $ENV{HTTP_REFERER};
	$ref = "/" unless($ref);

	$response->{success} = 1;
	$response->{message} = "Logout successful.";
	$response->{redir} = $ref;
}

sub register {
	my $reg_okay = 0;
	my %okay;
	my $pwhash;
	my $reg_error = "";

	$okay{name} = 1 if($postvalues->{name} =~ /^[a-z\d_\-]{4,128}+$/);

	my $pw_len = length $postvalues->{pass};
	if(($postvalues->{pass} eq $postvalues->{repass}) and ($pw_len >= 6)){
		$okay{pass} = 1;

		my $pass = $postvalues->{pass};
		$pass =~ s/'/'"'"'/g;

		chomp($pwhash = qx/echo '$postvalues->{name}$pass' | openssl sha512/);
		$pwhash =~ s/^.*\s([a-fA-F0-9]*)$/\1/g;
	} else {
		$reg_error = ($pw_len < 6) ? "Password too short." : "Passwords do not match.";
	}

	if(($okay{name} == 1) and ($okay{pass} == 1)){
		$reg_okay = 1;

		my $sql = qq/SELECT name FROM users WHERE name = ?;/;
		my $sth = $dbh->prepare($sql);
		$sth->execute($postvalues->{name});

		if(!$sth->fetchrow_array()){
			$sql = qq/INSERT INTO users(name, hash, pretty) values(?, ?, ?);/;
			$sth = $dbh->prepare($sql);
			$sth->execute($postvalues->{name}, $pwhash, $postvalues->{name});
		} else {
			$reg_okay = 0;
			$reg_error = "That username is already taken.";
		}
	}

	$response->{success} = $reg_okay;

	my $redir;
	if($reg_okay == 1){
		$redir = $postvalues->{onsuccess};
		if($redir eq ""){
			$redir = "/";
		}

		# Registration worked, so there's no reason create_session should fail...right?
		$response->{sid} = qx/create_session '$postvalues->{name}'/;
		$response->{domain} = $domain;
		$response->{redir} = $redir;
		$response->{message} = "Registration completed successfully.";
	} else {
		# Failed registration.
		$response->{message} = $reg_error;
	}
}

sub login {
	my $username = $postvalues->{name};
	$username =~ s/\\/\\\\/g;
	$username =~ s/(["'])/\\\1/g;

	if(length($username) > 0){
		my $pass = $postvalues->{pass};
		$pass =~ s/'/'"'"'/g;

		my $sql = qq/SELECT hash FROM users WHERE name = ?;/;
		my $sth = $dbh->prepare($sql);

		chomp(my $pwhash = qx/echo '$username$pass' | openssl sha512/);
		$pwhash =~ s/^.*\s([a-fA-F0-9]*)$/\1/g;

		$sth->execute($username);
		my @row = $sth->fetchrow_array();
		my $buffer = $row[0];

		if((length($buffer) > 0) and ($pwhash eq $buffer)){
			$okay = 1;
			$error = "Credentials accepted.";
		} else {
			# Passwords no good.
			$okay = 0;
		}
	} else {
		# Impossible user name.
		$okay = 0;
	}

	if($okay == 1){
		$redir = $postvalues->{onsuccess};
		if($redir eq ""){
			$redir = "/";
		}
	} else {
		$error = "Invalid username or password.";
		$redir = $postvalues->{onfailure};
		if($redir eq ""){
			$redir = $ENV{HTTP_REFERER};
			if ($redir eq ""){
				$redir = "/";
			}
		}
	}

	# Build reponse object.
	$response->{message} = $error;
	$response->{success} = $okay;
	$response->{redir} = $redir;

	# Only add the session and send the cookie if auth was OK.
	if($okay == 1){
		chomp(my $sid = qx/create_session '$username'/);
		if(($? == 0) and (length($sid) > 0)){
			$response->{domain} = $domain;
			$response->{sid} = $sid;
			$response->{name} = $username;
		}
	}
}
