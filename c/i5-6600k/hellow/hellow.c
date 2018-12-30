#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>		//sleep
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
#include <ctype.h> 		//isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

// #include "hellow.h"
#include "core.h"

int main(int argc, char *argv[]){

	int			c;

	printf("hellow\n");
	c = getc(stdin);
	if (c == '\033') { 	// if the first value is esc
	    getc(stdin); 				// skip the '['
	    c = getc(stdin);
	    switch(c) { 	// the real value
	        case 'A':
	            printf("up arrow\n");
	            break;
	        case 'B':
	            printf("down arrow\n");
	            break;
	        case 'C':
	            printf("right arrow\n");
	            break;
	        case 'D':
	            printf("left arrow\n");
	            break;
	    	}
		}
	else {
		if(c == _ESC)
			exit(1);
		else
			exit(0);

	}
}
