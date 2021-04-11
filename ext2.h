typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef uint pde_t;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef unsigned long ext2_fsblk_t;

#define EXT2_MIN_BLKSIZE 1024
#define EXT2_SUPER_MAGIC 0xEF53

#define EXT2_MAX_BGC 40

#define EXT2_NAME_LEN 255
#define EXT2_DIR_PAD                    4
#define EXT2_DIR_ROUND                  (EXT2_DIR_PAD - 1)
#define EXT2_DIR_REC_LEN(name_len)      (((name_len)  8  EXT2_DIR_ROUND) & \
                                         ~EXT2_DIR_ROUND)
#define EXT2_MAX_REC_LEN                ((1<<16)-1)
#define EXT2_MIN_BLOCK_SIZE    1024
#define EXT2_MAX_BLOCK_SIZE         4096
#define EXT2_BLOCK_SIZE(s)          ((s)->blocksize)
#define EXT2_ADDR_PER_BLOCK(s)      (EXT2_BLOCK_SIZE(s) / sizeof (uint32))
#define EXT2_BLOCK_SIZE_BITS(s)     ((s)->s_blocksize_bits)
struct ext2_superblock {
  uint32 s_inodes_count;    /* Inodes count */
  uint32 s_blocks_count;    /* Blocks count */
  uint32 s_r_blocks_count;  /* Reserved blocks count */
  uint32 s_free_blocks_count;  /* Free blocks count */
  uint32 s_free_inodes_count;  /* Free inodes count */
  uint32 s_first_data_block;  /* First Data Block */
  uint32 s_log_block_size;  /* Block size */
  uint32 s_log_frag_size;  /* Fragment size */
  uint32 s_blocks_per_group;  /* # Blocks per group */
  uint32 s_frags_per_group;  /* # Fragments per group */
  uint32 s_inodes_per_group;  /* # Inodes per group */
  uint32 s_mtime;    /* Mount time */
  uint32 s_wtime;    /* Write time */
  uint16 s_mnt_count;    /* Mount count */
  uint16 s_max_mnt_count;  /* Maximal mount count */
  uint16 s_magic;    /* Magic signature */
  uint16 s_state;    /* File system state */
  uint16 s_errors;    /* Behaviour when detecting errors */
  uint16 s_minor_rev_level;   /* minor revision level */
  uint32 s_lastcheck;    /* time of last check */
  uint32 s_checkinterval;  /* max. time between checks */
  uint32 s_creator_os;    /* OS */
  uint32 s_rev_level;    /* Revision level */
  uint16 s_def_resuid;    /* Default uid for reserved blocks */
  uint16 s_def_resgid;    /* Default gid for reserved blocks */

  /*
   * These fields are for EXT2_DYNAMIC_REV superblocks only.
   *
   * Note: the difference between the compatible feature set and
   * the incompatible feature set is that if there is a bit set
   * in the incompatible feature set that the kernel doesn't
   * know about, it should refuse to mount the filesystem.
   *
   * e2fsck's requirements are more strict; if it doesn't know
   * about a feature in either the compatible or incompatible
   * feature set, it must abort and not try to meddle with
   * things it doesn't understand...
   */
  uint32 s_first_ino;     /* First non-reserved inode */
  uint16 s_inode_size;     /* size of inode structure */
  uint16 s_block_group_nr;   /* block group # of this superblock */
  uint32 s_feature_compat;   /* compatible feature set */
  uint32 s_feature_incompat;   /* incompatible feature set */
  uint32 s_feature_ro_compat;   /* readonly-compatible feature set */
  uint8  s_uuid[16];    /* 128-bit uuid for volume */
  char   s_volume_name[16];   /* volume name */
  char   s_last_mounted[64];   /* directory where last mounted */
  uint32 s_algorithm_usage_bitmap; /* For compression */

  /*
   * Performance hints.  Directory preallocation should only
   * happen if the EXT2_COMPAT_PREALLOC flag is on.
   */
  uint8  s_prealloc_blocks;  /* Nr of blocks to try to preallocate*/
  uint8  s_prealloc_dir_blocks;  /* Nr to preallocate for dirs */
  uint16 s_padding1;

  /*
   * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
   */
  uint8  s_journal_uuid[16];  /* uuid of journal superblock */
  uint32 s_journal_inum;    /* inode number of journal file */
  uint32 s_journal_dev;    /* device number of journal file */
  uint32 s_last_orphan;    /* start of list of inodes to delete */
  uint32 s_hash_seed[4];    /* HTREE hash seed */
  uint8  s_def_hash_version;  /* Default hash version to use */
  uint8  s_reserved_char_pad;
  uint16 s_reserved_word_pad;
  uint32 s_default_mount_opts;
  uint32 s_first_meta_bg;   /* First metablock block group */
  uint32 s_reserved[190];  /* Padding to the end of the block */
};

#define EXT2_NDIR_BLOCKS  12
#define EXT2_IND_BLOCK    EXT2_NDIR_BLOCKS
#define EXT2_DIND_BLOCK   (EXT2_IND_BLOCK  1)
#define EXT2_TIND_BLOCK   (EXT2_DIND_BLOCK  1)
#define EXT2_N_BLOCKS     (EXT2_TIND_BLOCK  1)