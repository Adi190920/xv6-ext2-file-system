#include"types.h"
struct filesystem_type {
 char * name;
 struct vfs_operations * ops;
 struct inode_operations * iops;
//  struct list_head fs_list;
};


struct inode_operations {
 struct inode * (* dirlookup )( struct inode * dp , char * name , uint * off );
 void (* iupdate )( struct inode * ip );
 void (* itrunc )( struct inode * ip );
 void (* cleanup )( struct inode * ip );
 uint (* bmap )( struct inode * ip , uint bn );
 void (* ilock )( struct inode * ip );
 void (* iunlock )( struct inode * ip );
 void (* stati )( struct inode * ip , struct stat * st );
 int (* readi )( struct inode * ip , char * dst , uint off , uint n );
 int (* writei )( struct inode * ip , char * src , uint off , uint n );
 int (* dirlink )( struct inode * dp , char * name , uint inum , uint type );
 int (* unlink )( struct inode * dp , uint off );
 int (* isdirempty )( struct inode * dp );

};
struct vfs_operations {
 int (* fs_init )( void );
//  int (* mount )( struct inode * devi , struct inode * ip );
//  int (* unmount )( struct inode *);
 struct inode * (* getroot )( int , int );
 void (* readsb )( int dev , struct superblock * sb );    
 struct inode * (* ialloc )( uint dev , short type );
 uint (* balloc )( uint dev );
 void (* bzero )( int dev , int bno );
 void (* bfree )( int dev , uint b );
 void (* brelse )( struct buf * b );
 void (* bwrite )( struct buf * b );
 struct buf * (* bread )( uint dev , uint blockno );
 int (* namecmp )( const char *s , const char * t );
};
