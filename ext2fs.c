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
#include "icache.h"
// static uint   ext2_balloc( uint dev );
// static void   bfree( int dev , uint b );
// static uint   bmap( struct inode * ip , uint bn );
static void   ext2_itrunc(struct inode*);

struct inode_operations ext2fs_iops = {
    // .dirlookup  =   &dirlookup,
    .iupdate    =   &ext2_iupdate,
    // .iput       =   &iput,
    .itrunc     =   &ext2_itrunc,
    // // .cleanup    =   &cleanup,
    // .bmap       =   &bmap,
    .ilock = &ext2_ilock,
    .iunlock    =   &ext2_iunlock,
    // .iunlockput =   &iunlockput,
    // .stati      =   &ext2_stati,
    // .readi      =   &ext2_readi,
    // .writei     =   &writei,
    // .dirlink    =   &dirlink,
    // // .unlink     =   &unlink,
    // // .isdirempty =   &isdirempty,
    // // .iinit      =   &iinit,
    // // .getroot    =   &getroot,
    // // .readsb     =   &readsb,
    .ialloc = &ext2_ialloc,
    // .balloc     =   &ext2_balloc,
    // .bfree      =   &bfree,
    // .namecmp    =   &namecmp
};

struct filesystem_type ext2fs = {
  .name = "ext2fs",
  .iops = &ext2fs_iops,
};
struct ext2_addrs ext2_addrs[NINODE];
struct ext2_superblock exs;
extern struct icache icache;
extern struct filesystem_type inbuiltfs;
extern struct inode_operations inbuiltfs_iops;
extern struct addrs addrs[NINODE];
static struct inode* iget(uint dev, uint inum);

void ext2_readsb(int dev, struct ext2_superblock *sb)
{
  struct buf *bp;

  bp = bread(dev, 0);
  memmove(sb, bp->data + 1024, sizeof(*sb));
  brelse(bp);
}

void ext2_iinit(int dev)
{
  ext2_readsb(dev, &exs);
  cprintf("ext2_sb: magic %x icount = %d bcount = %d log block size  %d inodes per group \
   %d first inode %d inode size %d\n",
          exs.s_magic, exs.s_inodes_count, exs.s_blocks_count,
          exs.s_log_block_size, exs.s_inodes_per_group, exs.s_first_ino, exs.s_inode_size);
}

//PAGEBREAK!
// Allocate an inode on device dev.
// Mark it as allocated by  giving it type type.
// Returns an unlocked but allocated and referenced inode.
struct inode *
ext2_ialloc(uint dev, short type)
{
  int inum;
  struct buf *bp;
  struct ext2_group_desc gd;
  struct ext2_inode_large in;
  int group_no, inode_off;
  for (inum = 2; inum < exs.s_inodes_count; inum++)
  {
    group_no = GN(inum, exs);
    inode_off = IO(inum, exs);
    bp = bread(dev, BSIZE + sizeof(struct ext2_group_desc) * (group_no));
    memmove(&gd, bp->data, sizeof(struct ext2_group_desc));
    brelse(bp);
    bp = bread(dev, gd.bg_inode_table * BSIZE + inode_off * exs.s_inode_size);
    memmove(&in, bp->data, exs.s_inode_size);
    // bp = bread(dev, IBLOCK(inum, sb));
    // dip = (struct dinode*)bp->data + inum%IPB;
    if (in.i_mode == 0)
    { // a free inode
      memset(&in, 0, sizeof(in));
      in.i_mode = type;
      // log_write(bp);   // mark it allocated on the disk
      brelse(bp);
      return iget(dev, inum);
    }
    brelse(bp);
  }
  panic("ialloc: no inodes");
}

// Copy a modified in-memory inode to disk.
// Must be called after every change to an ip->xxx field
// that lives on disk, since i-node cache is write-through.
// Caller must hold ip->lock.
void ext2_iupdate(struct inode *ip)
{
  struct buf *bp;
  struct ext2_group_desc gd;
  struct ext2_inode_large in;
  struct ext2_addrs *addrs;
  int group_no, inode_off;
  group_no = GN(ip->inum, exs);
  inode_off = IO(ip->inum, exs);
  bp = bread(ip->dev, BSIZE + sizeof(struct ext2_group_desc) * (group_no));
  memmove(&gd, bp->data, sizeof(struct ext2_group_desc));
  brelse(bp);
  bp = bread(ip->dev, gd.bg_inode_table * BSIZE + inode_off * exs.s_inode_size);
  memmove(&in, bp->data, exs.s_inode_size);
  in.i_mode = ip->type;
  // dip->major = ip->major;
  // dip->minor = ip->minor;
  in.i_links_count = ip->nlink;
  in.i_size = ip->size;
  addrs = (struct ext2_addrs *)ip->addrs;
  memmove(in.i_block, addrs->addrs, sizeof(addrs->addrs));
  // log_write(bp);
  brelse(bp);
}

// Find the inode with number inum on device dev
// and return the in-memory copy. Does not lock
// the inode and does not read it from disk.
static struct inode*
iget(uint dev, uint inum)
{
  struct inode *ip, *empty;
  int i, j;
  acquire(&icache.lock);

  // Is the inode already cached?
  empty = 0;
  for(ip = &icache.inode[0]; ip < &icache.inode[NINODE]; ip++){
    if(ip->ref > 0 && ip->dev == dev && ip->inum == inum){
      ip->ref++;
      // ip->file_type = &inbuiltfs;
      release(&icache.lock);
      return ip;
    }
    if(empty == 0 && ip->ref == 0)    // Remember empty slot.
      empty = ip;
  }
  for(i = 0; i < NINODE; i++){
    if(addrs[i].busy == 0)
      break;
  }
  for(j = 0; j < NINODE; j++){
    if(ext2_addrs[j].busy == 0)
      break;
  }
  // Recycle an inode cache entry.
  if(empty == 0)
    panic("iget: no inodes");

  ip = empty;
  ip->dev = dev;
  ip->inum = inum;
  ip->ref = 1;
  ip->valid = 0; 
  if(dev == ROOTDEV){
    ip->file_type = &inbuiltfs;
    ip->addrs = (void *)&addrs[i];
    addrs[i].busy = 1;
  }
  else{
    ip->file_type = &ext2fs;
    ip->addrs = (void *)&ext2_addrs[j];
    ext2_addrs[j].busy = 1;
  }
  release(&icache.lock);
  return ip;
}

void ext2_ilock(struct inode *ip)
{
  struct buf *bp;
  struct ext2_group_desc gd;
  struct ext2_inode_large in;
  int group_no, inode_off;
  struct ext2_addrs *addrs;
  if (ip == 0 || ip->ref < 1)
    panic("ilock");

  acquiresleep(&ip->lock);

  if (ip->valid == 0)
  {
    group_no = GN(ip->inum, exs);
    inode_off = IO(ip->inum, exs);
    bp = bread(ip->dev, BSIZE + sizeof(struct ext2_group_desc) * (group_no));
    memmove(&gd, bp->data, sizeof(struct ext2_group_desc));
    bp = bread(ip->dev, gd.bg_inode_table * BSIZE + inode_off * exs.s_inode_size);
    memmove(&in, bp->data, exs.s_inode_size);
    // bp = bread(ip->dev, IBLOCK(ip->inum, exs));
    // dip = (struct dinode*)bp->data + ip->inum%IPB;
    ip->type = in.i_mode;
    // ip->major = in->major;
    // ip->minor = in->minor;
    ip->nlink = in.i_links_count;
    ip->size = in.i_size;
    addrs = (struct ext2_addrs *)ip->addrs;
    memmove(addrs->addrs, in.i_block, sizeof(addrs->addrs));
    brelse(bp);
    ip->valid = 1;
    if (ip->type == 0)
      panic("ilock: no type");
  }
}

// Unlock the given inode.
void
ext2_iunlock(struct inode *ip)
{
  if(ip == 0 || !holdingsleep(&ip->lock) || ip->ref < 1)
    panic("iunlock");

  releasesleep(&ip->lock);
}

// Drop a reference to an in-memory inode.
// If that was the last reference, the inode cache entry can
// be recycled.
// If that was the last reference and the inode has no links
// to it, free the inode (and its content) on disk.
// All calls to iput() must be inside a transaction in
// case it has to free the inode.
void
ext2_iput(struct inode *ip)
{
  struct ext2_addrs *addrs;
  addrs = (struct ext2_addrs *)ip->addrs;
  acquiresleep(&ip->lock);
  if(ip->valid && ip->nlink == 0){
    acquire(&icache.lock);
    int r = ip->ref;
    release(&icache.lock);
    if(r == 1){
      // inode has no links and no other references: truncate and free.
      ip->file_type->iops->itrunc(ip);
      ip->type = 0;
      ip->file_type->iops->iupdate(ip);
      ip->valid = 0;
      // fs->busy = 0;
      ip->file_type = 0;
    }
  }
  releasesleep(&ip->lock);

  acquire(&icache.lock);
  ip->ref--;
  if(ip->ref == 0){
    addrs->busy = 0;
    ip->file_type = 0;
  }
  release(&icache.lock);
}

// Common idiom: unlock, then put.
void
ext2_iunlockput(struct inode *ip)
{
  ip->file_type->iops->iunlock(ip);
  ip->file_type->iops->iput(ip);
}


// Truncate inode (discard contents).
// Only called when the inode has no links
// to it (no directory entries referring to it)
// and has no in-memory reference to it (is
// not an open file or current directory).
static void
ext2_itrunc(struct inode *ip)
{
  int i, j, k;
  struct buf *bp, *bp1, *bp2;
  uint *a, *b, *c;
  struct ext2_addrs *addrs;
  addrs = (struct ext2_addrs *)ip->addrs;
  for(i = 0; i < EXT2_NDIR_BLOCKS; i++){
    if(addrs->addrs[i]){
      ip->file_type->iops->bfree(ip->dev, addrs->addrs[i]);
      addrs->addrs[i] = 0;
    }
  }

  if(addrs->addrs[EXT2_IND_BLOCK]){
    bp = bread(ip->dev, addrs->addrs[EXT2_IND_BLOCK]);
    a = (uint32*)bp->data;
    for(j = 0; j < (BSIZE / sizeof(uint32)); j++){
      if(a[j])
        ip->file_type->iops->bfree(ip->dev, a[j]);
    }
    brelse(bp);
    ip->file_type->iops->bfree(ip->dev, addrs->addrs[EXT2_IND_BLOCK]);
    addrs->addrs[EXT2_IND_BLOCK] = 0;
  }
  if(addrs->addrs[EXT2_DIND_BLOCK]){
    bp = bread(ip->dev, addrs->addrs[EXT2_DIND_BLOCK]);
    a = (uint32*)bp->data;
    for(j = 0; j < (BSIZE / sizeof(uint32)); j++){
      if(a[j]){
        bp1 = bread(ip->dev, a[j]);
        b = (uint32*)bp->data;
        for(i = 0; i < (BSIZE / sizeof(uint32)); i++){
          if(b[i])
            ip->file_type->iops->bfree(ip->dev, b[i]);
        }
        brelse(bp1);
        ip->file_type->iops->bfree(ip->dev, a[j]); 
      }
    }
    brelse(bp);
    ip->file_type->iops->bfree(ip->dev, addrs->addrs[EXT2_DIND_BLOCK]);
    addrs->addrs[EXT2_DIND_BLOCK] = 0;
  }
  if(addrs->addrs[EXT2_TIND_BLOCK]){
    bp = bread(ip->dev, addrs->addrs[EXT2_DIND_BLOCK]);
    a = (uint32*)bp->data;
    for(j = 0; j < (BSIZE / sizeof(uint32)); j++){
      if(a[j]){
        bp1 = bread(ip->dev, a[j]);
        b = (uint32*)bp1->data;
        for(i = 0; i < (BSIZE / sizeof(uint32)); i++){
          if(b[i]){
            bp2 = bread(ip->dev, a[j]);
            c = (uint32*)bp2->data;
            for(k = 0; k < (BSIZE / sizeof(uint32)); k++){
              if(c[k])
                ip->file_type->iops->bfree(ip->dev, c[k]);
            }  
            brelse(bp2);
            ip->file_type->iops->bfree(ip->dev, b[i]);
          }
        }
        brelse(bp1);
        ip->file_type->iops->bfree(ip->dev, a[j]); 
      }
    }
    brelse(bp);
    ip->file_type->iops->bfree(ip->dev, addrs->addrs[EXT2_DIND_BLOCK]);
    addrs->addrs[EXT2_TIND_BLOCK] = 0;
  ip->size = 0;
  ip->file_type->iops->iupdate(ip);
  }
}

// Copy stat information from inode.
// Caller must hold ip->lock.
void
ext2_stati(struct inode *ip, struct stat *st)
{
  st->dev = ip->dev;
  st->ino = ip->inum;
  st->type = ip->type;
  st->nlink = ip->nlink;
  st->size = ip->size;
}
