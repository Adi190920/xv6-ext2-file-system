#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "file.h"

struct inode_ops s5_ops = {
    .read = &s5_read
};
struct file s5_file = {
    .inode_ops = &s5_ops,
};

char* s5_read(struct inode *inode){
    
    return inode->str; 
}
