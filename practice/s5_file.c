#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "file.h"

static struct inode_ops s5_ops = {
    .read = &s5_read
};
static struct file s5_file = {
    .inode_ops = &s5_ops,
};

char* s5_read(struct inode *inode){
    int bytes = 4, fd = 2;
    char *str = malloc(sizeof(char)*bytes);
    int i, j = 0;
    for(i = fd; i < strlen(inode->str) && bytes; bytes--){
        str[j++] = inode->str[i++];
    }  
    return str; 
}
