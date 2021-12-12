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

/* library functions */
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
	int next_fd = increase_fd_count();
	if (next_fd = -1) {
		free(path);
		return -1;
	}
	free(path);
	return return_file;
}

size_t f_read(void *ptr, size_t size, size_t nmemb, int fd) {
	// invalid fd
	if (open_files[fd].inode < 0) return -1;
	if (fd < 0 || fd >= MAX_OPEN_FILE_NUM) return -1;
	// user has no access
	if (curr_disk_img->inodes[open_files[fd].inode].permission & PERMISSION_R) return -1;
	// file can't be read
	if (!(open_files[fd].mode & O_RDONLY) && !(open_files[fd].mode & O_RDWR)) return -1;

	size_t total_size = size * nmemb;
	int file_size = open_files[fd].size;
	int block_size = curr_disk_img->sb.size;

	size_t curr_offset = 0;
	int read = 0; // read size
	int first = 0;

	if (file_size > 0) {
		first = file_size / block_size;
		size_t extra = file_size % block_size;
		if (extra > 0) {
			size_t new_size = block_size - extra;
			if (total_size < new_size) new_size = total_size;
			struct datablock tmp_block = get_data(open_files[fd].inode, first);
			memcpy(ptr + curr_offset, tmp_block.data + extra, new_size);
			free(tmp_block.data);
			total_size -= new_size;
			curr_offset += new_size;
			read += new_size;
			first++;
		}
	}

	// the rest of data
	size_t rest;
	int block_num = total_size / block_size;
	if (total_size % block_size > 0) block_num++;
	for (int i = first; i < first + block_num; i++) {
		if (total_size <= 0) break;

		struct datablock tmp_block = get_data(open_files[fd].inode, i);
		if (tmp_block.data == 0) break;

		rest = total_size % block_size;
		if (rest == 0) rest += block_size;

		memcpy(ptr + curr_offset, tmp_block.data, rest);
		free(tmp_block.data);
		total_size -= rest;
		curr_offset += rest;
		read += rest;
	}
	open_files[fd].size += nmemb * size;
	return read;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, int fd) {
	// invalid fd
	if (open_files[fd].inode < 0) return -1;
	if (fd < 0 || fd >= MAX_OPEN_FILE_NUM) return -1;
	// user has no access
	if (curr_disk_img->inodes[open_files[fd].inode].permission & PERMISSION_W) return -1;
	// file can't be read
	if (!(open_files[fd].mode & O_WRONLY) && !(open_files[fd].mode & O_RDWR)) return -1;

	size_t write_size = calculate_write_size();
	open_files[fd].size += write_size;

	struct inode *curr = &((curr_disk_img->inodes)[open_files[fd].inode]);
	if (open_files[fd].size + write_size > curr->size) {
		curr->size = open_files[fd].size + write_size;
		update_inode(open_files[fd].inode);
	}
	return write_size;
}

struct fileent find_file_in_dir(int dir, char *filename) {
	struct fileent file_in_dir;
	file_in_dir.inode = -1;
	bzero(file_in_dir.file_name, NAME_LENGTH);

	int old_size = open_files[dir].size;
	int size = curr_disk_img->inodes[open_files[dir].inode].size;

	if (curr_disk_img->inodes[open_files[dir].inode].type != IS_DIRECTORY) {
		return file_in_dir;
	}
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

	// update superblock
	curr_disk_img->sb.free_inode = curr_disk_img->inodes[curr_disk_img->sb.free_inode].next_free;
	update_sb();

	// update inode
	curr_disk_img->inodes[new_inode].type = type;
	curr_disk_img->inodes[new_inode].protect = 0;
	curr_disk_img->inodes[new_inode].nlink = 1;
	curr_disk_img->inodes[new_inode].size = 0;
	curr_disk_img->inodes[new_inode].uid = uid;
	curr_disk_img->inodes[new_inode].gid = uid;

	for (int i = 0; i < N_DBLOCKS; i++) {
		curr_disk_img->inodes[new_inode].dblocks[i] = 0;
	}
	for(int i = 0; i < N_IBLOCKS; i++) {
		curr_disk_img->inodes[new_inode].iblocks[i] = 0;
	}

	curr_disk_img->inodes[new_inode].i2block = 0;
	curr_disk_img->inodes[new_inode].i3block = 0;

	curr_disk_img->inodes[new_inode].parent = open_files[dir].inode;
	curr_disk_img->inodes[new_inode].next_free = 0;
	curr_disk_img->inodes[new_inode].permission = permission;

	update_inode(open_files[dir].inode);
	return new_inode;
}

/* returns -1 if number of open files greater than max open file num; returns 0 otherwise */
int increase_fd_count() {
	int old_fd_count = fd_count;
	fd_count = (fd_count + 1) % MAX_OPEN_FILE_NUM;
	while (open_files[fd_count].inode >= 0 && fd_count != old_fd_count) {
		// find available index in open_files
		fd_count = (fd_count + 1) % MAX_OPEN_FILE_NUM;
	}
	if (fd_count == old_fd_count) return -1;
	else return 0;
}

/* update superblock */
void update_sb() {
	lseek(curr_disk_img->fd, SUPER_OFFSET, SEEK_SET);
	write(curr_disk_img->fd, &(curr_disk_img->sb), sizeof(struct superblock));
}

/* update a single inode */
void update_inode(int inode) {
	lseek(curr_disk_img->fd, INODE_OFFSET + curr_disk_img->sb.inode_offset * curr_disk_img->sb.size + inode * sizeof(struct inode), SEEK_SET);
	write(curr_disk_img->fd, &(curr_disk_img->inodes[inode]), sizeof(struct inode));
}

/* get data from datablock */
struct datablock get_data(int inode, int block_num) {
	struct inode *curr = &(curr_disk_img->inodes[inode]);

	// dblock
	if (block_num < N_DBLOCKS) {
		int datablock = curr->dblocks[block_num];
		return get_dblock(datablock);
	}

    // iblock
	block_num -= N_DBLOCKS;
	int pointer_num = curr_disk_img->sb.size / INT_SIZE;
    if (block_num < N_IBLOCKS * pointer_num) {
        int count = block_num / pointer_num;
        if (block_num % pointer_num != 0) {
            count++;
        }
        return get_iblock(curr->iblocks[count], block_num);
    }

    // i2block
    block_num -= N_IBLOCKS * pointer_num;
    if (block_num < pointer_num * pointer_num) {
        return get_i2block(curr->i2block, &block_num);
    }

    // i3block
    block_num -= pointer_num * pointer_num;
    if (block_num < pointer_num * pointer_num * pointer_num) {
        return get_i3block(curr->i3block, &block_num);
    }

    // file or dir doesn't exist
    struct datablock null_db;
    null_db.data = 0;
    null_db.address = -1;
    return null_db;
}

/* datablock is the address of datablock, returns a datablock struct */
struct datablock get_dblock(int datablock) {
	struct datablock new;
	lseek(curr_disk_img->fd, INODE_OFFSET + (curr_disk_img->sb.data_offset + datablock)*curr_disk_img->sb.size, SEEK_SET);

	void *data = malloc(curr_disk_img->sb.size);
	read(curr_disk_img->fd, data, curr_disk_img->sb.size);
	new.data = data;
	new.address = datablock;
	free(data);
	return new;
}

/* iblock is the address of the indirect blocks */
struct datablock get_iblock(int iblock, int block_num) {
	struct datablock i_block = get_dblock(iblock);
	int datablock = *((int *) (i_block.data + block_num * INT_SIZE));
	return get_dblock(datablock);
}

struct datablock get_i2block(int i2block, int *block_num) {
	int pointer_num = curr_disk_img->sb.size / INT_SIZE;

	int count = 0;
	while((*block_num) >= pointer_num) {
		(*block_num) -= pointer_num;
		count++;
	}
	struct datablock i2_block = get_dblock(i2block);
	int iblock = *((int *) (i2_block.data + count * INT_SIZE));
	return get_iblock(i2block, *block_num);
}

struct datablock get_i3block(int i3block, int *block_num) {
	int pointer_num = curr_disk_img->sb.size / INT_SIZE;

	int count = 0;
	while((*block_num) >= pointer_num * pointer_num) {
		(*block_num) -= pointer_num * pointer_num;
		count++;
	}
	struct datablock i3_block = get_dblock(i3block);
	int i2block = *((int *) (i3_block.data + count * INT_SIZE));
	return get_i2block(i2block, block_num);
}