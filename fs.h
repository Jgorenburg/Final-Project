#ifndef FS_H
#define FS_H

#include "structs.h"
#include "shared.h"
// #include <sys/stat.h>


extern int curr_fd;
extern int uid;

void free_diskimage(struct diskimage *di);

/* helper functions */
int check_valid_fd(int fd);
void rel_or_abs_path(const char *filename);
struct filent find_file_in_dir(int dir, char *filename);
int strend(const char *s, const char *t);
int create_file(int dir, char type, char *filename, int permission);
int increase_fd_count();
void update_superblock();
void update_inode(int inode);
void update_db(struct datablock *db);
struct datablock get_data(int inode, int block_num);
struct datablock get_dblock(int datablock);
struct datablock get_iblock(int iblock, int block_num);
struct datablock get_i2block(int i2block, int *block_num);
struct datablock get_i3block(int i3block, int *block_num);
void create_free_blocks(int block_num);
void clean_dblock(int datablock, int *total_size);
void clean_iblock(int iblock, int *total_size);
void clean_i2block(int i2block, int *total_size);
void clean_i3block(int i3block, int *total_size);
int write_file(const void *ptr, size_t size, size_t nmemb, int fd, int file_size);
int remove_dir(int dir);
int write_data(int inode, int block_num, void *data);
int find_free();
void write_dblock(int dblock, void *data);
void write_iblock(int iblock, int block_num, void *data, int available[4]);
void write_i2block(int i2block, int *block_num, void *data, int available[4]);
void write_i3block(int i3block, int *block_num, void *data, int available[4]);

/* library functions */
int f_open(const char *filename, const char *mode);
size_t f_read(void *ptr, size_t size, size_t nmemb, int fd);
size_t f_write(const void *ptr, size_t size, size_t nmemb, int fd);
int f_close(int fd);
int f_seek(int fd, long int offset, int whence);
void f_rewind(int fd);
int f_stat(int fd, struct stat *buf);
int f_remove(const char *filename);
int f_opendir(const char *dirname);
struct filent f_readdir(int fd);
int f_closedir(int fd);
int f_mkdir(const char *dirname, mode_t mode);
int f_rmdir(const char *dirname);
int f_mount(const char *source, const char *target, int mountflags, void *data);
int f_umount(const char *target, int flags);

#endif
