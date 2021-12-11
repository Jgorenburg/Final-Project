#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "fs.h"

#define ROOT '/'
#define DELIM "/"
#define MAX_OPEN_FILE_NUM 1000
#define INIT_SIZE 2

struct open_file open_files[MAX_OPEN_FILE_NUM]; // list of open files
struct disk_img *curr_disk_img = 0;
int fd_count = 0;
int root_inode = 0;
/* extern var */
extern int curr_fd;
extern int uid;

int f_open(const char *filename, const char *mode) {
	struct fileent file_in_dir;
	int return_file;

    char *path = malloc(strlen(filename)+1);
    char *path_parts = strtok(path, DELIM);

	if (*filename == ROOT) {
		// absolute path
		open_files[fd_count].inode = root_inode;
		open_files[fd_count].size = INIT_SIZE - 1;
		open_files[fd_count].mode = O_RDONLY;
	} else {
		// relative path
		if (open_files[curr_fd].inode == 0) {
			open_files[fd_count].size = INIT_SIZE - 1;
		} else {
			open_files[fd_count].size = INIT_SIZE;
		}
		open_files[fd_count].inode = open_files[curr_fd].inode;
		open_files[fd_count].mode = O_RDONLY;
	}

	int new_mode;
    if(strcmp(mode, "r") == 0) {
		new_mode =  O_RDONLY;
	} else if(strcmp(mode, "r+") == 0) {
		new_mode =  O_RDWR;
	} else if(strcmp(mode, "w") == 0) {
		new_mode =  O_WRONLY | O_CREAT | O_TRUNC;
	} else if(strcmp(mode, "w+") == 0) {
		new_mode =  O_RDWR | O_CREAT | O_TRUNC;
	} else if(strcmp(mode, "a") == 0) {
		new_mode =  O_WRONLY | O_CREAT | O_APPEND;
	} else if(strcmp(mode, "a+") == 0) {
		new_mode =  O_RDWR | O_CREAT | O_APPEND;
	} else {
        return -1;
    }

	while (path_parts) {
		file_in_dir = find_file_in_dir(fd_count, path_parts);
		if (file_in_dir.inode < 0) {
			// file not found
			if (strend(filename, path_parts) && (new_mode & O_CREAT)) {
				// create new file
				// what if the user creates a folder?
				file_in_dir.inode = create_file(fd_count, IS_FILE, path_parts, DEFAULT_FILE_PERMISSION);
				if (file_in_dir.inode < 0) {
					// fail to create the file
					free(path);
					return -1;
				}
			} else {
				free(path);
				return -1;
			}
		}
		
		open_files[fd_count].inode = file_in_dir.inode;
		if (file_in_dir.inode == 0) {
			open_files[fd_count].size = INIT_SIZE - 1;
		} else {
			open_files[fd_count].size = INIT_SIZE;
		}
		open_files[fd_count].mode = O_RDONLY;
		path_parts = strtok(NULL, DELIM);
	}

	open_files[fd_count].size = 0;
	open_files[return_file].mode = new_mode;
	if (uid != curr_disk_img->inodes[open_files[return_file].inode].uid) {
		// user has no permission
		free(path);
		return -1;
	}
	if(open_files[return_file].mode & O_APPEND) {
		open_files[return_file].size = curr_disk_img->inodes[open_files[return_file].inode].size;
	} else {
		open_files[return_file].size = 0;
	}
	int next_fd = increment_fd_count();
	if (next_fd = -1) {
		free(path);
		return -1;
	}
	free(path);
	return return_file;
}

struct fileent find_file_in_dir(int dir, char *filename) {
	struct fileent file_in_dir;
	file_in_dir.inode = -1;
	bzero(file_in_dir.file_name, NAME_LENGTH);

	if (curr_disk_img->inodes[open_files[dir].inode].type != IS_DIRECTORY) {
		return file_in_dir;
	}

	int old_size = open_files[dir].size;
	int size = curr_disk_img->inodes[open_files[dir].inode].size;
	if (open_files[dir].size < size) {
		// there are more sub-directories in this dir
		file_in_dir = f_readdir(dir);
	}
	while (open_files[dir].size < size && strncmp(filename, file_in_dir.file_name, NAME_LENGTH) != 0) {
		file_in_dir = f_readdir(dir);
	}
	if (strncmp(filename, file_in_dir.file_name, NAME_LENGTH) != 0) {
		// fild not found
		file_in_dir.inode = -1;
	}

	open_files[dir].size = old_size; // change dir size back
	return file_in_dir;
}

/* returns 1 if string t is present at the end of string s */
int strend(char *s, char *t) {
    if (strlen(s) >= strlen(t) && !strcmp (s+strlen(s)-strlen(t),t))
        return 1;
    return 0;
}

/* returns inode of new file */
int create_file(int dir, char type, char *filename, int permission) {
	int new_inode;
	new_inode = curr_disk_img->sb.free_inode;

	
}

/* returns -1 if number of open files greater than max open file num; returns 0 otherwise */
int increment_fd_count() {
	int old_fd_count = fd_count;
	fd_count = (fd_count + 1) % MAX_OPEN_FILE_NUM;
	while (open_files[fd_count].inode >= 0 && fd_count != old_fd_count) {
		// find available index in open_files
		fd_count = (fd_count + 1) % MAX_OPEN_FILE_NUM;
	}
	if (fd_count == old_fd_count) return -1;
	else return 0;
}
