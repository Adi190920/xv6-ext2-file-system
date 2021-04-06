struct file{
    struct inode_ops *inode_ops;

};


struct inode{
    char str[5];
    struct file *file_type;
};

struct inode_ops{
    char* (*read)(struct inode *inode);
};

char * ext2_read(struct inode *inode);
char* s5_read(struct inode *inode);