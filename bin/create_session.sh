#!/bin/bash
# Mike Perron (2013)
#
# Create a random session ID for a given user and return that value on stdout.
# SIDS are 64 characters, base-52 numbers encoded as [a-zA-Z]. This should give
# an extremely small chance for collision.
if [ -z "$1" ]; then exit 1; fi

user=$1
sql="SELECT id_user FROM users WHERE name='$user';"
database="`mod_home accounts`/accounts.db"

if ! uid=$(sqlite3 "$database" "$sql"); then exit 1; fi
if [ -z "$uid" ]; then exit 1; fi

# 64-character string, alphabet as a base-52 number set.
while :; do
	sid=$(tr -dc "[:alpha:]" < /dev/urandom | head -c 64)
	sql="SELECT users.name FROM users LEFT JOIN sids ON users.id_user = sids.id_user WHERE sids.id_session='$sid';"

	if [ -z "$(sqlite3 "$database" "$sql")" ]; then 
		sql="INSERT INTO sids(id_session, id_user) values('$sid', '$uid');"
		if ! sqlite3 "$database" "$sql"; then exit 1; fi
		printf "$sid"
		exit 0
	fi
done
