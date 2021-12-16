#ifndef MAIN_H
#define MAIN_H

#include <sys/stat.h>
#include "parser.h"
#include "executor.h"
//#include "shared.h"
#include "linkedlist.h"

void set_dir_img(struct diskimage *di, FILE *dsk);
int main();

#endif
