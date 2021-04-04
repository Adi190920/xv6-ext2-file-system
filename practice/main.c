#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "file.h"

extern struct file s5_file, ext2_file;
int main(){
    struct inode s5_inode = {.str = "hello worl", .file_type = &s5_file};
    struct inode ext2_inode = {.str = "My name is", .file_type = &ext2_file};
    printf("%s", ext2_read(&ext2_inode));
    printf("%s", s5_read(&s5_inode));
    return 0;
}
