#include "format.h"

int main (int argc, char *argv[]) {
	if (argc < 2) {
		printf("too few arguments\n");
		return 1;
	}
	else if (argc > 2) {
		printf("too many arguments\n");
		return 1;
	}

	char* fileName = argv[1];
	FILE *fp;

	fp = fopen(fileName, "w");
	if (fp < 0) {
		printf("error: cannot open file %s\n", fileName);
		return 1;
	} 

	// makes our file the correct size
	// code from https://www.linuxquestions.org/questions/programming-9/how-to-create-a-file-of-pre-defined-size-in-c-789667/
	int fileSize = DEFAULTSIZE;

	int result = fseek(fp, fileSize-1, SEEK_SET);
	if (result == -1) {
		fclose(fp);
		perror("Error calling lseek() to 'stretch' the file");
		return 1;
	}
	result = fwrite("", 1,  1, fp);
	if (result < 0) {
		fclose(fp);
		perror("Error writing a byte at the end of the file");
		return 1;
	}			
	fseek(fp, 0, SEEK_SET);

	struct inode* init_inode;
	// size of the initial inode the full inode space
	init_inode = (struct inode*) malloc ((fileSize - OFFSET) / 7);
	init_inode->nlink = 0;
	init_inode->size = (fileSize - OFFSET) / 7;	
	time_t time = clock();
	init_inode->ctime = time;
	init_inode->mtime = time;
	init_inode->atime = time;		
	init_inode->next_free = ENDLIST; 

	struct superblock* sb;
	sb = (struct superblock*) malloc (SBSIZE); 
	sb->size = BLOCKSIZE;
	sb->inode_offset = IOFFSET;
	// inodes get 1/7 of the remaining space
	sb->data_offset = IOFFSET + ((fileSize - OFFSET) / 7) / BLOCKSIZE;
	// swap gets the final block
	sb->swap_offset = ((fileSize - OFFSET) / BLOCKSIZE) - 1;
	sb->free_inode = 2 * SBSIZE + sb->inode_offset * BLOCKSIZE;
	sb->free_block = 2 * SBSIZE + sb->data_offset * BLOCKSIZE;		

	
					
	fseek(fp, SBSIZE, SEEK_SET);
	fwrite(sb, SBSIZE, 1, fp);
	fseek(fp, sb->inode_offset, SEEK_SET);
	fwrite(init_inode, init_inode->size, 1, fp);
	free(sb);
	free(init_inode);	

	fclose(fp);
	return 0;
}
