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
static uint ext2_balloc(uint , uint );
static void ext2_bfree(int , uint );
static uint ext2_bmap(struct inode *, uint );
static void ext2_itrunc(struct inode *);

struct inode_operations ext2fs_iops = {
    .dirlookup = &ext2_dirlookup,
    .iupdate = &ext2_iupdate,
    .iput = &ext2_iput,
    .ilock = &ext2_ilock,
    .iunlock = &iunlock,
    .iunlockput = &iunlockput,
    .stati = &stati,
    .readi = &ext2_readi,
    .writei = &ext2_writei,
    .dirlink = &ext2_dirlink,
    .unlink     =   &ext2_unlink,
    .isdirempty =   &ext2_isdirempty,
    .ialloc = &ext2_ialloc,
    .namecmp = &ext2_namecmp
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
static struct inode *iget(uint dev, uint inum);
#define min(a, b) ((a) < (b) ? (a) : (b))

void ext2_readsb(int dev, struct ext2_superblock *sb)
{
  struct buf *bp;
  bp = bread(dev, 0);
  memmove(sb, bp->data + 1024, sizeof(*sb));
  brelse(bp);
}

static uint
zeroth_bit(char *bmap)
{
  int b, m;
  for (b = 0; b < exs.s_blocks_per_group * 8; b++)
  {
    m = 1 << (b % 8);
    if ((bmap[b / 8] & m) == 0)
    {
      bmap[b / 8] = bmap[b / 8] | m;
      return b;
    }
  }
  return -1;
}
// Allocate a zeroed disk block.
static uint
ext2_balloc(uint dev, uint inum)
{
  uint bblock;
  struct buf *bp, *gdbp;
  struct ext2_group_desc gd;
  int zbit, group_no = GN(inum, exs);
  gdbp = bread(dev, 1);
  memmove(&gd, gdbp->data + group_no * sizeof(gd), sizeof(gd));
  brelse(gdbp);
  bp = bread(dev, gd.bg_block_bitmap);
  if((zbit = zeroth_bit((char *)bp->data)) > -1){
    bblock = gd.bg_block_bitmap + zbit;
    bwrite(bp);
    bzero(dev, bblock);
    brelse(bp);
    return bblock;
  }
  brelse(bp);
  panic("balloc: out of blocks");
}
// Free a disk block.
static void
ext2_bfree(int dev, uint b)
{
  struct buf *gd_bp, *bp;
  int m;
  struct ext2_group_desc gd;
  uint bg = b / exs.s_blocks_per_group;
  uint index = b % exs.s_blocks_per_group;
  gd_bp = bread(dev, 1);
  memmove(&gd, gd_bp->data + bg * sizeof(gd), sizeof(gd));
  bp = bread(dev, gd.bg_block_bitmap);
  index -= gd.bg_block_bitmap;
  m = 1 << (index % 8);
  if ((bp->data[index / 8] & m) == 0)
    panic("freeing free block");
  bp->data[index / 8] &= ~m;
  bwrite(bp);
  brelse(bp);
  brelse(gd_bp);
}
// Free a inode.
static void
ext2_ifree(struct inode *ip)
{
  int group_no, m;;
  struct buf *bp, * gd_bp;
  struct ext2_group_desc gd;
  group_no = GN(ip->inum, exs);
  gd_bp = bread(ip->dev, 1);
  memmove(&gd, gd_bp->data + group_no * sizeof(gd), sizeof(gd));
  brelse(gd_bp);
  bp = bread(ip->dev, gd.bg_inode_bitmap);
  uint index = (ip->inum - 1)% exs.s_inodes_per_group;
  m = 1 << (index % 8);
  if ((bp->data[index / 8] & m) == 0)
    panic("freeing free inode");
  bp->data[index / 8] &= ~m;
  bwrite(bp);
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
  int i, index, inode_index, inum;
  uint blocknum, b_grp_count = exs.s_blocks_count / exs.s_blocks_per_group;
  struct buf *bp, *ibp, *gd_bp;
  struct ext2_group_desc gd;
  struct ext2_inode *in;
  for (i = 0; i <= b_grp_count; i++){
    gd_bp = bread(dev, 1);
    memmove(&gd, gd_bp->data + sizeof(gd) * i, sizeof(struct ext2_group_desc));
    brelse(gd_bp);
    bp = bread(dev, gd.bg_inode_bitmap);
    index = zeroth_bit((char *)bp->data);
    if(index == -1){
      brelse(gd_bp);
      brelse(bp);
      continue;
    }
    blocknum = gd.bg_inode_table + index / (BSIZE / sizeof(struct ext2_inode));
    ibp = bread(dev, blocknum);
    inode_index = index % (BSIZE / sizeof(struct ext2_inode));
    in = (struct ext2_inode*)ibp->data + inode_index;
    memset(in, 0, sizeof(*in));
    if(type == T_DIR)
      in->i_mode = S_IFDIR;
    if(type == T_FILE)
      in->i_mode = S_IFREG;
    bwrite(ibp);
    bwrite(bp);
    brelse(ibp);
    brelse(bp);
    inum = i * exs.s_inodes_per_group + index + 1;
    return iget(dev, inum);
  }
  brelse(bp);
  panic("ialloc: no inodes");
}
void
fill_ext2_fields(struct ext2_inode *in)
{
  in->i_blocks = 0;
  in->i_dtime = 0;
  in->i_faddr = 0;
  in->i_file_acl = 0;
  in->i_flags = 0;
  in->i_generation = 0;
  in->i_gid = 0;
  in->i_mtime = 0;
  in->i_size_high = 0;
  in->i_uid = 0;
  in->i_atime = 0;
}
// Copy a modified in-memory inode to disk.
// Must be called after every change to an ip->xxx field
// that lives on disk, since i-node cache is write-through.
// Caller must hold ip->lock.
void ext2_iupdate(struct inode *ip)
{
  struct buf *bp, *gdbp;
  struct ext2_group_desc gd;
  struct ext2_inode in;
  struct ext2_addrs *addrs;
  int group_no, inode_off;
  uint blocknum, inode_index;
  group_no = GN(ip->inum, exs);
  inode_off = IO(ip->inum, exs);
  gdbp = bread(ip->dev, 1);
  memmove(&gd, gdbp->data + group_no * sizeof(gd), sizeof(gd));
  blocknum = gd.bg_inode_table + inode_off / (BSIZE / exs.s_inode_size);
  inode_index = inode_off % (BSIZE / exs.s_inode_size);
  bp = bread(ip->dev, blocknum);
  memmove(&in, bp->data + inode_index *exs.s_inode_size, sizeof(in));
  if(ip->type == T_DIR)
    in.i_mode = S_IFDIR;
  if(ip->type == T_FILE)
    in.i_mode = S_IFREG;
  in.i_links_count = ip->nlink;
  in.i_size = ip->size;
  fill_ext2_fields(&in);
  addrs = (struct ext2_addrs *)ip->addrs;
  memmove(in.i_block, addrs->addrs, sizeof(addrs->addrs));
  memmove(bp->data + (inode_off * exs.s_inode_size), &in, sizeof(in));
  bwrite(bp); 
  brelse(gdbp);
  brelse(bp);
}

// Find the inode with number inum on device dev
// and return the in-memory copy. Does not lock
// the inode and does not read it from disk.
static struct inode *
iget(uint dev, uint inum)
{
  struct inode *ip, *empty;
  int i, j;
  acquire(&icache.lock);
  // Is the inode already cached?
  empty = 0;
  for (ip = &icache.inode[0]; ip < &icache.inode[NINODE]; ip++){
    if (ip->ref > 0 && ip->dev == dev && ip->inum == inum){
      ip->ref++;
      release(&icache.lock);
      return ip;
    }
    if (empty == 0 && ip->ref == 0) // Remember empty slot.
      empty = ip;
  }
  for (i = 0; i < NINODE; i++){
    if (addrs[i].busy == 0)
      break;
  }
  for (j = 0; j < NINODE; j++){
    if (ext2_addrs[j].busy == 0)
      break;
  }
  // Recycle an inode cache entry.
  if (empty == 0)
    panic("iget: no inodes");

  ip = empty;
  ip->dev = dev;
  ip->inum = inum;
  ip->ref = 1;
  ip->valid = 0;
  if (dev == ROOTDEV){
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

void 
ext2_ilock(struct inode *ip)
{
  struct buf *bp, *bp1;
  struct ext2_group_desc gd;
  struct ext2_inode in;
  int group_no, inode_off;
  struct ext2_addrs *addrs;
  uint blocknum, inode_index;
  if (ip == 0 || ip->ref < 1)
    panic("ilock");

  acquiresleep(&ip->lock);

  if (ip->valid == 0){
    group_no = GN(ip->inum, exs);
    inode_off = IO(ip->inum, exs);
    bp1 = bread(ip->dev, 1);
    memmove(&gd, bp1->data + sizeof(gd) * (group_no), sizeof(gd));
    brelse(bp1);
    blocknum = gd.bg_inode_table + inode_off / (BSIZE / exs.s_inode_size);
    inode_index = inode_off % (BSIZE / exs.s_inode_size);
    bp = bread(ip->dev, blocknum);
    memmove(&in, bp->data + inode_index * exs.s_inode_size, sizeof(in));
    if (S_ISDIR(in.i_mode) || in.i_mode == T_DIR){
      ip->type = T_DIR;
    }
    else if (S_ISREG(in.i_mode) || in.i_mode == T_FILE){
      ip->type = T_FILE;
    }
    ip->major = 0;
    ip->minor = 0;
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
  if (ip->valid && ip->nlink == 0){
    acquire(&icache.lock);
    int r = ip->ref;
    release(&icache.lock);
    if (r == 1){
      // inode has no links and no other references: truncate and free.
      ext2_ifree(ip);
      ext2_itrunc(ip);
      ip->type = 0;
      ip->size = 0;
      ip->file_type->iops->iupdate(ip);
      ip->valid = 0;
      ip->file_type = 0;
    }
  }
  releasesleep(&ip->lock);

  acquire(&icache.lock);
  ip->ref--;
  if (ip->ref == 0){
    addrs->busy = 0;
    ip->file_type = 0;
  }
  release(&icache.lock);
}

static uint
ext2_bmap(struct inode *ip, uint bn)
{
  uint addr, *a, *b, *c;
  struct buf *bp, *bp1, *bp2;
  struct ext2_addrs *ad;
  ad = (struct ext2_addrs *)ip->addrs;
  if (bn < EXT2_NDIR_BLOCKS){
    if ((addr = ad->addrs[bn]) == 0){
      ad->addrs[bn] = addr = ext2_balloc(ip->dev, ip->inum);
    }
    return addr;
  }
  bn -= EXT2_NDIR_BLOCKS;
  if (bn < NINDIRECT1){
    if ((addr = ad->addrs[EXT2_IND_BLOCK]) == 0)
      ad->addrs[EXT2_IND_BLOCK] = addr = ext2_balloc(ip->dev, ip->inum);
    bp = bread(ip->dev, addr);
    a = (uint *)bp->data;
    if ((addr = a[bn]) == 0)
      a[bn] = addr = ext2_balloc(ip->dev, ip->inum);
    brelse(bp);
    return addr;
  }
  bn -= NINDIRECT1;
  if (bn < NDINDIRECT){
    if ((addr = ad->addrs[EXT2_DIND_BLOCK]) == 0)
      ad->addrs[EXT2_DIND_BLOCK] = addr = ext2_balloc(ip->dev, ip->inum);
    bp = bread(ip->dev, addr);
    a = (uint *)bp->data;
    if ((addr = a[bn / NINDIRECT1]) == 0)
      a[bn / NINDIRECT1] = addr = ext2_balloc(ip->dev, ip->inum);
    bp1 = bread(ip->dev, addr);
    b = (uint *)bp->data;
    if ((addr = b[bn / NINDIRECT1]) == 0)
      b[bn / NINDIRECT1] = addr = ext2_balloc(ip->dev, ip->inum);
    brelse(bp1);
    brelse(bp);
    return addr;
  }
  bn -= NDINDIRECT;
  if (bn < NTINDIRECT){
    if ((addr = ad->addrs[EXT2_TIND_BLOCK]) == 0)
      ad->addrs[EXT2_TIND_BLOCK] = addr = ext2_balloc(ip->dev, ip->inum);
    bp = bread(ip->dev, addr);
    a = (uint *)bp->data;
    if ((addr = a[bn / NINDIRECT1]) == 0)
      a[bn / NINDIRECT1] = addr = ext2_balloc(ip->dev, ip->inum);
    bp1 = bread(ip->dev, addr);
    b = (uint *)bp->data;
    if ((addr = b[bn / NINDIRECT1]) == 0)
      b[bn / NINDIRECT1] = addr = ext2_balloc(ip->dev, ip->inum);
    bp2 = bread(ip->dev, addr);
    c = (uint *)bp->data;
    if ((addr = c[bn / NINDIRECT1]) == 0)
      b[bn / NINDIRECT1] = addr = ext2_balloc(ip->dev, ip->inum);
    brelse(bp2);
    brelse(bp1);
    brelse(bp);
    return addr;
  }
  panic("bmap: out of range");
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
  for (i = 0; i < EXT2_NDIR_BLOCKS; i++){
    if (addrs->addrs[i]){
      ext2_bfree(ip->dev, addrs->addrs[i]);
      addrs->addrs[i] = 0;
    }
  }
  if (addrs->addrs[EXT2_IND_BLOCK]){
    bp = bread(ip->dev, addrs->addrs[EXT2_IND_BLOCK]);
    a = (uint *)bp->data;
    for (j = 0; j < (BSIZE / sizeof(uint)); j++){
      if (a[j]){
        ext2_bfree(ip->dev, a[j]);
        a[j] = 0;
      }
    }
    brelse(bp);
    ext2_bfree(ip->dev, addrs->addrs[EXT2_IND_BLOCK]);
    addrs->addrs[EXT2_IND_BLOCK] = 0;
  }
  if (addrs->addrs[EXT2_DIND_BLOCK]){
    bp = bread(ip->dev, addrs->addrs[EXT2_DIND_BLOCK]);
    a = (uint *)bp->data;
    for (j = 0; j < (BSIZE / sizeof(uint)); j++){
      if (a[j]){
        bp1 = bread(ip->dev, a[j]);
        b = (uint *)bp->data;
        for (i = 0; i < (BSIZE / sizeof(uint)); i++){
          if (b[i]){
            ext2_bfree(ip->dev, b[i]);
            b[i] = 0;
          }
        }
        brelse(bp1);
        ext2_bfree(ip->dev, a[j]);
        a[j] = 0;
      }
    }
    brelse(bp);
    ext2_bfree(ip->dev, addrs->addrs[EXT2_DIND_BLOCK]);
    addrs->addrs[EXT2_DIND_BLOCK] = 0;
  }
  if (addrs->addrs[EXT2_TIND_BLOCK]){
    bp = bread(ip->dev, addrs->addrs[EXT2_DIND_BLOCK]);
    a = (uint *)bp->data;
    for (j = 0; j < (BSIZE / sizeof(uint)); j++){
      if (a[j]){
        bp1 = bread(ip->dev, a[j]);
        b = (uint *)bp1->data;
        for (i = 0; i < (BSIZE / sizeof(uint)); i++){
          if (b[i]){
            bp2 = bread(ip->dev, a[j]);
            c = (uint *)bp2->data;
            for (k = 0; k < (BSIZE / sizeof(uint)); k++){
              if (c[k]){
                ext2_bfree(ip->dev, c[k]);
                c[k] = 0;
              }
            }
            brelse(bp2);
            ext2_bfree(ip->dev, b[i]);
            b[i] = 0;
          }
        }
        brelse(bp1);
        ext2_bfree(ip->dev, a[j]);
        a[j] = 0;
      }
    }
    brelse(bp);
    ext2_bfree(ip->dev, addrs->addrs[EXT2_DIND_BLOCK]);
    addrs->addrs[EXT2_TIND_BLOCK] = 0;
    ip->size = 0;
    ip->file_type->iops->iupdate(ip);
  }
}

//PAGEBREAK!
// Read data from inode.
// Caller must hold ip->lock.
int ext2_readi(struct inode *ip, char *dst, uint off, uint n)
{
  uint tot, m = 0;
  struct buf *bp;
  if (ip->type == T_DEV){
    if (ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].read)
      return -1;
    return devsw[ip->major].read(ip, dst, n);
  }

  if (off > ip->size || off + n < off)
    return -1;
  if (off + n > ip->size)
    n = ip->size - off;
  for (tot = 0; tot < n; tot += m, off += m, dst += m){
    bp = bread(ip->dev, ext2_bmap(ip, (off + m) / BSIZE));
    m = min(n - tot, BSIZE - off % BSIZE);
    memmove(dst, bp->data + off % BSIZE, m);
    brelse(bp);
  }
  return n;
}

// PAGEBREAK!
// Write data to inode.
// Caller must hold ip->lock.
int ext2_writei(struct inode *ip, char *src, uint off, uint n)
{
  uint tot, m = 0;
  struct buf *bp;

  if (ip->type == T_DEV){
    if (ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].write)
      return -1;
    return devsw[ip->major].write(ip, src, n);
  }

  if (off > ip->size || off + n < off)
    return -1;
  if (off + n > EXT2_MAXFILE * BSIZE)
    return -1;
  for (tot = 0; tot < n; tot += m, off += m, src += m){
    bp = bread(ip->dev, ext2_bmap(ip, off / BSIZE));
    m = min(n - tot, BSIZE - off % BSIZE);
    memmove(bp->data + off % BSIZE, src, m);
    bwrite(bp);
    brelse(bp);
  }
  if (n > 0 && off > ip->size){
    ip->size = off;
    ip->file_type->iops->iupdate(ip);
  }
  return n;
}

int
ext2_namecmp(const char *s, const char *t)
{
  return strncmp(s, t, EXT2_NAME_LEN);
}
// Look for a directory entry in a directory.
// If found, set *poff to byte offset of entry.
struct inode *
ext2_dirlookup(struct inode *dp, char *name, uint *poff)
{
  uint off, inum;
  struct ext2_dir_entry_2 de;
  
  for (off = 0; off < dp->size; off += de.rec_len){
    if (dp->file_type->iops->readi(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
      panic("dirlookup read");
    if (de.inode == 0){
      continue;
    }
    de.name[de.name_len] = 0;
    cprintf("dirlookup : inode : %d, name : %s  rec len : %d\n", de.inode, de.name, de.rec_len);
    if (dp->file_type->iops->namecmp(name, de.name) == 0){
      // entry matches path element
      if (poff)
        *poff = off;
      inum = de.inode;
      return iget(dp->dev, inum);
    }
  }

  return 0;
}

// Write a new directory entry (name, inum) into the directory dp.
int ext2_dirlink(struct inode *dp, char *name, uint inum)
{
  int off;
  struct ext2_dir_entry_2 de;
  struct inode *ip;

  // Check that name is not present.
  if ((ip = dp->file_type->iops->dirlookup(dp, name, 0)) != 0){
    ip->file_type->iops->iput(ip);
    return -1;
  }

  // Look for an empty dirent.
  for (off = 0; off < dp->size; off += de.rec_len){
    if (dp->file_type->iops->readi(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
      panic("dirlink read");
    if (de.rec_len == BSIZE - off % BSIZE)
      break;
  }
  de.rec_len = sizeof(de) + strlen(de.name) - EXT2_NAME_LEN;
  if (dp->file_type->iops->writei(dp, (char *)&de, off, de.rec_len) != de.rec_len)
    panic("dirlink");
  off += de.rec_len;
  strncpy(de.name, name, EXT2_NAME_LEN);
  de.inode = inum;
  de.name_len = strlen(de.name);
  de.rec_len = BSIZE - off % BSIZE;
  if (dp->file_type->iops->writei(dp, (char *)&de, off, de.rec_len) != de.rec_len)
    panic("dirlink");
  return 0;
}
