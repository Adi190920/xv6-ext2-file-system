
struct ext2_fs {
  char * name;
  uint busy;
  struct inode_operations * iops;
  uint addrs[EXT2_N_BLOCKS];
};
struct fs {
  char  name[10];
  uint busy;
  struct inode_operations * iops;
  uint addrs[NDIRECT + 1];
};

struct inode_operations {
  struct inode * (* dirlookup )( struct inode * dp , char * name , uint * off );
  void (* iupdate )( struct inode * ip );
  void (* iput )( struct inode * ip );
  void (* itrunc )( struct inode * ip );
  // void (* cleanup )( struct inode * ip );
  uint (* bmap )( struct inode * ip , uint bn );
  void (* ilock )( struct inode * ip );
  void (* iunlock )( struct inode * ip );
  void (* iunlockput )( struct inode * ip );
  void (* stati )( struct inode * ip , struct stat * st );
  int (* readi )( struct inode * ip , char * dst , uint off , uint n );
  int (* writei )( struct inode * ip , char * src , uint off , uint n );
  int (* dirlink )( struct inode * dp , char * name , uint inum );
  // int (* unlink )( struct inode * dp , uint off );
  // int (* isdirempty )( struct inode * dp );
  // int (* iinit )(int dev );
  // struct inode * (* getroot )( int , int );
  // void (* readsb )( int dev , struct superblock * sb );    
  struct inode * (* ialloc )( uint dev , short type );
  uint (* balloc )( uint dev );
  void (* bfree )( int dev , uint b );
  int (* namecmp )( const char *s , const char * t );
};


