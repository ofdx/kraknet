#!/bin/sh
if [ -z "$1" ]
then
	echo "Usage: $0 <file.db>"
else
	echo "PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE users (user varchar(32) unique not null, hash varchar(128) not null, perm tinyint default(1), pretty varchar(128));
INSERT INTO "users" VALUES('example','.Z87Dy17nfg',1,'example');
COMMIT;
"|sqlite3 $1
fi
