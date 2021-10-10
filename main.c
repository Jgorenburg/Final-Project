#include "main.h"

void main() {
	joblist = (struct LinkedList*)malloc(sizeof(*joblist));

	while(1) {
		parserMain();	
		execute();	
	}
}
