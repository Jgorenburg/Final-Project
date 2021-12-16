#include "structs.h"

// formats directory entry as a string
int formatDir (struct filent* dir, char* output) {	
	sprintf(output, "%s\t%s\t%d\t%s\n", dir->perms, dir->user, dir->inode, dir->file_name);	
	return strlen(output);	
}

// like formatDir, but turn name into . and ..
int dotDir (struct filent* dir, char* output) {	
	sprintf(output, "%s\t%s\t%d\t.\n", dir->perms, dir->user, dir->inode);	
	return strlen(output);	
}

int dotdotDir (struct filent* dir, char* output) {	
	sprintf(output, "%s\t%s\t%d\t..\n", dir->perms, dir->user, dir->inode);	
	return strlen(output);	
}

