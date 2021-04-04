#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "file.h"
#include "s5.h"
char p[10] = "hello worl";
struct inode_ops s5_ops = {
    .read = &s5_read
};
struct file s5_file = {
    .inode_ops = &s5_ops,
};

char* s5_read(int fd, int bytes){
    char *str = malloc(sizeof(char)*bytes);
    int i, j = 0;
    for(i = fd; i < strlen(p) && bytes; bytes--){
        str[j++] = p[i++];
    }  
    return str; 
}

int main(){
    char *q;
    q = s5_file.inode_ops->read(2, 4);
    printf("%s\n", q);
}