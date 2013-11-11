/*
	mod_home
	Mike Perron (2013)

	Usage: mod_home <module name>

	If module name is specified, prints the path to the module.
	Otherwise prints the path to the mods folder.
*/
#include <stdio.h>
#include <stdlib.h>

#include "generic.h"

int main(int argc, char **argv){
	printf("%s\n", (argc<2)?mod_home(""):mod_home(*(argv+1)));
	return 0;
}
