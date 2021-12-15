#include "../format.h"
#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h> 
int pwd_fd;
int uid = 0;

void test_f_open() {
	struct fileent *new_dir;
	new_dir->file_name[0] = "new.txt";

	int result;
	int fd;

	// formatDir(new_dir, );

}