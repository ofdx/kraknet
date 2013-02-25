#!/bin/sh
if [ -z "$1" ]
then
	echo "Usage: $0 <file.db>"
else
	echo "PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE sids
(
sid varchar(128) unique not null,
user varchar(32) not null,
date datetime default current_timestamp,
ip varchar(32),
ua varchar(255)
);
COMMIT;
"|sqlite3 $1
fi
