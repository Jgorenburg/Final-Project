#ifndef FS_H
#define FS_H

#include "structs.h"

struct file *f_open(char *filename, char *mode);
size_t f_read(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(void *ptr, size_t size, size_t nmemb, FILE *stream);
int f_close(FILE *stream);
int f_seek(FILE *stream, long int offset, int whence);
void f_rewind(FILE *stream);
int f_stat(int fd, struct stat *buf);
int f_remove(char *filename);
struct file *f_opendir(int fd); // is a directory handle just the directory file?
int f_closedir(struct file *dirp);
int f_mkdir(char *path, mode_t mode);
int f_rmdir(char *path);
int f_mount(char *source, char *target, char *filesystemtype, unsigned long mountflags, void *data);
int f_umount(char *target);

#endif