#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "file.h"
#include "ext2_file.h"

char p[10] = "hello worl";
struct inode_ops ext2_ops = {
    .read = &ext2_read
};
struct file ext2_file = {
    .inode_ops = &ext2_ops,
};

char* ext2_read(int fd, int bytes){
    char *str = malloc(sizeof(char)*bytes);
    int i, j = 0;
    for(i = strlen(p) - fd ; i > -1 && bytes; bytes--){
        str[j++] = p[i--];
    }  
    return str; 
}

int main(){
    char *q;
    q = ext2_file.inode_ops->read(2, 4);
    printf("%s\n", q);
}