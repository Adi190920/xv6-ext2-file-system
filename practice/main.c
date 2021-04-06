#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "file.h"

extern struct file s5_file, ext2_file;
extern struct inode_operations s5_ops, ext2_ops;
int main(){
    struct inode s5_inode = {.str = "hello", .file_type = &s5_file};
    struct inode ext2_inode = {.str = "My na", .file_type = &ext2_file};
    printf("%s\n", ext2_read(&ext2_inode));
    printf("%s", s5_read(&s5_inode));
    return 0;
}
