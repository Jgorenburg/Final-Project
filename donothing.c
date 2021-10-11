#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
	printf("starting\n");
	int stallVal = 5;
	if (argc > 1){
		stallVal = atoi(argv[1]);
	}
	sleep(stallVal);
	printf("finished\n");
	return 0;
}
