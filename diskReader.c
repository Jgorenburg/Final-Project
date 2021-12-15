#include "fs/structs.h"
#include "format.h"
#include <stdio.h>
#include <errno.h>

int main(int argc, char *argv[]) {
	
	if (argc != 2) {
		perror("invalid number of args, should be 2\n");
		return 1;
	}	

	FILE *fp;
	fp = fopen(argv[1], "r");
	if (ferror(fp)) {
		printf("cannot open %s\n", argv[1]);
		return 1;
	}
	

	// reading the superblock
	struct superblock *sb = malloc(sizeof(struct superblock));
	fseek(fp, SBSIZE, SEEK_SET);
	fread(sb, sizeof(struct superblock), 1, fp);

	// reading the root dir
	char* root = malloc(sb->size);
	fseek(fp, 2 * SBSIZE, SEEK_SET);
	fread(root, sb->size, 1, fp);

	// reads all inodes
/*	int loc = OFFSET;
	while (loc < OFFSET + sb->data_offset * BLOCKSIZE) {
		
	}
*/
	
	// dirty implementation - reads the current 2 inodes
	struct inode *iroot = malloc(sizeof(struct inode));
	fseek(fp, OFFSET, SEEK_SET);
	fread(iroot, sizeof(struct inode), 1, fp);

	struct inode *iadmin = malloc(sizeof(struct inode));
	fseek(fp, OFFSET + sizeof(struct inode), SEEK_SET);
	fread(iadmin, sizeof(struct inode), 1, fp);
	
	struct inode *iguest = malloc(sizeof(struct inode));
	fseek(fp, OFFSET + 2 * sizeof(struct inode) , SEEK_SET);
	fread(iguest, sizeof(struct inode), 1, fp);
	
	struct inode *iempty = malloc(sizeof(struct inode));
	fseek(fp, OFFSET + 3 * sizeof(struct inode), SEEK_SET);
	fread(iempty, sizeof(struct inode), 1, fp);

	char * admin = malloc(sb->size);
	fseek(fp, iadmin->dblocks[0], SEEK_SET);
	fread(admin, sb->size, 1, fp);

	char * guest = malloc(sb->size);
	fseek(fp, iguest->dblocks[0], SEEK_SET);
	fread(guest, sb->size, 1, fp);
	
	

	free(sb);
	free(root);
	free(iroot);
	free(iempty);
	return 0;
}
