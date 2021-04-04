struct file{
    struct inode_ops *inode_ops;

};

struct inode_ops{
    char* (*read)(int fd, int bytes);
};