#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include "fs.h"

#define ROOT '/'
#define DELIM "/"
#define MAX_OPEN_FILE_NUM 1000
#define INIT_SIZE 2
#define POINTER_NUM dimage->sb.size / INT_SIZE

#define DEFAULTSIZE 1000000
#define BLOCKSIZE 512
#define SBSIZE 512
#define OFFSET (SBSIZE * 2 + BLOCKSIZE)
#define IOFFSET 0
#define ENDLIST -1

struct open_file open_files[MAX_OPEN_FILE_NUM]; // list of open files
int fd_count = 0;
int root_inode = 0;
/* extern var */

int curr_fd = 0;
int uid = 0;

void free_diskimage(struct diskimage *di) {
	free(di->inodes);
	int numBlocks = di->sb.swap_offset - di->sb.data_offset;
	for (int i = 0; i < numBlocks; i++) {
		free(di->blocks[i]);
	}
	free(di->blocks);
	free(di->root);
}



// need to set errno

/* library functions */
int f_open(const char *filename, const char *mode) {
	struct filent file_in_dir;
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
	if (uid != dimage->inodes[open_files[return_file].inode].uid) {
		// user has no permission
		free(path);
		return -1;
	}
	if(open_files[return_file].mode & O_APPEND) {
		open_files[return_file].size = dimage->inodes[open_files[return_file].inode].size;
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
	// struct inode curr = dimage->inodes[curDir];
	// FILE *ptr = disk;
	// int file_size = dimage->sb.size;
	// void *out_buffer = malloc(file_size);
	// void *in_buffer = malloc(file_size);

	// bzero(in_buffer, file_size);
	// bzero(out_buffer, file_size);

	// read(ptr, out_buffer, file_size);
	
	return size;
}

size_t f_write(const void *ptr, size_t size, size_t nmemb, int fd) {
	if (check_valid_fd(fd) == -1) return -1;
	// user has no access
	if (dimage->inodes[open_files[fd].inode].permission & PERMISSION_W) return -1;
	// file can't be read
	if (!(open_files[fd].mode & O_WRONLY) && !(open_files[fd].mode & O_RDWR)) return -1;

	size_t write_size = size * nmemb;
	open_files[fd].size += write_size;

	struct inode *curr = &((dimage->inodes)[open_files[fd].inode]);
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
		new_offset = ((dimage->inodes)[open_files[fd].inode]).size + offset;
	}

	open_files[fd].size = new_offset;
	return 0;
}

void f_rewind(int fd) {
	if (check_valid_fd(fd) != -1) {
		if(open_files[fd].inode == 0)
			open_files[fd].size = INIT_SIZE-1;
		else if(dimage->inodes[open_files[fd].inode].type == IS_FILE)
			open_files[fd].size = 0;
		else if(dimage->inodes[open_files[fd].inode].type == IS_DIRECTORY)
			open_files[fd].size = INIT_SIZE;
	}
}

int f_stat(int fd, struct stat *buf) {
	if (check_valid_fd(fd) == -1) return -1;

	buf->st_dev = (dev_t) dimage->id;
	buf->st_ino = (ino_t) open_files[fd].inode;
	buf->st_mode = (mode_t) open_files[fd].mode;
	buf->st_nlink = (nlink_t) dimage->inodes[open_files[fd].inode].nlink;
	buf->st_uid = (uid_t) dimage->inodes[open_files[fd].inode].uid;
	buf->st_gid = (gid_t) dimage->inodes[open_files[fd].inode].gid;
	// what is st_rdev?
	buf->st_size = (off_t) dimage->inodes[open_files[fd].inode].size;
	buf->st_blksize = (blksize_t) dimage->sb.size;
	buf->st_blocks = (blkcnt_t) buf->st_size / buf->st_blksize;
	// st_atime, st_mtime, st_ctime?

	return 0;
}

int f_remove(const char *filename) {
	rel_or_abs_path(filename);

	struct filent file_in_dir;

	char *path = malloc(strlen(filename)+1);
	char *path_parts = strtok(path, DELIM);

	while (path_parts) {
		file_in_dir = find_file_in_dir(fd_count, path_parts);

		if (file_in_dir.inode < 0) {
			// file not found
			free(path);
			return -1;
		}
		if(dimage->inodes[file_in_dir.inode].type == IS_DIRECTORY) {
			free(path);
			return -1;
		}
		if(uid > 0 && uid != dimage->inodes[file_in_dir.inode].uid) {
			free(path);
			return -1;
		}
		if (strend(filename, path_parts)) {
			int curr_dir_inode = open_files[fd_count].inode;

			// remove file from parent
			if(dimage->inodes[open_files[fd_count].inode].type != IS_DIRECTORY) {
				// parent should be a dir
				return -1;
			}
			if(f_seek(fd_count, dimage->inodes[open_files[fd_count].inode].size-1, SEEK_SET) < 0) {
				// find parent
				return -1;
			}
			struct filent parent_dir = f_readdir(fd_count);
			f_rewind(fd_count);

			struct filent child;
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
			struct inode *parent_inode = &(dimage->inodes[file_in_dir.inode]);
			parent_inode->size -= 1;

			// update free block

			int entry_num = dimage->sb.size / entry_size;
			if (parent_inode->size % entry_num == 0) {
				struct datablock tmp_block = get_data(curr_dir_inode, parent_inode->size / entry_num);
				create_free_blocks(tmp_block.address);
			}

			// update superblock free_inode pointer
			dimage->sb.free_inode = file_in_dir.inode;
			update_superblock();

			// update file inode
			struct inode *file_inode = &(dimage->inodes[file_in_dir.inode]);
			file_inode->nlink = 0;
			file_inode->size = 0;
			file_inode->next_free = dimage->sb.free_inode;
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
	struct filent file_in_dir;

	while (path_parts) {
		file_in_dir = find_file_in_dir(fd_count, path_parts);

		if (file_in_dir.inode < 0) {
			free(path);
			return -1;
		}
		if (strend(dirname, path_parts)) {
			if (dimage->inodes[file_in_dir.inode].type != IS_DIRECTORY) {
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

struct filent f_readdir(int fd) {
	if (check_valid_fd < 0) {
		struct filent *err = ((struct filent *) NULL);
		return *err;
	}
	if(!(dimage->inodes[open_files[fd].inode].permission & PERMISSION_R)) {
		struct filent *err = ((struct filent *) NULL);
		return *err;
	}

	struct filent return_struct;

	int entry_size = NAME_LENGTH + INT_SIZE;
	int entry_num = dimage->sb.size / entry_size;
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
	if (dimage->inodes[open_files[fd].inode].type != IS_DIRECTORY) return -1;

	open_files[fd].inode = -1;
	open_files[fd].size = 0;
	open_files[fd].mode = -1;
	return 0;
}

int f_moveDir(const char *dirname) {

	char *path = (char *) malloc(NAME_LENGTH + 1);
	bzero(path, NAME_LENGTH + 1);
	strcpy(path, dirname);

	char* delim_path[100];

	char *path_parts = strtok(path, DELIM);
	int index = 0;
	while (path_parts != NULL) {
		delim_path[index] = malloc(strlen(path_parts));
		delim_path[index] = path_parts;
		path_parts = strtok(NULL, DELIM);
		index++;
	}

	if (index == 0) {
		return 0;
	}

	int tempDir = curDir;
	int pos = 0;	
	while (pos < index) {
		struct inode idir = dimage->inodes[tempDir];
		if ((tempDir = findDir(delim_path[pos], idir)) == -1) {
			printf("error: path to new directory does not exist\n");
			return 1;
		}	
		pos++;
	}	
	return tempDir;
}

int findDir(const char *dirname, const struct inode i) {
	int loc = 0;
	while (loc < i.size) {
		int dnode = loc / dimage->sb.size;
		int offset = loc % dimage->sb.size;
		char * fileblock = malloc(dimage->sb.size);
		int dloc = i.dblocks[dnode];
		if (dloc == -1) {
			strcpy(fileblock, dimage->root);
		}
		else {
			strcpy(fileblock, dimage->blocks[dloc]);	
		}

		strtok(fileblock, "\t");
		while (fileblock != NULL) {
			strtok(NULL, "\t");
			char * id = strtok(NULL, "\t");
			char * name = strtok(NULL, "\n");
			if (name != NULL && strcmp(name, dirname) == 0) {
				int numID;
				sscanf(id, "%d", &numID);
				free(fileblock);
				return numID;
			}
			if (fileblock != NULL) {
				strtok(NULL, "\t");	
			}	
		}
		loc += dimage->sb.size;
		free(fileblock);
	}
	return -1;
}

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

char *find_pwd_dir() {
	// printf("curDir: %d\n", curDir);
	struct inode curr = dimage->inodes[curDir];
	char *data = malloc(curr.size);
	data = dimage->blocks[curDir-2];
	char** tokens;

    // printf("data=[%s]\n\n", data);

	tokens = str_split(data, '\n');
	char* old;

    if (tokens)
    {
        int i;
        for (i = 0; *(tokens + i); i++)
        {
			old = malloc(sizeof(*(tokens + i)));
			old = *(tokens + i);
			// printf("month=[%s]\n", old);
			if (i == (curDir - 1)) break;
            free(*(tokens + i));
        }
        // printf("\n");
		// printf("%s", old);
        free(tokens);
    }

	data = old;
    tokens = str_split(data, '\t');
	char *path = malloc(500);
	path = strcat(path, "/");
	char *result;

    if (tokens)
    {
        int i;
        for (i = 0; *(tokens + i); i++)
        {
			result = malloc(100);
			result = strcat(result, *(tokens + i));
			result = strcat(result, "/");
			// printf("path=[%s]\n", path);
			if (i == 3) break;
            free(*(tokens + i));
        }
        printf("\n");
		path = strcat(path, result);
		// printf("path: %s\n", path);
		printf("%s", result);
        free(tokens);
    }

}

int f_mkdir(const char *dirname, mode_t mode) {
	if (!(mode & PERMISSION_R) | !(mode & PERMISSION_W) | !(mode & PERMISSION_X)) {
		// set to default permission
		mode = DEFAULT_DIR_PERMISSION;
	}


	char *path = (char *) malloc(NAME_LENGTH + 1);
	bzero(path, NAME_LENGTH + 1);
	strcpy(path, dirname);

	char* delim_path[100];

	char *path_parts = strtok(path, DELIM);
	int index = 0;
	while (path_parts != NULL) {
		delim_path[index] = malloc(strlen(path_parts));
		delim_path[index] = path_parts;
		path_parts = strtok(NULL, DELIM);
		index++;
	}
	index--;
	int tempDir = curDir;
	int pos = 0;	
	while (pos < index) {
		struct inode idir = dimage->inodes[tempDir];
		if ((tempDir = findDir(delim_path[pos], idir)) == -1) {
			printf("error: path to new directory does not exist\n");
			return 1;
		}	
		pos++;
	}

	struct filent * newDir = malloc (sizeof(struct filent));
	newDir->file_name = delim_path[index];
	newDir->inode = dimage->sb.free_inode;
	newDir->user = "guest";
	newDir->perms = "----------";

	struct inode* iparent = &dimage->inodes[tempDir];
	iparent->nlink++;
	char *dirEntry = (char *) malloc (5 * NAME_LENGTH);
	int dirLen = formatDir(newDir, dirEntry);
	iparent->size += dirLen;
	if (tempDir != 0) {
		strcat(dimage->blocks[iparent->dblocks[iparent->size / dimage->sb.size]], dirEntry);	
	}
	else {
		strcat(dimage->root, dirEntry);
	}

	struct inode* new_inode = &dimage->inodes[dimage->sb.free_inode];
	new_inode->nlink = 0;
	new_inode->next_free = 0;
	new_inode->type = 1;
	new_inode->size = 0;
	clock_t time = clock();
	new_inode->ctime = time;	
	new_inode->mtime = time;	
	new_inode->atime = time;	
	new_inode->dblocks[0] = dimage->sb.free_block++;

	free(dirEntry);	
	dirEntry = (char *) malloc (5 * NAME_LENGTH);
	dirLen = dotdotDir(newDir, dirEntry);
	new_inode->size += dirLen;
	strcat(dimage->blocks[new_inode->dblocks[0]], dirEntry);
	free(dirEntry);	
	dirEntry = (char *) malloc (5 * NAME_LENGTH);
	dirLen = dotDir(newDir, dirEntry);
	new_inode->size += dirLen;
	strcat(dimage->blocks[new_inode->dblocks[0]], dirEntry);
	free(dirEntry);
	free(newDir);
	for (int i = 0; i < index; i++){
		free(delim_path[i]);
	}

	return 0;	
}

int f_rmdir(const char *dirname) {
	int dir = f_opendir(dirname);
	if (check_valid_fd(dir) == -1) return -1;

	if(uid != dimage->inodes[open_files[dir].inode].uid) return -1;

	int result = remove_dir(dir);
	if (result == -1) return -1;

	if (f_remove(dirname) == -1) return -1;

	dimage->inodes[open_files[dir].inode].type = IS_FILE;
	int entry_size = NAME_LENGTH + INT_SIZE;
	dimage->inodes[open_files[dir].inode].size *= entry_size;
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

struct filent find_file_in_dir(int dir, char *filename) {
	struct filent file_in_dir;
	file_in_dir.inode = -1;
	bzero(file_in_dir.file_name, NAME_LENGTH);

	printf("PASS\n");

	int old_size = open_files[dir].size;

	printf("CURR\n");
	int size = dimage->inodes[open_files[dir].inode].size;
	printf("CURR END\n");

	if (dimage->inodes[open_files[dir].inode].type != IS_DIRECTORY) {
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
	new_inode = dimage->sb.free_inode;

	// update superblock
	dimage->sb.free_inode = dimage->inodes[dimage->sb.free_inode].next_free;
	update_superblock();

	// update inode
	dimage->inodes[new_inode].type = type;
	dimage->inodes[new_inode].protect = 0;
	dimage->inodes[new_inode].nlink = 1;
	dimage->inodes[new_inode].size = 0;
	dimage->inodes[new_inode].uid = uid;
	dimage->inodes[new_inode].gid = uid;

	for (int i = 0; i < N_DBLOCKS; i++) {
		dimage->inodes[new_inode].dblocks[i] = 0;
	}
	for(int i = 0; i < N_IBLOCKS; i++) {
		dimage->inodes[new_inode].iblocks[i] = 0;
	}

	dimage->inodes[new_inode].i2block = 0;
	dimage->inodes[new_inode].i3block = 0;

	dimage->inodes[new_inode].parent = open_files[dir].inode;
	dimage->inodes[new_inode].next_free = 0;
	dimage->inodes[new_inode].permission = permission;

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
	lseek(dimage->fd, SUPER_OFFSET, SEEK_SET);
	f_write(&(dimage->sb), sizeof(struct superblock), 1, dimage->fd);
}

/* update a single inode */
void update_inode(int inode) {
	lseek(dimage->fd, INODE_OFFSET + dimage->sb.inode_offset * dimage->sb.size + inode * sizeof(struct inode), SEEK_SET);
	f_write(&(dimage->inodes[inode]), sizeof(struct inode), 1, dimage->fd);
}

/* get data from datablock */
struct datablock get_data(int inode, int block_num) {
	struct inode *curr = &(dimage->inodes[inode]);

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
	lseek(dimage->fd, INODE_OFFSET + (dimage->sb.data_offset + datablock)*dimage->sb.size, SEEK_SET);

	void *data = malloc(dimage->sb.size);
	f_read(data, dimage->sb.size, 1, dimage->fd);
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
	int *tmp_free_block = (int*) malloc(dimage->sb.size);

	int free_head = dimage->sb.free_block;
	// read curr free
	lseek(dimage->fd, INODE_OFFSET + (dimage->sb.free_block + free_head) * (dimage->sb).size, SEEK_SET);
	bzero(tmp_free_block, dimage->sb.size);
	f_read(tmp_free_block, dimage->sb.size, 1, dimage->fd);
	int index = tmp_free_block[0] + 1;
	tmp_free_block[index] = block_num;
	tmp_free_block[0] += 1;

	// write free block back
	lseek(dimage->fd, INODE_OFFSET + (dimage->sb.free_block + free_head) * (dimage->sb).size, SEEK_SET);
	f_write((char*) tmp_free_block, dimage->sb.size, 1, dimage->fd);
	free(tmp_free_block);
}

void clean_dblock(int datablock, int *total_size) {
	(*total_size) -= dimage->sb.size;
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

	struct inode *curr = &(dimage->inodes[open_files[fd].inode]);
	if (curr->size < open_files[fd].size) {
		return -1;
	}

	int block_size = (dimage->sb).size;

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
	if(dimage->inodes[open_files[fd_count].inode].type != IS_DIRECTORY) return -1;

	struct filent file_in_dir;
	file_in_dir.inode = 0;

	int file_size = dimage->inodes[open_files[dir].inode].size;
	if(open_files[dir].size < file_size) {
		file_in_dir = f_readdir(dir);
	}

	while(open_files[dir].size < file_size) {
		if(file_in_dir.inode != open_files[dir].inode && file_in_dir.inode != dimage->inodes[open_files[dir].inode].parent) {
			if (dimage->inodes[file_in_dir.inode].type == IS_DIRECTORY) {
				if(increase_fd_count() < 0) return -1;

				if(file_in_dir.inode == 0) {
					open_files[fd_count].size = INIT_SIZE - 1;
				} else{
					open_files[fd_count].size = INIT_SIZE;
				}

				open_files[fd_count].inode = file_in_dir.inode;
				open_files[fd_count].mode = O_RDONLY;

				remove_dir(fd_count);
			} else if (dimage->inodes[file_in_dir.inode].type == IS_FILE) {
				int file_in_dir_inode = file_in_dir.inode;
				// start removing file
				int index = open_files[dir].inode;
				struct inode *dir_inode = &(dimage->inodes[index]);
				// if(find_and_remove_entry(dir, file_in_dir_inode) < 0) {
				// 	return -1;
				// }
				dir_inode->size--;
				update_inode(index);

				int entry_size = NAME_LENGTH + INT_SIZE;
				int N_FILE_ENTRY = dimage->sb.size / entry_size;
				int last_block_ind = dir_inode->size/N_FILE_ENTRY;
				int last_block_rmd = dir_inode->size % N_FILE_ENTRY;
				if(last_block_rmd == 0) {
					struct datablock tmp_block = get_data(index, last_block_ind);
					create_free_blocks(tmp_block.address);
					free(tmp_block.data);
				}

				// file inode
				struct inode *file_inode = &(dimage->inodes[file_in_dir_inode]);
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

				file_inode->next_free = dimage->sb.free_inode;
				update_inode(file_in_dir_inode);

				// sb
				dimage->sb.free_inode = file_in_dir_inode;
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
	struct inode *curr = &(dimage->inodes[inode]);

	size_t file_size = curr->size;
	size_t file_block = file_size / dimage->sb.size;
	if (file_size % dimage->sb.size != 0) {
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
	int free_head = dimage->sb.free_block;
	lseek(dimage->fd, INODE_OFFSET + (dimage->sb.free_block + free_head) * (dimage->sb).size, SEEK_SET);

	int *tmp_free_block = (int*) malloc(dimage->sb.size);
	bzero(tmp_free_block, dimage->sb.size);
	f_read(tmp_free_block, dimage->sb.size, 1, dimage->fd);

	int block_num = tmp_free_block[tmp_free_block[0]];

	tmp_free_block[0] -= 1;
	if(tmp_free_block[0] == 0) {
		(dimage->sb).free_block = free_head + 1;
	}

	update_superblock();

	// write free block
	lseek(dimage->fd, INODE_OFFSET + (dimage->sb.free_block + free_head) * (dimage->sb).size, SEEK_SET);
	f_write((char*) tmp_free_block, dimage->sb.size, 1, dimage->fd);
	free(tmp_free_block);

	return block_num;
}

void write_dblock(int dblock, void *data) {
	lseek(dimage->fd, INODE_OFFSET + (dimage->sb.data_offset + dblock) * dimage->sb.size, SEEK_SET);
	f_write(data, dimage->sb.size, 1, dimage->fd);
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
	lseek(dimage->fd, INODE_OFFSET + (dimage->sb.data_offset + db->address) * dimage->sb.size, SEEK_SET);
	write(dimage->fd, db->data, dimage->sb.size);
}

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

