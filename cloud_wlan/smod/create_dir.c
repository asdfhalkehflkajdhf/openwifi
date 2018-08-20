#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include "create_dir.h"

static int one_dir_make(char *path, mode_t mode)
{
    if (mkdir(path, mode) != 0) {
        return -1;
    }

    chmod(path, mode);

    return 0;
}

static int get_pre_path(char *path)
{
    int len = strlen(path);

    while (len > 0 && path[len-1] == '/') {
        path[len-1] = '\0'; 
        len--;
    }

    while (len > 0 && path[len-1] != '/') {
        path[len-1] = '\0';
        len--;
    }    

    while (len > 0 && path[len-1] == '/') {
        path[len-1] = '\0'; 
        len--;
    }

    return 0;
}

int recursive_make_dir(char *path, mode_t mode)
{
    if (path == NULL || strlen(path) == 0) {
        return -1;
    } 

    struct stat info;
    if (stat(path, &info) == 0) {
        //directory exist
        if (info.st_mode & S_IFDIR) {
            return 0;
        }
        //same name file exist
        return -1;
    }

    //try create directory
    if (one_dir_make(path, mode) == 0) {
        return 0;
    }

    //get pre path
    char predir[2048];
    if (strlen(path) > sizeof(predir)-1) {
        return -1;
    }
    strncpy(predir, path, sizeof(predir));
    get_pre_path(predir);
    //create pre path
    if (recursive_make_dir(predir, mode) != 0) {
        return -1;
    } 

    //create current path again
    int ret = 0;
    ret = one_dir_make(path, mode);

    return ret;
}
int get_file_size(const char* filename)
{
    struct stat st;

    if (stat(filename, &st) == 0)
    {
        return st.st_size;
    }

    return 0;
}

