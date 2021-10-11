#include "parser.h"

// initialize argArray
void init_arg() {
	for (i=0;i<MAX_CMD;i++) {
		for (j=0;j<MAX_CMD_LEN;j++) {
			argArray[i][j]='\0';
		}
	}
}

// checks if character is a specials symbol
// return 0 if it is, else return 1
int special_symbol(char item) {
	for (int i=0;i<strlen(specialChars)-1;i++) {
		if (item==specialChars[i]) {return 0;}
	}
	return 1;
}

// return 1 if input is invalid (skip to next input)
// else return 0
int parse(char* buf) {
	argc=0; // initialization
	if (buf==NULL || buf[0] == EOF) {return 1;}
	for (i = 0, j = 0; i < strlen(buf); i++) {
		if (special_symbol(buf[i])==0) {
			if (j != 0) {
				argArray[argc++][j] = '\0';
				j = 0;
			}
			argArray[argc++][j] = buf[i];
		}
		else if (buf[i] != ' ') {argArray[argc][j++] = buf[i];} 
		else {
			if (j != 0) {
				argArray[argc++][j] = '\0';
				j = 0;
			}
		}
	}
	if (j != 0) {argArray[argc++][j] = '\0';}
	return 0;
}

int parserMain() {
	char* buffer;
	// while (1) {
	//    char* buffer;
	buffer=readline("\n>> ");
	init_arg();
	if (parse(buffer)==1) {printf("\n");}    // handle ctrl-d and parse input
	free(buffer);
	// }
	return 0;
}

