/*
	decode
	Mike Perron (2013)

	x2c and unescape_url copyright NCSA.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char x2c(char *what){
	register char digit;
	digit=(what[0]>='A'?((what[0]&0xdf)-'A')+10:(what[0]-'0'));
	digit*=16;
	digit+=(what[1]>='A'?((what[1]&0xdf)-'A')+10:(what[1]-'0'));
	return digit;
}
void unescape_url(char *url){
	register int x, y;
	for(x=0, y=0;url[y];++x, ++y){
		if((url[x]=url[y])=='%'){
			url[x]=x2c(&url[y+1]);
			y+=2;
		}
	}
	url[x]=0;
}

int main(int argc, char **argv){
	char *str=NULL, *s;
	size_t n;

	if(argc<2)
		getline(&str, &n, stdin);
	else {
		str=calloc(1+strlen(*(argv+1)), sizeof(char));
		strcpy(str, *(argv+1));
	}

	if(s=strpbrk(str, "\r\n"))
		*s=0;
	while(s=strchr(str, '+'))
		*s=' ';
	unescape_url(str);
	fputs(str, stdout);
	return 0;
}
