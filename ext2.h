

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
#define EXT2_ADDR_PER_BLOCK(s)      (EXT2_BLOCK_SIZE(s) / sizeof (uint))
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
  uint s_inodes_count;    /* Inodes count */
  uint s_blocks_count;    /* Blocks count */
  uint s_r_blocks_count;  /* Reserved blocks count */
  uint s_free_blocks_count;  /* Free blocks count */
  uint s_free_inodes_count;  /* Free inodes count */
  uint s_first_data_block;  /* First Data Block */
  uint s_log_block_size;  /* Block size */
  uint s_log_frag_size;  /* Fragment size */
  uint s_blocks_per_group;  /* # Blocks per group */
  uint s_frags_per_group;  /* # Fragments per group */
  uint s_inodes_per_group;  /* # Inodes per group */
  uint s_mtime;    /* Mount time */
  uint s_wtime;    /* Write time */
  ushort s_mnt_count;    /* Mount count */
  ushort s_max_mnt_count;  /* Maximal mount count */
  ushort s_magic;    /* Magic signature */
  ushort s_state;    /* File system state */
  ushort s_errors;    /* Behaviour when detecting errors */
  ushort s_minor_rev_level;   /* minor revision level */
  uint s_lastcheck;    /* time of last check */
  uint s_checkinterval;  /* max. time between checks */
  uint s_creator_os;    /* OS */
  uint s_rev_level;    /* Revision level */
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
  uint s_first_ino;     /* First non-reserved inode */
  ushort s_inode_size;     /* size of inode structure */
  ushort s_block_group_nr;   /* block group # of this superblock */
  uint s_feature_compat;   /* compatible feature set */
  uint s_feature_incompat;   /* incompatible feature set */
  uint s_feature_ro_compat;   /* readonly-compatible feature set */
  uchar  s_uuid[16];    /* 128-bit uuid for volume */
  char   s_volume_name[16];   /* volume name */
  char   s_last_mounted[64];   /* directory where last mounted */
  uint s_algorithm_usage_bitmap; /* For compression */

  /*
   * Performance hints.  Directory preallocation should only
   * happen if the EXT2_COMPAT_PREALLOC flag is on.
   */
  uchar  s_prealloc_blocks;  /* Nr of blocks to try to preallocate*/
  uchar  s_prealloc_dir_blocks;  /* Nr to preallocate for dirs */
  ushort s_padding1;

  /*
   * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
   */
  uchar  s_journal_uuid[16];  /* uuid of journal superblock */
  uint s_journal_inum;    /* inode number of journal file */
  uint s_journal_dev;    /* device number of journal file */
  uint s_last_orphan;    /* start of list of inodes to delete */
  uint s_hash_seed[4];    /* HTREE hash seed */
  uchar  s_def_hash_version;  /* Default hash version to use */
  uchar  s_reserved_char_pad;
  ushort s_reserved_word_pad;
  uint s_default_mount_opts;
  uint s_first_meta_bg;   /* First metablock block group */
  uint s_reserved[190];  /* Padding to the end of the block */
};
/*
 * Structure of a blocks group descriptor
 */
struct ext2_group_desc
{
        uint   bg_block_bitmap;        /* Blocks bitmap block */
        uint   bg_inode_bitmap;        /* Inodes bitmap block */
        uint   bg_inode_table;         /* Inodes table block */
        ushort   bg_free_blocks_count;   /* Free blocks count */
        ushort   bg_free_inodes_count;   /* Free inodes count */
        ushort   bg_used_dirs_count;     /* Directories count */
        ushort   bg_flags;
        uint   bg_exclude_bitmap_lo;   /* Exclude bitmap for snapshots */
        ushort   bg_block_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+bitmap) LSB */
        ushort   bg_inode_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+bitmap) LSB */
        ushort   bg_itable_unused;       /* Unused inodes count */
        ushort   bg_checksum;            /* crc16(s_uuid+group_num+group_desc)*/
};
struct ext2_inode {
/*00*/	ushort	i_mode;		/* File mode */
	ushort	i_uid;		/* Low 16 bits of Owner Uid */
	uint	i_size;		/* Size in bytes */
	uint	i_atime;	/* Access time */
	uint	i_ctime;	/* Inode change time */
/*10*/	uint	i_mtime;	/* Modification time */
	uint	i_dtime;	/* Deletion Time */
	ushort	i_gid;		/* Low 16 bits of Group Id */
	ushort	i_links_count;	/* Links count */
	uint	i_blocks;	/* Blocks count */
/*20*/	uint	i_flags;	/* File flags */
	union {
		struct {
			uint	l_i_version; /* was l_i_reserved1 */
		} linux1;
		struct {
			uint  h_i_translator;
		} hurd1;
	} osd1;				/* OS dependent 1 */
/*28*/	uint	i_block[EXT2_N_BLOCKS];/* Pointers to blocks */
/*64*/	uint	i_generation;	/* File version (for NFS) */
	uint	i_file_acl;	/* File ACL */
	uint	i_size_high;
/*70*/	uint	i_faddr;	/* Fragment address */
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
			uchar	h_i_frag;	/* Fragment number */
			uchar	h_i_fsize;	/* Fragment size */
			ushort	h_i_mode_high;
			ushort	h_i_uid_high;
			ushort	h_i_gid_high;
			uint	h_i_author;
		} hurd2;
	} osd2;				/* OS dependent 2 */
};

struct ext2_dir_entry_2 {
	uint	inode;			/* Inode number */
	ushort	rec_len;		/* Directory entry length */
	uchar	name_len;		/* Name length */
	uchar	file_type;
	char	name[EXT2_NAME_LEN];	/* File name */
};

#define IO(inum, exs)     ((inum - 1) % exs.s_inodes_per_group)  

#define GN(inum, exs)     ((inum - 1) / exs.s_inodes_per_group )

#define NINDIRECT1        (BSIZE / sizeof(uint))

#define NDINDIRECT        (BSIZE / sizeof(uint))*NINDIRECT1

#define NTINDIRECT        (BSIZE / sizeof(uint))*NDINDIRECT

#define EXT2_MAXFILE       (NINDIRECT1 + NDINDIRECT + NTINDIRECT)

#define EXT2_INODES_PER_BLOCK(exs)  (exs.s_inodes_per_group / exs.s_blocks_per_group)

//file type
#define S_IFMT  00170000
#define S_IFSOCK 0140000
#define S_IFLNK  0120000
#define S_IFREG  0100000
#define S_IFBLK  0060000
#define S_IFDIR  0040000
#define S_IFCHR  0020000
#define S_IFIFO  0010000
#define S_ISUID  0004000
#define S_ISGID  0002000
#define S_ISVTX  0001000
#define S_ISLNK(m)     (((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)     (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)     (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)     (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)     (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)    (((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)    (((m) & S_IFMT) == S_IFSOCK)