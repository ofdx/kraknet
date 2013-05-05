/*
	utc_date
	Written by Mike Perron (2012)

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

	Gets the time for Kraknet posts. The format is always four digit year, two
	digit month, two digit day of the month, two digit hour, two digit minute,
	two digit second, plus nine digits for nanoseconds. This replaces a pipe to
	date(1) (`date +%Y%m%d%H%M%S%N`).

	Note that post_time allocates 32 bytes of memory for the formatted string
	and returns a pointer to a statically allocated buffer.
*/
#include "utc_date.h"

char *post_time(const char *fmt,int nano){
	static char *str=NULL;
	struct timespec ts;
	time_t t;
	struct tm *tmp;
	long int li;
	int count, l;

	if(!str)
		str=calloc(32,sizeof(char));

	clock_gettime(CLOCK_REALTIME, &ts);
	t=ts.tv_sec;
	tmp=localtime(&t);
	strftime(str,32,fmt,tmp);
	if(nano)
		sprintf(str,"%s%09ld",str,ts.tv_nsec);
	/* The infamous "double post" bug stemmed from this line of code.
	 * Previously, it was only %ld instead of %09ld, meaning that prepending
	 * zeroes were not added, and the string was truncated. For values of
	 * nanoseconds less than 100000000 (about 10% of all time), the string was
	 * the wrong length, and the magnitude of the nanoseconds was apparently
	 * ten times greater. */

	return str;
}
