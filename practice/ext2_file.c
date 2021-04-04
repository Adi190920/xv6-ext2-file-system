#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "file.h"


static struct inode_ops ext2_ops = {
    .read = &ext2_read
};
static struct file ext2_file = {
    .inode_ops = &ext2_ops,
};

char* ext2_read(struct inode *inode ){
    int bytes = 4, fd = 2;
    char *str = malloc(sizeof(char)*bytes);
    int i, j = 0;
    for(i = strlen(inode->str) - fd ; i > -1 && bytes; bytes--){
        str[j++] = inode->str[i--];
    }  
    return str; 
}

