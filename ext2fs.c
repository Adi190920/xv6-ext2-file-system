#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "ext2.h"
#include "vfs.h"
#include "buf.h"
#include "file.h"


// static uint   balloc( uint dev );
// static void   bfree( int dev , uint b );
// static uint   bmap( struct inode * ip , uint bn );
// static void   itrunc(struct inode*);

struct inode_operations ext2fs_iops =  {
  // .dirlookup  =   &dirlookup,
  // .iupdate    =   &iupdate,
  // .iput       =   &iput,
  // .itrunc     =   &itrunc,
  // // .cleanup    =   &cleanup,
  // .bmap       =   &bmap,
  // .ilock      =   &ext2fs_ilock,
  // .iunlock    =   &iunlock,
  // .iunlockput =   &iunlockput,
  // .stati      =   &stati,
  // .readi      =   &readi,
  // .writei     =   &writei,
  // .dirlink    =   &dirlink,
  // // .unlink     =   &unlink,
  // // .isdirempty =   &isdirempty,
  // // .iinit      =   &iinit,
  // // .getroot    =   &getroot,
  // // .readsb     =   &readsb,   
  // .ialloc     =   &ialloc,
  // .balloc     =   &balloc,
  // .bfree      =   &bfree,
  // .namecmp    =   &namecmp
};

struct ext2_fs ext2fs[NINODE];
//  = {
//   .name = "ext2fs",
//   .iops = &ext2fs_iops,
// };

struct ext2_superblock exs;

void
ext2_readsb(int dev, struct ext2_superblock *sb)
{
  struct buf *bp;

  bp = bread(dev, 0);
  memmove(sb, bp->data + 1024, sizeof(*sb));
  brelse(bp);
}

void ext2_iinit(int dev){
  int i = 0;
  for(i = 0; i < NINODE; i++){
    ext2fs[i].busy = 0;
    ext2fs->iops = &ext2fs_iops;
    ext2fs->name = "ext2fs";
  }
	ext2_readsb(dev, &exs);
  cprintf("ext2_sb: magic %x icount = %d bcount = %d log block size  %d inodes per group \
   %d first inode %d inode size %d\n", exs.s_magic, exs.s_inodes_count, exs.s_blocks_count,\
    exs.s_log_block_size, exs.s_inodes_per_group, exs.s_first_ino, exs.s_inode_size);

}

// void
// ext2fs_ilock(struct inode *ip)
// {
//   struct buf *bp;
//   // struct dinode *dip;
//   struct ext2_group_desc *gd;
//   struct ext2_inode_large *in;
//   int group_no, inode_off;
  
//   if(ip == 0 || ip->ref < 1)
//     panic("ilock");

//   acquiresleep(&ip->lock);

//   if(ip->valid == 0){
//     group_no = GN(ip->inum, exs.s_inodes_per_group);
//     inode_off = IO(ip->inum, exs.s_inodes_per_group);
//     bp = bread(ip->dev, BSIZE + sizeof(struct ext2_group_desc)*(group_no));
//     memmove(gd, bp->data, sizeof(struct ext2_group_desc));
//     bp = bread(ip->dev, gd.bg_inode_table * BSIZE + inode_offs * exs.s_inode_size);
//     memmove(in, bp->data, exs.s_inode_size);
//     // bp = bread(ip->dev, IBLOCK(ip->inum, exs));
//     // dip = (struct dinode*)bp->data + ip->inum%IPB;
//     ip->type = in->i_mode;
//     // ip->major = in->major;
//     // ip->minor = in->minor;
//     ip->nlink = in->i_links_count;
//     ip->size = in->i_size;
//     ip->file_type = &ext2fs;
//     memmove(ip->addrs, dip->addrs, sizeof(ip->addrs));
//     brelse(bp);
//     ip->valid = 1;
//     if(ip->type == 0)
//       panic("ilock: no type");
//   }
// }
