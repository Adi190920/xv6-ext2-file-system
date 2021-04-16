
typedef unsigned char uint8;
typedef unsigned int uint32;

typedef unsigned long ext2_fsblk_t;
#define EXT2_ROOT_INO            2      /* Root inode */
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

/*
 * Constants relative to the data blocks
 */
#define EXT2_NDIR_BLOCKS		12
#define EXT2_IND_BLOCK			EXT2_NDIR_BLOCKS
#define EXT2_DIND_BLOCK			(EXT2_IND_BLOCK + 1)
#define EXT2_TIND_BLOCK			(EXT2_DIND_BLOCK + 1)
#define EXT2_N_BLOCKS			(EXT2_TIND_BLOCK + 1)

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
  ushort s_mnt_count;    /* Mount count */
  ushort s_max_mnt_count;  /* Maximal mount count */
  ushort s_magic;    /* Magic signature */
  ushort s_state;    /* File system state */
  ushort s_errors;    /* Behaviour when detecting errors */
  ushort s_minor_rev_level;   /* minor revision level */
  uint32 s_lastcheck;    /* time of last check */
  uint32 s_checkinterval;  /* max. time between checks */
  uint32 s_creator_os;    /* OS */
  uint32 s_rev_level;    /* Revision level */
  ushort s_def_resuid;    /* Default uid for reserved blocks */
  ushort s_def_resgid;    /* Default gid for reserved blocks */

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
  ushort s_inode_size;     /* size of inode structure */
  ushort s_block_group_nr;   /* block group # of this superblock */
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
  ushort s_padding1;

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
  ushort s_reserved_word_pad;
  uint32 s_default_mount_opts;
  uint32 s_first_meta_bg;   /* First metablock block group */
  uint32 s_reserved[190];  /* Padding to the end of the block */
};
/*
 * Structure of a blocks group descriptor
 */
struct ext2_group_desc
{
        uint32   bg_block_bitmap;        /* Blocks bitmap block */
        uint32   bg_inode_bitmap;        /* Inodes bitmap block */
        uint32   bg_inode_table;         /* Inodes table block */
        ushort   bg_free_blocks_count;   /* Free blocks count */
        ushort   bg_free_inodes_count;   /* Free inodes count */
        ushort   bg_used_dirs_count;     /* Directories count */
        ushort   bg_flags;
        uint32   bg_exclude_bitmap_lo;   /* Exclude bitmap for snapshots */
        ushort   bg_block_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+bitmap) LSB */
        ushort   bg_inode_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+bitmap) LSB */
        ushort   bg_itable_unused;       /* Unused inodes count */
        ushort   bg_checksum;            /* crc16(s_uuid+group_num+group_desc)*/
};
struct ext2_inode_large {
/*00*/	ushort	i_mode;		/* File mode */
	ushort	i_uid;		/* Low 16 bits of Owner Uid */
	uint32	i_size;		/* Size in bytes */
	uint32	i_atime;	/* Access time */
	uint32	i_ctime;	/* Inode Change time */
/*10*/	uint32	i_mtime;	/* Modification time */
	uint32	i_dtime;	/* Deletion Time */
	ushort	i_gid;		/* Low 16 bits of Group Id */
	ushort	i_links_count;	/* Links count */
	uint32	i_blocks;	/* Blocks count */
/*20*/	uint32	i_flags;	/* File flags */
	union {
		struct {
			uint32	l_i_version; /* was l_i_reserved1 */
		} linux1;
		struct {
			uint32  h_i_translator;
		} hurd1;
	} osd1;				/* OS dependent 1 */
/*28*/	uint32	i_block[EXT2_N_BLOCKS];/* Pointers to blocks */
/*64*/	uint32	i_generation;	/* File version (for NFS) */
	uint32	i_file_acl;	/* File ACL */
	uint32	i_size_high;
/*70*/	uint32	i_faddr;	/* Fragment address */
	union {
		struct {
			ushort	l_i_blocks_hi;
			ushort	l_i_file_acl_high;
			ushort	l_i_uid_high;	/* these 2 fields    */
			ushort	l_i_gid_high;	/* were reserved2[0] */
			ushort	l_i_checksum_lo; /* crc32c(uuid+inum+inode) */
			ushort	l_i_reserved;
		} linux2;
		struct {
			uint8	h_i_frag;	/* Fragment number */
			uint8	h_i_fsize;	/* Fragment size */
			ushort	h_i_mode_high;
			ushort	h_i_uid_high;
			ushort	h_i_gid_high;
			uint32	h_i_author;
		} hurd2;
	} osd2;				/* OS dependent 2 */
/*80*/	ushort	i_extra_isize;
	ushort	i_checksum_hi;	/* crc32c(uuid+inum+inode) */
	uint32	i_ctime_extra;	/* extra Change time (nsec << 2 | epoch) */
	uint32	i_mtime_extra;	/* extra Modification time (nsec << 2 | epoch) */
	uint32	i_atime_extra;	/* extra Access time (nsec << 2 | epoch) */
/*90*/	uint32	i_crtime;	/* File creation time */
	uint32	i_crtime_extra;	/* extra File creation time (nsec << 2 | epoch)*/
	uint32	i_version_hi;	/* high 32 bits for 64-bit version */
/*9c*/	uint32   i_projid;       /* Project ID */
};

#define IO(inum, exs)     ((inum - 1) % exs.s_inodes_per_group)  

#define GN(inum, exs)     ((inum - 1) / exs.s_inodes_per_group )