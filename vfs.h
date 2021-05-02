#include"types.h"
struct filesystem_type {
  char * name;
  struct inode_operations * iops;
};
struct addrs{
  uint busy;
  uint addrs[NDIRECT + 1];
};
struct ext2_addrs{
  uint busy;
  uint addrs[EXT2_N_BLOCKS];
};
struct inode_operations {
  struct inode * (* dirlookup )( struct inode * dp , char * name , uint * off );
  void (* iupdate )( struct inode * ip );
  void (* iput )( struct inode * ip );
  void (* ilock )( struct inode * ip );
  void (* iunlock )( struct inode * ip );
  void (* iunlockput )( struct inode * ip );
  void (* stati )( struct inode * ip , struct stat * st );
  int (* readi )( struct inode * ip , char * dst , uint off , uint n );
  int (* writei )( struct inode * ip , char * src , uint off , uint n );
  int (* dirlink )( struct inode * dp , char * name , uint inum );
  void (* unlink )( struct inode * dp , uint off );
  int (* isdirempty )( struct inode * dp );   
  struct inode * (* ialloc )( uint dev , short type );
  int (* namecmp )( const char *s , const char * t );
  struct inode* (* namei )( char *s);
};


