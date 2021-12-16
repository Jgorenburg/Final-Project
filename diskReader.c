#include "structs.h"
#include "format.h"
#include <stdio.h>
#include <errno.h>


// program to read a disk (must use gdb)
// Steps: 
// 1. to run - gdb diskread
// 2. make a breakpoint - b 91
// 3. start [name of your disk]
// 4. continue
// 5. at breakpoint, can read superblock and all disks, along with the portion of
// 	files in the dblocks

void tester(struct diskimage *di) {
	di->id = 1;
}

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
	
	struct diskimage* test = malloc(sizeof(struct diskimage));
	tester(test);	

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
	// clean implementation - reads all inodes
	int numInodes = ((sb->data_offset - sb->inode_offset) * sb->size) / sizeof(struct inode) + 1;
	struct inode* allInodes[numInodes];
	char *allData[numInodes];
	int loc = OFFSET;	

	for (int i = 0; i < numInodes; i++) {
		struct inode *new_inode = malloc(sizeof(struct inode));
		fseek(fp, loc, SEEK_SET);
		fread(new_inode, sizeof(struct inode), 1, fp);
		allInodes[i] = new_inode;

		// currently only reading data from dblocks
		if (new_inode->size > 0) {
			int roundSize = new_inode->size + sb->size - (new_inode->size % sb->size);
			allData[i] = malloc(roundSize);
			for (int j = 0; j < roundSize; j += sb->size) {
				if (j / sb->size < N_DBLOCKS) {
					fseek(fp, (3 + sb->data_offset + new_inode->dblocks[j / sb->size]) * sb->size, SEEK_SET);
					fread(allData[i] + j, sb->size, 1, fp); 
				}
			}
		}

		loc += sizeof(struct inode);
	}		


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

	char * admin = malloc(sb->size);
	fseek(fp, iadmin->dblocks[0], SEEK_SET);
	fread(admin, sb->size, 1, fp);

	char * guest = malloc(sb->size);
	fseek(fp, iguest->dblocks[0], SEEK_SET);
	fread(guest, sb->size, 1, fp);
	
	
	for (int i = 0; i < numInodes; i++) {
		if (allInodes[i]->size > 0) {
			free(allData[i]);
		}
		free(allInodes[i]);
	} 

	free(sb);
	free(root);
	free(admin);
	free(guest);
	free(iroot);
	free(iguest);
	free(iadmin);
	return 0;
}
