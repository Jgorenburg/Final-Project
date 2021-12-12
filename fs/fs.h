#ifndef FS_H
#define FS_H

#include "structs.h"
#include <sys/stat.h>
#include <sys/types.h>

/* helper functions */
struct fileent find_file_in_dir(int dir, char *filename);
int strend(char *s, char *t);
int create_file(int dir, char type, char *filename, int permission);
int increase_fd_count();
void update_sb();
void update_inode(int inode);
struct datablock get_data(int inode, int block_num);
struct datablock get_dblock(int inode);

/* library functions */
int f_open(const char *filename, const char *mode);
size_t f_read(void *ptr, size_t size, size_t nmemb, int fd);
size_t fwrite(void *ptr, size_t size, size_t nmemb, int fd);
int f_close(int fd);
int f_seek(int fd, long int offset, int whence);
void f_rewind(int fd);
int f_stat(int fd, struct stat *buf);
int f_remove(const char *filename);
int f_opendir(const char *path);
struct fileent f_readdir(int fd);
int f_closedir(int fd);
int f_mkdir(const char *path, mode_t mode);
int f_rmdir(const char *path);
int f_mount(const char *source, const char *target, int mountflags, void *data);
int f_umount(const char *target, int flags);

#endif