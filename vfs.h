#include"types.h"
struct filesystem_type {
 char * name;
 struct inode_operations * iops;
};


struct inode_operations {
 struct inode * (* dirlookup )( struct inode * dp , char * name , uint * off );
//  struct inode * (* iget )( uint dev, uint inum );
 void (* iupdate )( struct inode * ip );
 void (* iput )( struct inode * ip );
 void (* itrunc )( struct inode * ip );
//  void (* cleanup )( struct inode * ip );
 uint (* bmap )( struct inode * ip , uint bn );
 void (* ilock )( struct inode * ip );
 void (* iunlock )( struct inode * ip );
 void (* stati )( struct inode * ip , struct stat * st );
 int (* readi )( struct inode * ip , char * dst , uint off , uint n );
 int (* writei )( struct inode * ip , char * src , uint off , uint n );
 int (* dirlink )( struct inode * dp , char * name , uint inum);
//  int (* unlink )( struct inode * dp , uint off );
//  int (* isdirempty )( struct inode * dp );
//  int (* iinit )(int dev );
//  struct inode * (* getroot )( int , int );
//  void (* readsb )( int dev , struct superblock * sb );    
 struct inode * (* ialloc )( uint dev , short type );
 uint (* balloc )( uint dev );
//  void (* bzero )( int dev , int bno );
 void (* bfree )( int dev , uint b );
//  void (* brelse )( struct buf * b );
//  void (* bwrite )( struct buf * b );
//  struct buf * (* bread )( uint dev , uint blockno );
 int (* namecmp )( const char *s , const char * t );
 void (* iunlockput)(struct inode *ip);
};


