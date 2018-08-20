
#ifndef __CREATE_DIR_H__
#define __CREATE_DIR_H__

#include <sys/stat.h>

//directory mode 07xx
#define DIR_MODE 0755

extern int recursive_make_dir(char *path, mode_t mode);
extern int get_file_size(const char* filename);

#endif


