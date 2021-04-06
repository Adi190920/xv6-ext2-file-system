#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "file.h"


struct inode_ops ext2_ops = {
    .read = &ext2_read
};
struct file ext2_file = {
    .inode_ops = &ext2_ops,
};

char* ext2_read(struct inode *inode ){

    return inode->str; 
}

