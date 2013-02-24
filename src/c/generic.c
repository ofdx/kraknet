/*
	generic
	Written by Mike Perron (2012-2013)

	General functions to patch things together for krakws.
*/
#include "generic.h"

/*	Splits words at spaces and tabs. This function creates a copy of the
	original string. The first word may be the same as the second, and should
	only be used to free memory. The last pointer will be NULL, signifying the
	end of the string. */
char **chop_words(const char *src){
	char **v=NULL,*str,*r;
	int count=2;

	str=calloc(1+strlen(src),sizeof(char));
	*(v=malloc(count*sizeof(char**)))=str;
	strcpy(str,src);

	*(v+count-1)=strtok_r(str,WORDS_DELIMINATOR,&r);
	do v=realloc(v,++count*sizeof(char**));
	while(*(v+count-1)=strtok_r(NULL,WORDS_DELIMINATOR,&r));

	return v;
}


char x2c(char *what){
	register char digit;
	digit=(what[0]>='A'?((what[0]&0xdf)-'A')+10:(what[0]-'0'));
	digit*=16;
	digit+=(what[1]>='A'?((what[1]&0xdf)-'A')+10:(what[1]-'0'));
	return digit;
}

void unescape_url(char *url){
	register int x,y;
	for(x=0,y=0;url[y];++x,++y){
		if((url[x]=url[y])=='%'){
			url[x]=x2c(&url[y+1]);
			y+=2;
		}
	}
	url[x]=0;
}

void sanitize_str(char *str){
	do if(*str=='\r'||*str=='\n')
		break;
	while(*(str++));
	*str=0;
}

void unquote_str(char *str){
	size_t l=strlen(str);

	if(*str=='\"')
		memmove(str,str+1,l--);

	if(*(str+(--l))=='\"')
		*(str+l)=0;
}

/*	Opens a stream to $conf_dir/cname */
FILE *get_conf_stream(char *cname,const char *mode){
	FILE *stream;
	char *str,*a;

	if(!(a=getenv("conf_dir")))
		return NULL;

	str=calloc(strlen(cname)+strlen(a)+2,sizeof(char));
	sprintf(str,"%s/%s",a,cname);

	stream=fopen(str, mode);
	free(str);

	return stream;
}

/*	If you only need to read one value from a configuration file, this function
 *	is better than get_conf_line_s, because it handles the stream itself. */
char *get_conf_line(char *fname,char *value){
	char *return_string;
	FILE *stream;

	if(!(stream=fopen(fname,"r")))
		return NULL;

	return_string=get_conf_line_s(stream,value,SEEK_FORWARD_ONLY);

	fclose(stream);
	return return_string;
}

/*	This function returns a pointer to a statically allocated buffer containing
 *	the arguments (following a '=') from a line in a stream. There are two mode
 *	possibilities: SEEK_FORWARD_ONLY will search forward from the file pointers
 *	current position; SEEK_RESET_OK allows the function to search the remainder
 *	of the file, reset the pointer to the beginning, and search from wherever it
 *	started. SEEK_RESET_OK is optimal for reading actual config files multiple
 *	times. Do not use on non-file streams. */
char *get_conf_line_s(FILE *stream, char *value, char mode){
	static char *str=NULL;
	static size_t n=256;

	char *return_string=NULL;
	char *a,*s;
	ssize_t r;

	long initial_file_pos=ftell(stream);

	if(!str)
		str=calloc(n,sizeof(char));

	do{	r=getline(&str,&n,stream);

		// EOF handling and looping.
		if(feof(stream) || r<0)
			if(mode==SEEK_RESET_OK && initial_file_pos){
				rewind(stream);
				mode=SEEK_POST_REWIND;
			} else break;
		if(mode==SEEK_POST_REWIND && ftell(stream)>=initial_file_pos)
			break;

		// Skip comments
		if(*str=='#')
			continue;
		sanitize_str(str);

		// Skip indentation
		for(s=str;*s==' '||*s=='\t';s++);
		a=s;

		// Match needle to the beginning of the line.
		if(a==strcasestr(a,value)){
			for(s=a;*s;s++)
				if(*s=='=')
					break;
			a=(*s)?s+1:s;

			return_string=str;
			strcpy(str,a);
		}
	}	while(!return_string);

	unquote_str(return_string);

	return return_string;
}
