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


	// makes superblock
	struct superblock* sb;
	sb = (struct superblock*) malloc (sizeof(struct superblock)); 
	sb->size = BLOCKSIZE;
	sb->inode_offset = IOFFSET;
	// inodes get 1/7 of the remaining space
	sb->data_offset = IOFFSET + ((fileSize - OFFSET) / 7) / BLOCKSIZE;
	// swap gets the final block
	sb->swap_offset = ((fileSize - OFFSET) / BLOCKSIZE) - 1;
	sb->free_inode = OFFSET + sb->inode_offset * BLOCKSIZE;
	sb->free_block = OFFSET + sb->data_offset * BLOCKSIZE;		

	// making root directory
	struct dirent* rootdir = malloc (sizeof(struct dirent));
	rootdir->file_name = "/";
	rootdir->inode = 1;
	rootdir->user = "root";
	rootdir->perms = "----------";

	// making admin and guest account directories
	
	struct dirent* admindir = malloc (sizeof(struct dirent));
	admindir->file_name = "admin";
	admindir->inode = 1;
	admindir->user = "admin";
	admindir->perms = "----------";	

	struct dirent* guestdir = malloc (sizeof(struct dirent));
	guestdir->file_name = "guest";
	guestdir->inode = 2;
	guestdir->user = "guest";
	guestdir->perms = "----------";	

	// . and .. are made by function calls for dot(curdir, dotdir) and dotdot(curdir, dotdotdir)	


	time_t time = clock();

	// making inode for the root dir
	struct inode* root_inode = malloc (sizeof(struct inode));
	root_inode->nlink = 2;
	root_inode->next_free = 0;
	//TODO: root_inode->protection
	root_inode->type = 1;
	root_inode->size = sizeof(struct dirent);
	//root_inode->uid
	//root_inode->gid
	root_inode->ctime = time;	
	root_inode->mtime = time;	
	root_inode->atime = time;	
	root_inode->dblocks[0] = 2 * SBSIZE;

	// making inodes for admin and guest

	struct inode* admin_inode = malloc (sizeof(struct inode));
	admin_inode->nlink = 18;
	admin_inode->next_free = 0;
	//TODO: root_inode->protection
	admin_inode->type = 1;
	admin_inode->size = sizeof(struct dirent);
	//root_inode->uid
	//root_inode->gid
	admin_inode->ctime = time;	
	admin_inode->mtime = time;	
	admin_inode->atime = time;	
	admin_inode->dblocks[0] = OFFSET + sb->data_offset * sb->size;

	struct inode* guest_inode = malloc (sizeof(struct inode));
	guest_inode->nlink = 19;
	guest_inode->next_free = 0;
	//TODO: root_inode->protection
	guest_inode->type = 1;
	guest_inode->size = sizeof(struct dirent);
	//root_inode->uid
	//root_inode->gid
	guest_inode->ctime = time;	
	guest_inode->mtime = time;	
	guest_inode->atime = time;	
	guest_inode->dblocks[0] = OFFSET + (sb->data_offset + 1) * sb->size;


	// makes inode for all empty disk space
	struct inode* init_inode;
	// size of the initial inode the full inode space
	init_inode = (struct inode*) malloc (sizeof(struct inode));
	init_inode->nlink = 20;
	init_inode->size = ((fileSize - OFFSET) / 7) - root_inode->size - admin_inode->size - guest_inode->size;	
	init_inode->ctime = time;
	init_inode->mtime = time;
	init_inode->atime = time;		
	init_inode->next_free = ENDLIST; 
	printf("%d\n", init_inode->size);
	
	// writing the superblock					
	fseek(fp, SBSIZE, SEEK_SET);
	fwrite(sb, SBSIZE, 1, fp);
	
	// writing the root directory
	char *root = (char *) malloc (5 * NAME_LENGTH);
	// start with . and ..
	int dirLen = formatDir(rootdir, root);
	fseek(fp, 2 * SBSIZE, SEEK_SET);
	fwrite(root, dirLen, 1, fp);
	dirLen = formatDir(rootdir, root);
	fwrite(root, dirLen, 1, fp);
	dirLen = formatDir(admindir, root);	
	fwrite(root, dirLen, 1, fp);
	dirLen = formatDir(guestdir, root);
	fwrite(root, dirLen, 1, fp);
	free(root);

	// writing guest and admin directories	
	char *admin = (char *) malloc (5 * NAME_LENGTH);
	dirLen = formatDir(admindir, admin);
	fseek(fp, admin_inode->dblocks[0], SEEK_SET);
	fwrite(admin, dirLen, 1, fp);
	dirLen = formatDir(rootdir, admin);
	fwrite(admin, dirLen, 1, fp);
	free(admin);

	char *guest = (char *) malloc (5 * NAME_LENGTH);
	dirLen = formatDir(guestdir, guest);
	fseek(fp, guest_inode->dblocks[0], SEEK_SET);
	fwrite(guest, dirLen, 1, fp);
	dirLen = formatDir(rootdir, guest);
	fwrite(guest, dirLen, 1, fp);
	free(guest);
	
	// writing the root's inode
	fseek(fp, OFFSET, SEEK_SET);
	fwrite(root_inode, sizeof(struct inode), 1, fp);

	// writing the empty inode
	fseek(fp, OFFSET + 3 * sizeof(struct inode), SEEK_SET);
	fwrite(init_inode, sizeof(struct inode), 1, fp);

	// writing guest and admin info
	fseek(fp, OFFSET + sizeof(struct inode), SEEK_SET);
	fwrite(admin_inode, sizeof(struct inode), 1, fp);
	fseek(fp, OFFSET + 2 * sizeof(struct inode), SEEK_SET);
	fwrite(guest_inode, sizeof(struct inode), 1, fp); 	
	
	fseek(fp, OFFSET + sb->size * sb->data_offset, SEEK_SET);
	fwrite(admindir, sizeof(struct dirent), 1, fp);
	fseek(fp, OFFSET + sb->size * (1 + sb->data_offset), SEEK_SET);
	fwrite(guestdir, sizeof(struct dirent), 1, fp);

	free(sb);
	free(init_inode);	
	free(root_inode);	
	free(admin_inode);
	free(guest_inode);
	free(guestdir);
	free(admindir);
	free(rootdir);

	fclose(fp);
	return 0;
}
