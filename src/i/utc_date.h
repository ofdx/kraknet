/*
	utc_date
	Written by Mike Perron (2012)

	Gets the time for Kraknet posts. The format is always four digit year, two
	digit month, two digit day of the month, two digit hour, two digit minute,
	two digit second, plus nine digits for nanoseconds. This replaces a pipe to
	date(1) (`date +%Y%m%d%H%M%S%N`).

	Note that post_time allocates 32 bytes of memory for the formatted string
	and returns a pointer. This memory should be freed after use.
*/
#ifndef KRAKNET_UTC_DATE_H
#define KRAKNET_UTC_DATE_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define KRAKNET_POST "%Y%m%d%H%M%S"
#define KRAKNET_TIMESTAMP "%l:%M:%S %P"

extern char *post_time(const char *fmt,int nano);

#endif
