#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdio.h>
#include <stdlib.h>

#define NAME_LENGTH 100
#define N_DBLOCKS 10
#define N_IBLOCKS 4

// file information struct
struct stat {
    dev_t     st_dev;     /* ID of device containing file */
    ino_t     st_ino;     /* inode number */
    mode_t    st_mode;    /* protection */
    nlink_t   st_nlink;   /* number of hard links */
    uid_t     st_uid;     /* user ID of owner */
    gid_t     st_gid;     /* group ID of owner */
    dev_t     st_rdev;    /* device ID (if special file) */
    off_t     st_size;    /* total size, in bytes */
    blksize_t st_blksize; /* blocksize for file system I/O */
    blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
    time_t    st_atime;   /* time of last access */
    time_t    st_mtime;   /* time of last modification */
    time_t    st_ctime;   /* time of last status change */
};

struct file {
	int inode; // the i-node number for the file
	int size; // size of the entry (including padding)
	int type; // 0 for file, 1 for directory, 2 for more
	char[NAME_LENGTH + 1]; // name of file (with an /0)
    struct stat;
};

struct superblock {
	int size; // the size of blocks in bytes
	int inode_offset; // the offset of the inode region
	int data_offset; // the offset of the data region
    int inode_num; // the number of inodes
	int disk_blocks_num; // the number of disk blocks
	int free_block; // the pointer to the bitmap of free disk blocks
	int free_inode; // the pointer to the bitmap of free inodes
	int directories; // the number of directories
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
};

// directory entry struct
struct dirent {
    int inode_number; // pointer to the list of inodes in the dir
    int file_name; // pointer to the list of file names in the dir
};

#endif