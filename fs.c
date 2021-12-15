#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "fs.h"

#define ROOT '/'
#define DELIM "/"
#define MAX_OPEN_FILE_NUM 1000
#define INIT_SIZE 2
#define POINTER_NUM curr_disk_img->sb.size / INT_SIZE

struct open_file open_files[MAX_OPEN_FILE_NUM]; // list of open files
struct disk_img *curr_disk_img = 0;
int fd_count = 0;
int root_inode = 0;
/* extern var */
extern int curr_fd = 0;
extern int uid = 0;

// need to set errno

/* library functions */
int f_open(const char *filename, const char *mode) {
	struct fileent file_in_dir;
	int return_file;

    char *path = malloc(strlen(filename)+1);
	strcpy(path, filename);
    char *path_parts = strtok(path, DELIM);

	rel_or_abs_path(filename);

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
		printf("PASS\n");
		if (file_in_dir.inode < 0) {
			// file not found
			if (strend(filename, path_parts) && (new_mode & O_CREAT)) {
				// create new file
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
	return_file = fd_count;
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
	int fd_count = increase_fd_count();
	if (fd_count == -1) {
		free(path);
		return -1;
	}
	free(path);
	return return_file;
}

size_t f_read(void *ptr, size_t size, size_t nmemb, int fd) {
	if (check_valid_fd(fd) == -1) return -1;
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

size_t f_write(const void *ptr, size_t size, size_t nmemb, int fd) {
	if (check_valid_fd(fd) == -1) return -1;
	// user has no access
	if (curr_disk_img->inodes[open_files[fd].inode].permission & PERMISSION_W) return -1;
	// file can't be read
	if (!(open_files[fd].mode & O_WRONLY) && !(open_files[fd].mode & O_RDWR)) return -1;

	size_t write_size = size * nmemb;
	open_files[fd].size += write_size;

	struct inode *curr = &((curr_disk_img->inodes)[open_files[fd].inode]);
	if (open_files[fd].size + write_size > curr->size) {
		curr->size = open_files[fd].size + write_size;
		update_inode(open_files[fd].inode);
	}
	return write_size;
}

int f_close(int fd) {
	if (check_valid_fd(fd) == -1) return -1;

	open_files[fd].inode = -1;
	open_files[fd].size = 0;
	open_files[fd].mode = -1;
	return 0;
}

int f_seek(int fd, long int offset, int whence) {
	if (check_valid_fd(fd) == -1) return -1;

	long new_offset = offset;
	if (whence == SEEK_CUR) {
		new_offset = open_files[fd].size + offset;
	} else if (whence == SEEK_END) {
		new_offset = ((curr_disk_img->inodes)[open_files[fd].inode]).size + offset;
	}

	open_files[fd].size = new_offset;
	return 0;
}

void f_rewind(int fd) {
	if (check_valid_fd(fd) != -1) {
		if(open_files[fd].inode == 0)
			open_files[fd].size = INIT_SIZE-1;
		else if(curr_disk_img->inodes[open_files[fd].inode].type == IS_FILE)
			open_files[fd].size = 0;
		else if(curr_disk_img->inodes[open_files[fd].inode].type == IS_DIRECTORY)
			open_files[fd].size = INIT_SIZE;
	}
}

int f_stat(int fd, struct stat *buf) {
	if (check_valid_fd(fd) == -1) return -1;

	buf->st_dev = (dev_t) curr_disk_img->id;
	buf->st_ino = (ino_t) open_files[fd].inode;
	buf->st_mode = (mode_t) open_files[fd].mode;
	buf->st_nlink = (nlink_t) curr_disk_img->inodes[open_files[fd].inode].nlink;
	buf->st_uid = (uid_t) curr_disk_img->inodes[open_files[fd].inode].uid;
	buf->st_gid = (gid_t) curr_disk_img->inodes[open_files[fd].inode].gid;
	// what is st_rdev?
	buf->st_size = (off_t) curr_disk_img->inodes[open_files[fd].inode].size;
	buf->st_blksize = (blksize_t) curr_disk_img->sb.size;
	buf->st_blocks = (blkcnt_t) buf->st_size / buf->st_blksize;
	// st_atime, st_mtime, st_ctime?

	return 0;
}

int f_remove(const char *filename) {
	rel_or_abs_path(filename);

	struct fileent file_in_dir;

    char *path = malloc(strlen(filename)+1);
    char *path_parts = strtok(path, DELIM);

	while (path_parts) {
		file_in_dir = find_file_in_dir(fd_count, path_parts);

		if (file_in_dir.inode < 0) {
			// file not found
			free(path);
			return -1;
		}
		if(curr_disk_img->inodes[file_in_dir.inode].type == IS_DIRECTORY) {
			free(path);
			return -1;
		}
		if(uid > 0 && uid != curr_disk_img->inodes[file_in_dir.inode].uid) {
			free(path);
			return -1;
		}
		if (strend(filename, path_parts)) {
			int curr_dir_inode = open_files[fd_count].inode;

			// remove file from parent
			if(curr_disk_img->inodes[open_files[fd_count].inode].type != IS_DIRECTORY) {
				// parent should be a dir
				return -1;
			}
			if(f_seek(fd_count, curr_disk_img->inodes[open_files[fd_count].inode].size-1, SEEK_SET) < 0) {
				// find parent
				return -1;
			}
			struct fileent parent_dir = f_readdir(fd_count);
			f_rewind(fd_count);

			struct fileent child;
			child.inode = -1;
			child = f_readdir(fd_count);
			while (child.inode >=0 && child.inode != file_in_dir.inode) child = f_readdir(fd_count);
			if (child.inode < 0) {
				// file not in dir
				return -1;
			}
			open_files[fd_count].size -= 1;
			int entry_size = NAME_LENGTH + INT_SIZE;

			if(write_file(&(parent_dir.inode), sizeof(int), 1, fd_count, open_files[fd_count].size * entry_size) != INT_SIZE) {
				return -1;
			}
			if(write_file(parent_dir.file_name, NAME_LENGTH, 1, fd_count, open_files[fd_count].size * entry_size + INT_SIZE) != NAME_LENGTH) {
				return -1;
			}

			update_inode(curr_dir_inode);
			struct inode *parent_inode = &(curr_disk_img->inodes[file_in_dir.inode]);
			parent_inode->size -= 1;

			// update free block
			
			int entry_num = curr_disk_img->sb.size / entry_size;
			if (parent_inode->size % entry_num == 0) {
				struct datablock tmp_block = get_data(curr_dir_inode, parent_inode->size / entry_num);
				create_free_blocks(tmp_block.address);
			}

			// update superblock free_inode pointer
			curr_disk_img->sb.free_inode = file_in_dir.inode;
			update_superblock();

			// update file inode
			struct inode *file_inode = &(curr_disk_img->inodes[file_in_dir.inode]);
			file_inode->nlink = 0;
			file_inode->size = 0;
			file_inode->next_free = curr_disk_img->sb.free_inode;
			update_inode(file_in_dir.inode);

			// clean file inode block
			int total_size = file_inode->size;

			for (int i = 0; i < N_DBLOCKS; i++) {
				if (total_size <= 0) break;
				clean_dblock(file_inode->dblocks[i], &total_size);
			}

			for (int i = 0; i < N_IBLOCKS; i++) {
				if (total_size <= 0) break;
				clean_iblock(file_inode->iblocks[i], &total_size);
			}

			if (total_size > 0) {
				clean_i2block(file_inode->i2block, &total_size);
				if (total_size > 0) {
					clean_i3block(file_inode->i3block, &total_size);
				}
			}
		}

		if(file_in_dir.inode == 0) {
			open_files[fd_count].size = INIT_SIZE-1;
		}
		else {
			open_files[fd_count].size = INIT_SIZE;
		}
		open_files[fd_count].inode = file_in_dir.inode;
		open_files[fd_count].mode = O_RDONLY;

		path_parts = strtok(NULL, DELIM);
	}
	return 0;
}

int f_opendir(const char *dirname) {
	int target_dir;
	char *path = malloc(strlen(dirname) + 1);
	strcpy(path, dirname);
	if((*path) == ROOT) {
		open_files[fd_count].inode = root_inode;
		open_files[fd_count].size = 0;
		open_files[fd_count].mode = O_RDONLY;
	} else {
		if(open_files[fd_count].inode == 0) {
			open_files[fd_count].size = 0;
		}
		open_files[fd_count].inode = open_files[curr_fd].inode;
		open_files[fd_count].mode = O_RDONLY;
	}

	char *path_parts = strtok(path, DELIM);
	struct fileent file_in_dir;

	while (path_parts) {
		file_in_dir = find_file_in_dir(fd_count, path_parts);

		if (file_in_dir.inode < 0) {
			free(path);
			return -1;
		}
		if (strend(dirname, path_parts)) {
			if (curr_disk_img->inodes[file_in_dir.inode].type != IS_DIRECTORY) {
				free(path);
				return -1;
			}
		}

		if(file_in_dir.inode == 0) {
			open_files[fd_count].size = INIT_SIZE-1;
		}
		else
			open_files[fd_count].size = INIT_SIZE;
		open_files[fd_count].inode = file_in_dir.inode;
		open_files[fd_count].mode = O_RDONLY;
		path_parts = strtok(NULL, DELIM);
	}

	target_dir = fd_count;
	int result = increase_fd_count();
	if (result == -1) {
		free(path);
		return -1;
	}
	free(path);
	return target_dir;
}

struct fileent f_readdir(int fd) {
	if (check_valid_fd < 0) {
		struct fileent *err = ((struct fileent *) NULL);
		return *err;
	}
	if(!(curr_disk_img->inodes[open_files[fd].inode].permission & PERMISSION_R)) {
		struct fileent *err = ((struct fileent *) NULL);
		return *err;
	}

	struct fileent return_struct;
	
	int entry_size = NAME_LENGTH + INT_SIZE;
	int entry_num = curr_disk_img->sb.size / entry_size;
	int block_num = open_files[fd].size / entry_num;

	struct datablock dir_data = get_data(open_files[fd].inode, block_num);
	int rest = open_files[fd].size % entry_num;

	return_struct.inode = *((int *) (dir_data.data + rest * entry_size));
	memcpy(return_struct.file_name, dir_data.data + entry_size * rest + INT_SIZE, NAME_LENGTH);

	open_files[fd].size += 1;
	free(dir_data.data);
	
	return return_struct;
}

int f_closedir(int fd) {
	if (check_valid_fd(fd) < 0) return -1;
	if (curr_disk_img->inodes[open_files[fd].inode].type != IS_DIRECTORY) return -1;

	open_files[fd].inode = -1;
	open_files[fd].size = 0;
	open_files[fd].mode = -1;
	return 0;
}

int f_mkdir(const char *dirname, mode_t mode) {
	if (!(mode & PERMISSION_R) | !(mode & PERMISSION_W) | !(mode & PERMISSION_X)) {
		// set to default permission
		mode = DEFAULT_DIR_PERMISSION;
	}

	char *path = (char *) malloc(NAME_LENGTH + 1);
	bzero(path, NAME_LENGTH + 1);
	strcpy(path, dirname);

	char *path_parts = strtok(path, DELIM);
	rel_or_abs_path(dirname);

	struct fileent file_in_dir;
	while (path_parts) {
		printf("IN FUNCRION\n");
		file_in_dir = find_file_in_dir(fd_count, path_parts);

		if(!(curr_disk_img->inodes[open_files[fd_count].inode].permission & PERMISSION_W)) {
			return -1;
		}
		

		if(strend(dirname, path_parts)) {
			
			
			if (file_in_dir.inode < 0) {
				// dir doesn't exist
				if(create_file(fd_count, IS_DIRECTORY, path_parts, mode) < 0) {
					free(path);
					return -1;
				}
				// path
				if(open_files[fd_count].inode == 0) {
					open_files[fd_count].size = INIT_SIZE - 1;
				} else {
					open_files[fd_count].size = INIT_SIZE;
				}
				file_in_dir = find_file_in_dir(fd_count, path_parts);
				open_files[fd_count].inode = file_in_dir.inode;
				open_files[fd_count].size = 0;
				open_files[fd_count].mode = O_RDONLY;

				char file_name[NAME_LENGTH];
				bzero(file_name, NAME_LENGTH);
				int parent_inode = open_files[fd_count].inode;
				int entry_size = NAME_LENGTH + INT_SIZE;

				file_name[0] = '.';
				if(write_file(&(file_in_dir.inode), INT_SIZE, 1, fd_count, open_files[fd_count].size * entry_size) != INT_SIZE) {
					free(path);
					return -1;
				}
				if(write_file(file_name, NAME_LENGTH, 1, fd_count, open_files[fd_count].size*entry_size + INT_SIZE) != NAME_LENGTH) {
					free(path);
					return -1;
				}
				open_files[fd_count].size += 1;
				curr_disk_img->inodes[file_in_dir.inode].size += 1;

				file_name[1] = '.';
				if(write_file(&parent_inode, INT_SIZE, 1, fd_count, open_files[fd_count].size * entry_size) != INT_SIZE) {
					free(path);
					return -1;
				}
				if(write_file(file_name, NAME_LENGTH, 1, fd_count, open_files[fd_count].size*entry_size + INT_SIZE) != NAME_LENGTH) {
					free(path);
					return -1;
				}
				update_inode(file_in_dir.inode);

				open_files[fd_count].size += 1;
				curr_disk_img->inodes[file_in_dir.inode].size += 1;

				free(path);
				return 0;
			}
			free(path);
			return -1;
		} else {
			if (file_in_dir.inode < 0) {
				free(path);
				return -1;
			}
		}

		if(open_files[fd_count].inode == 0) {
			open_files[fd_count].size = INIT_SIZE - 1;
		} else {
			open_files[fd_count].size = INIT_SIZE;
		}
		open_files[fd_count].inode = file_in_dir.inode;
		open_files[fd_count].mode = O_RDONLY;
		path_parts = strtok(NULL, DELIM);
	}
	free(path);
	return 0;
}

int f_rmdir(const char *dirname) {
	int dir = f_opendir(dirname);
	if (check_valid_fd(dir) == -1) return -1;

	if(uid != curr_disk_img->inodes[open_files[dir].inode].uid) return -1;

	int result = remove_dir(dir);
	if (result == -1) return -1;

	if (f_remove(dirname) == -1) return -1;

	curr_disk_img->inodes[open_files[dir].inode].type = IS_FILE;
	int entry_size = NAME_LENGTH + INT_SIZE;
	curr_disk_img->inodes[open_files[dir].inode].size *= entry_size;
	update_inode(open_files[dir].inode);

	if (f_close(dir) == -1) return -1;

	return 0;
}


int check_valid_fd(int fd) {
	// invalid fd
	if (open_files[fd].inode < 0) return -1;
	if (fd < 0 || fd >= MAX_OPEN_FILE_NUM) return -1;

	return 0;
}

void rel_or_abs_path(const char *filename) {
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
}

struct fileent find_file_in_dir(int dir, char *filename) {
	struct fileent file_in_dir;
	file_in_dir.inode = -1;
	bzero(file_in_dir.file_name, NAME_LENGTH);

	printf("PASS\n");

	int old_size = open_files[dir].size;
	
	printf("CURR\n");
	int size = curr_disk_img->inodes[open_files[dir].inode].size;
	printf("CURR END\n");

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
int strend(const char *s, const char *t) {
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
	update_superblock();

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
void update_superblock() {
	lseek(curr_disk_img->fd, SUPER_OFFSET, SEEK_SET);
	f_write(&(curr_disk_img->sb), sizeof(struct superblock), 1, curr_disk_img->fd);
}

/* update a single inode */
void update_inode(int inode) {
	lseek(curr_disk_img->fd, INODE_OFFSET + curr_disk_img->sb.inode_offset * curr_disk_img->sb.size + inode * sizeof(struct inode), SEEK_SET);
	f_write(&(curr_disk_img->inodes[inode]), sizeof(struct inode), 1, curr_disk_img->fd);
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
	 
    if (block_num < N_IBLOCKS *  POINTER_NUM) {
        int count = block_num /  POINTER_NUM;
        if (block_num %  POINTER_NUM != 0) {
            count++;
        }
        return get_iblock(curr->iblocks[count], block_num);
    }

    // i2block
    block_num -= N_IBLOCKS *  POINTER_NUM;
    if (block_num <  POINTER_NUM *  POINTER_NUM) {
        return get_i2block(curr->i2block, &block_num);
    }

    // i3block
    block_num -=  POINTER_NUM *  POINTER_NUM;
    if (block_num <  POINTER_NUM *  POINTER_NUM *  POINTER_NUM) {
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
	f_read(data, curr_disk_img->sb.size, 1, curr_disk_img->fd);
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
	int count = 0;
	while((*block_num) >=  POINTER_NUM) {
		(*block_num) -=  POINTER_NUM;
		count++;
	}
	struct datablock i2_block = get_dblock(i2block);
	int iblock = *((int *) (i2_block.data + count * INT_SIZE));
	return get_iblock(iblock, *block_num);
}

struct datablock get_i3block(int i3block, int *block_num) {
	int count = 0;
	while((*block_num) >=  POINTER_NUM *  POINTER_NUM) {
		(*block_num) -=  POINTER_NUM *  POINTER_NUM;
		count++;
	}
	struct datablock i3_block = get_dblock(i3block);
	int i2block = *((int *) (i3_block.data + count * INT_SIZE));
	return get_i2block(i2block, block_num);
}

void create_free_blocks(int block_num) {
	int *tmp_free_block = (int*) malloc(curr_disk_img->sb.size);
	
	int free_head = curr_disk_img->sb.free_block;
	// read curr free
	lseek(curr_disk_img->fd, INODE_OFFSET + (curr_disk_img->sb.free_block + free_head) * (curr_disk_img->sb).size, SEEK_SET);
	bzero(tmp_free_block, curr_disk_img->sb.size);
	f_read(tmp_free_block, curr_disk_img->sb.size, 1, curr_disk_img->fd);
	int index = tmp_free_block[0] + 1;
	tmp_free_block[index] = block_num;
	tmp_free_block[0] += 1;

	// write free block back
	lseek(curr_disk_img->fd, INODE_OFFSET + (curr_disk_img->sb.free_block + free_head) * (curr_disk_img->sb).size, SEEK_SET);
	f_write((char*) tmp_free_block, curr_disk_img->sb.size, 1, curr_disk_img->fd);
	free(tmp_free_block);
}

void clean_dblock(int datablock, int *total_size) {
	(*total_size) -= curr_disk_img->sb.size;
	create_free_blocks(datablock);
}

void clean_iblock(int iblock, int *total_size) {
	struct datablock i_block = get_dblock(iblock);

	for (int i = 0; i < POINTER_NUM; i++) {
		int dblock_address = ((int *)i_block.data)[i];
		clean_dblock(dblock_address, total_size);
	}
	create_free_blocks(iblock);
}

void clean_i2block(int i2block, int *total_size) {
	struct datablock i2_block = get_dblock(i2block);

	for (int i = 0; i < POINTER_NUM; i++) {
		int iblock_address = ((int *)i2_block.data)[i];
		clean_iblock(iblock_address, total_size);
	}
	create_free_blocks(i2block);
}

void clean_i3block(int i3block, int *total_size) {
	struct datablock i3_block = get_dblock(i3block);

	for (int i = 0; i < POINTER_NUM; i++) {
		int i2block_address = ((int *)i3_block.data)[i];
		clean_i2block(i2block_address, total_size);
	}
	create_free_blocks(i3block);
}

int write_file(const void *ptr, size_t size, size_t nmemb, int fd, int file_size) {
	size_t total_size = size * nmemb;

	struct inode *curr = &(curr_disk_img->inodes[open_files[fd].inode]);
	if (curr->size < open_files[fd].size) {
		return -1;
	}

	int block_size = (curr_disk_img->sb).size;

	size_t curr_offset = 0;
	struct datablock tmp_block;
	int first = file_size / block_size;

	if (file_size > 0) {
		size_t first_rest = file_size % block_size;
		if (first_rest != 0){
			size_t extra_size = block_size - first_rest;

			if (total_size < extra_size) extra_size = total_size;
			tmp_block = get_data(open_files[fd].inode, first);
			
			memcpy(tmp_block.data + first_rest, ptr + curr_offset, extra_size);
			write_data(open_files[fd].inode, first, tmp_block.data);
			curr_offset += extra_size;
			total_size -= extra_size;
			first++;
		}
	}

	int num_block = total_size / block_size;
	if (total_size % block_size != 0){
		num_block += 1;
	}

	size_t rest = total_size % block_size;
	for (int i = first; i < first + num_block; i++) {
		// read data
		if (total_size <= 0) break;
		tmp_block = get_data(open_files[fd].inode, i);

		rest = total_size % block_size;
		if (rest == 0){
			rest += block_size;
		}
		memcpy(tmp_block.data, ptr + curr_offset, rest);
		write_data(open_files[fd].inode, i, tmp_block.data);

		curr_offset += rest;
		total_size -= rest;
	}
	
	return nmemb * size;
}

int remove_dir(int dir) {
	if(curr_disk_img->inodes[open_files[fd_count].inode].type != IS_DIRECTORY) return -1;

	struct fileent file_in_dir;
	file_in_dir.inode = 0;

	int file_size = curr_disk_img->inodes[open_files[dir].inode].size;
	if(open_files[dir].size < file_size) {
		file_in_dir = f_readdir(dir);
	}

	while(open_files[dir].size < file_size) {
		if(file_in_dir.inode != open_files[dir].inode && file_in_dir.inode != curr_disk_img->inodes[open_files[dir].inode].parent) {
			if (curr_disk_img->inodes[file_in_dir.inode].type == IS_DIRECTORY) {
				if(increase_fd_count() < 0) return -1;
				
				if(file_in_dir.inode == 0) {
					open_files[fd_count].size = INIT_SIZE - 1;
				} else{
					open_files[fd_count].size = INIT_SIZE;
				}

				open_files[fd_count].inode = file_in_dir.inode;
				open_files[fd_count].mode = O_RDONLY;

				remove_dir(fd_count);
			} else if (curr_disk_img->inodes[file_in_dir.inode].type == IS_FILE) {
				int file_in_dir_inode = file_in_dir.inode;
				// start removing file
				int index = open_files[dir].inode;
				struct inode *dir_inode = &(curr_disk_img->inodes[index]);
				// if(find_and_remove_entry(dir, file_in_dir_inode) < 0) {
				// 	return -1;
				// }
				dir_inode->size--;
				update_inode(index);

				int entry_size = NAME_LENGTH + INT_SIZE;
				int N_FILE_ENTRY = curr_disk_img->sb.size / entry_size;
				int last_block_ind = dir_inode->size/N_FILE_ENTRY;
				int last_block_rmd = dir_inode->size % N_FILE_ENTRY;
				if(last_block_rmd == 0) {
					struct datablock tmp_block = get_data(index, last_block_ind);
					create_free_blocks(tmp_block.address);
					free(tmp_block.data);
				}

				// file inode
				struct inode *file_inode = &(curr_disk_img->inodes[file_in_dir_inode]);
				file_inode->nlink = 0; 
				int rest = file_inode->size;

				// clean blocks
				for (int i = 0; i < N_DBLOCKS; i++){
					if (rest <= 0) break;
					clean_dblock((file_inode->dblocks)[i], &rest);
				}
				// clean iblock
				for (int i = 0; i < N_IBLOCKS; i++){
					if (rest <= 0) break;
					clean_iblock((file_inode->iblocks)[i], &rest);
				}
				// clean i2block
				if (rest > 0){
					clean_i2block(file_inode->i2block, &rest);
				}
				// clean i3block
				if (rest > 0){
					clean_i3block(file_inode->i3block, &rest);
				}
				file_inode->size = 0;

				file_inode->next_free = curr_disk_img->sb.free_inode;
				update_inode(file_in_dir_inode);

				// sb
				curr_disk_img->sb.free_inode = file_in_dir_inode;
				update_superblock();

				// end
				open_files[dir].size -= 1;
			} else {
				return -1;
			}
		}
		file_in_dir = f_readdir(dir);
	}
	
	return 0;
}

int write_data(int inode, int block_num, void *data) {
	struct inode *curr = &(curr_disk_img->inodes[inode]);

	size_t file_size = curr->size;
	size_t file_block = file_size / curr_disk_img->sb.size;
	if (file_size % curr_disk_img->sb.size != 0) {
		file_block += 1;
	}

	int *available = malloc(4 * sizeof(int)); // each index represents dblock, iblock, i1block, i2block respectively
	bzero(available, 4 * sizeof(int));

	// check what points to free blocks
	if(file_block < block_num + 1) {
		int num = block_num + 1;
		
		if (num <= N_DBLOCKS) {
			// dblock
			available[0] = 1;
		} else if (num <= N_DBLOCKS + POINTER_NUM * N_IBLOCKS) {
			// iblock
			int full_iblock_num = 0; // full iblocks
			int datablock_num = num - N_DBLOCKS; // num of data blocks in not full blocks
			while(datablock_num >= POINTER_NUM) {
				datablock_num -= POINTER_NUM; 
				full_iblock_num++;
			}
			available[0] = 1;
			if (file_block <= N_DBLOCKS + full_iblock_num * POINTER_NUM && datablock_num == 1){
				available[1] = 1;
			}
		} else if (num <=  N_DBLOCKS + POINTER_NUM * N_IBLOCKS + POINTER_NUM * POINTER_NUM) {
			// i2block
			int datablock_rem = 0; // reamining datablock
			int full_iblock = num - (N_DBLOCKS + POINTER_NUM * N_IBLOCKS);
			while(full_iblock >= POINTER_NUM) {
				full_iblock -= POINTER_NUM;
				datablock_rem++;
			}

			available[0] = 1;
			if (file_block <=  N_DBLOCKS +  N_IBLOCKS * POINTER_NUM + datablock_rem * POINTER_NUM && full_iblock == 1){
				available[1] = 1;
				if (file_block <=  N_DBLOCKS +  N_IBLOCKS * POINTER_NUM && datablock_rem == 0){
					available[2] = 1;
				}
			}
		} else if (num <=  N_DBLOCKS + POINTER_NUM * N_IBLOCKS + POINTER_NUM * POINTER_NUM + POINTER_NUM * POINTER_NUM * POINTER_NUM) {
			// i3block
			int total_datablock_before_i3 = N_DBLOCKS + POINTER_NUM * N_IBLOCKS + POINTER_NUM * POINTER_NUM;
			int not_full_i1_i2_block = num - total_datablock_before_i3;
			int full_i2_block_num = 0;
			while(not_full_i1_i2_block >= POINTER_NUM * POINTER_NUM) {
				not_full_i1_i2_block -= POINTER_NUM * POINTER_NUM;
				full_i2_block_num++;
			}

			int not_full_i1_block = not_full_i1_i2_block;
			not_full_i1_i2_block = 0;
			while (not_full_i1_block >= POINTER_NUM) {
				not_full_i1_block -= POINTER_NUM;
				not_full_i1_i2_block++;
			}
			
			available[0] = 1;
			if (file_block <= total_datablock_before_i3 + full_i2_block_num * POINTER_NUM * POINTER_NUM + not_full_i1_i2_block * POINTER_NUM && not_full_i1_block == 1) {
				available[1] = 1;
				if (file_block <= total_datablock_before_i3 + full_i2_block_num * POINTER_NUM * POINTER_NUM && not_full_i1_i2_block == 0) {
					available[2] = 1;
					if (file_block <= total_datablock_before_i3 && full_i2_block_num == 0){
						available[3] = 1;
					}
				}
			}
		}
	}

	// dblock
	if(block_num < N_DBLOCKS) {
		if (available[0]){
			curr->dblocks[block_num] = find_free();
			update_inode(inode);
		}
		write_dblock(curr->dblocks[block_num], data);
		free(available);
		return 0;
	}

	block_num -= N_DBLOCKS;

	// iblock
	int i = 0;
	while(block_num >= POINTER_NUM && i < N_IBLOCKS) {
		block_num -= POINTER_NUM;
		i++;
	}
	
	if(i < N_IBLOCKS) {
		if (available[1]){
			curr->iblocks[i] = find_free();
			update_inode(inode);
		}
		write_iblock(curr->iblocks[i], block_num, data, available);
		free(available);
		return 0;
	}

	// i2block
	if(block_num < POINTER_NUM * POINTER_NUM) {
		if (available[2]){
			curr->i2block = find_free();
			update_inode(inode);
		}
		write_i2block(curr->i2block, &block_num, data, available);
		free(available);
		return 0;
	}

	block_num -= POINTER_NUM * POINTER_NUM;

	// i3block
	if(block_num < POINTER_NUM * POINTER_NUM * POINTER_NUM) {
		if (available[3]){
			curr->i3block = find_free();
			update_inode(inode);
		}
		write_i3block(curr->i3block, &block_num, data, available);
		free(available);
		return 0;
	}

	free(available);
	return -1;
}

int find_free() {
	// read free block
	int free_head = curr_disk_img->sb.free_block;
	lseek(curr_disk_img->fd, INODE_OFFSET + (curr_disk_img->sb.free_block + free_head) * (curr_disk_img->sb).size, SEEK_SET);

	int *tmp_free_block = (int*) malloc(curr_disk_img->sb.size);
	bzero(tmp_free_block, curr_disk_img->sb.size);
	f_read(tmp_free_block, curr_disk_img->sb.size, 1, curr_disk_img->fd);

	int block_num = tmp_free_block[tmp_free_block[0]];

	tmp_free_block[0] -= 1;
	if(tmp_free_block[0] == 0) {
		(curr_disk_img->sb).free_block = free_head + 1;
	}

	update_superblock();

	// write free block
	lseek(curr_disk_img->fd, INODE_OFFSET + (curr_disk_img->sb.free_block + free_head) * (curr_disk_img->sb).size, SEEK_SET);
	f_write((char*) tmp_free_block, curr_disk_img->sb.size, 1, curr_disk_img->fd);
	free(tmp_free_block);

	return block_num;
}

void write_dblock(int dblock, void *data) {
	lseek(curr_disk_img->fd, INODE_OFFSET + (curr_disk_img->sb.data_offset + dblock) * curr_disk_img->sb.size, SEEK_SET);
	f_write(data, curr_disk_img->sb.size, 1, curr_disk_img->fd);
}

void write_iblock(int iblock, int block_num, void *data, int available[4]) {
	struct datablock i_block = get_dblock(iblock);

	int dblock;
	if (available[0]){
		dblock = find_free();
		((int *)i_block.data)[block_num] = dblock;
		update_db(&i_block);
	}
	else{
		dblock = ((int *) i_block.data)[block_num];
	}
	write_dblock(dblock, data);
}

void write_i2block(int i2block, int *block_num, void *data, int available[4]) {
	struct datablock i_2block = get_dblock(i2block);
	int iblock;
	int count = 0;
	while((*block_num) >= POINTER_NUM) {
		(*block_num) -= POINTER_NUM;
		count++;
	}

	if(available[1]) {
		iblock = find_free();
		((int *)i_2block.data)[count] = iblock;
		update_db(&i_2block);
	} else{
		iblock = ((int *) i_2block.data)[count];
	}
	write_iblock(iblock, *block_num, data, available);
}

void write_i3block(int i3block, int *block_num, void *data, int available[4]) {
	struct datablock i_3block = get_dblock(i3block);
	int i2block;
	int count = 0;
	while((*block_num) >= POINTER_NUM) {
		(*block_num) -= POINTER_NUM;
		count++;
	}

	if(available[2]) {
		i2block = find_free();
		((int *)i_3block.data)[count] = i2block;
		update_db(&i_3block);
	} else{
		i2block = ((int *) i_3block.data)[count];
	}
	write_iblock(i2block, *block_num, data, available);
}

void update_db(struct datablock *db) {
	lseek(curr_disk_img->fd, INODE_OFFSET + (curr_disk_img->sb.data_offset + db->address) * curr_disk_img->sb.size, SEEK_SET);
	write(curr_disk_img->fd, db->data, curr_disk_img->sb.size);
}