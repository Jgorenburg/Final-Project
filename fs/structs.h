#ifndef STRUCTS_H
#define STRUCTS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdint.h>

#define NAME_LENGTH 100

#define N_DBLOCKS 10
#define N_IBLOCKS 4

#define INT_SIZE sizeof(int)
#define BOOT_OFFSET 0
#define SUPER_OFFSET 512
#define INODE_OFFSET 1024

#define IS_DIRECTORY 'd'
#define IS_FILE 'f'
#define IS_OTHER_FILE 'm'

#define PERMISSION_R 36
#define	PERMISSION_W 72
#define PERMISSION_X 98
#define DEFAULT_FILE_PERMISSION (PERMISSION_R | PERMISSION_W)
#define DEFAULT_DIR_PERMISSION (PERMISSION_R | PERMISSION_W | PERMISSION_X)

// // file information struct
// struct stat {
//     dev_t     st_dev;     /* ID of device containing file */
//     ino_t     st_ino;     /* inode number */
//     mode_t    st_mode;    /* protection */
//     nlink_t   st_nlink;   /* number of hard links */
//     uid_t     st_uid;     /* user ID of owner */
//     gid_t     st_gid;     /* group ID of owner */
//     dev_t     st_rdev;    /* device ID (if special file) */
//     off_t     st_size;    /* total size, in bytes */
//     blksize_t st_blksize; /* blocksize for file system I/O */
//     blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
//     time_t    st_atime;   /* time of last access */
//     time_t    st_mtime;   /* time of last modification */
//     time_t    st_ctime;   /* time of last status change */
// };

struct file {
	int inode; // the i-node number for the file
	int size; // size of the entry (including padding)
	char type; // 'f' for file, 'd' for directory, 'm' for more
	// char[NAME_LENGTH + 1]; // name of file (with an /0)
};

struct open_file {
	int inode; // the i-node number for the file
	int size; // number of file entries for directory files, number of bytes for regular files
	// char type; // 'f' for file, 'd' for directory, 'm' for more
	int mode; // r, w, a, etc.
};

struct superblock {
	int size; // the size of blocks in bytes
	int inode_offset; // the offset of the inode region
	int data_offset; // the offset of the data region
    int swap_offset;
	int free_inode; // the pointer to the head of the list of free inodes
    int free_block; // the pointer to the head of the list of free disk blocks
};

struct inode {
	int type; /* 1 for directory and 0 for regular files */
	int protect; /* protection field */
	int nlink; /* number of links to this file */
	int size; /* number of bytes in file */
	int uid; /* owner’s user ID */
	int gid; /* owner’s group ID */
	int ctime; /* change time */
	int mtime; /* modification time */
	int atime; /* access time */
	int dblocks[N_DBLOCKS]; /* pointers to data blocks */
	int iblocks[N_IBLOCKS]; /* pointers to indirect blocks */
	int i2block; /* pointer to doubly indirect block */
	int i3block; /* pointer to triply indirect block */
	int parent; /* inode of parent dir if type is dir, curr dir for regular files */
	int next_free; /* next free inode */
	int permission;
};

struct datablock {
	void *data;
	int address;
};

// directory entry struct
struct dirent {
	char *perms; // the permisions for this file	
	char *user; 
	int inode;
	int modTime; // when this file was last modified
	char *file_name; // pointer to the list of file names in the dir
};

// file entry struct 
struct fileent {
	int inode;
	char file_name[NAME_LENGTH];
};

// disk image file struct
struct disk_img {
	int id;
	struct superblock sb;
	struct inode *inodes;
	struct disk_img *next;
	int fd;
};

#endif
