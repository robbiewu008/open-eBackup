#ifndef XFSFS_H
#define XFSFS_H

#include <stdint.h>
#include <stddef.h>
#include <vector>
#include <queue>
#include <stack>
#include "afs/FileSystem.h"
#include "afs/ImgReader.h"
#include "afs/Bitmap.h"
#include "afs/FSCommon.h"

const int INODE_SET = 1;
const int BLOCK_SET = 2;

const uint8_t TARGET_FLAG = 2;

const uint8_t FILE_TYPE_0 = 0; // 不确定Entry是否含有文件类型域
const uint8_t FILE_TYPE_1 = 1; // Entry含有文件类型域
const uint8_t FILE_TYPE_2 = 2; // Entry不含文件类型域

const int INODESIZE = 2048;
const unsigned int BLOCKSIZE = 4096;

const int XFS_OFFSET_HDR = 0X64;
const int XFS_OFFSET_HDR_CRC = 0XB0;

const int XFS_OFFSET_TREE = 0XB0;
const int XFS_OFFSET_TREE_CRC = 0X154;

const int NAME_LEN = 256;
const int FILE_OFFSET = 0x10;

const int LIMIT_TAG = 0x30;
const int LIMIT_TAG_CRC = 0x60;

const int SERCTORSIZE = 512;

const int PTR_SIZE = 72;

const uint8_t BTREE_CRC_OFFSET = 24;

const int COUNT_NUM = 1000;

const int NAME_SEAM = 7;

const string PATH_PATTERN = "/";

#define BBSHIFT 9
const int XFS_DIR2_DATA_FREE_TAG = 0xffff;
const int XFS_DIR2_DATA_ALIGN = 8;
const int XFS_DIR3_FT_MAX = 9;

static inline uint32_t xfs_mask32lo(int n)
{
    return ((uint32_t)1 << (n)) - 1;
}

static inline uint64_t xfs_mask64lo(int n)
{
    return ((uint64_t)1 << (n)) - 1;
}

#define XFS_INO_MASK(k) (__uint32_t)((1ULL << (k)) - 1)
#define XFS_INO_OFFSET_BITS(mp) (mp)->sb_inopblog
#define XFS_INO_AGBNO_BITS(mp) (mp)->sb_agblklog
#define XFS_INO_AGINO_BITS(mp) (mp)->m_agino_log
#define XFS_INO_AGNO_BITS(mp) (mp)->m_agno_log
#define XFS_INO_BITS(mp) (XFS_INO_AGNO_BITS(mp) + XFS_INO_AGINO_BITS(mp))
#define XFS_INO_TO_AGNO(mp, i) ((xfs_agnumber_t)((i) >> XFS_INO_AGINO_BITS(mp)))
#define XFS_INO_TO_AGBNO(mp, i) (((xfs_agblock_t)(i) >> XFS_INO_OFFSET_BITS(mp)) & XFS_INO_MASK(XFS_INO_AGBNO_BITS(mp)))

#define XFS_DIR2_LEAF_SIZE (1ULL << (32 + 3))                 // /32G
#define XFS_DIR2_LEAF_OFFSET (XFS_DIR2_LEAF_SIZE / BLOCKSIZE) // /0x800000(8388608)
#define XFS_SB_VERSION_NUM(sb_versionnum) ((sb_versionnum) & 0x000f)
#define XFS_SB_HAS_INCOMPAT_FEATURE(sb_features_incompat, feature)  (((sb_features_incompat) & feature) != 0)
#define XFS_SB_VERSION_HASMOREBITS(sb_versionnum) \
    (XFS_SB_VERSION_NUM(sb_versionnum) == XFS_SB_VERSION_5 || ((sb_versionnum) & XFS_SB_VERSION_MOREBITSBIT))
#define XFS_SB_FEAT_INCOMPAT_FTYPE (1 << 0) /* filetype in dirent */
#define XFS_SB_VERSION2_FTYPE 0x00000200    /* inode type in dir */

/*
 * XFS_BIG_BLKNOS needs block layer disk addresses to be 64 bits.
 * XFS_BIG_INUMS requires XFS_BIG_BLKNOS to be set.
 */
#if defined(CONFIG_LBDAF) || (defined(BITS_PER_LONG) && (BITS_PER_LONG == 64))
#define XFS_BIG_BLKNOS 1
#define XFS_BIG_INUMS 1
#else
#define XFS_BIG_BLKNOS 0
#define XFS_BIG_INUMS 0
#endif

/*
 * Additional type declarations for XFS
 */
typedef int8_t __int8_t;
typedef uint8_t __uint8_t;
typedef int16_t __int16_t;
typedef uint16_t __uint16_t;
typedef int32_t __int32_t;
typedef uint32_t __uint32_t;
typedef int64_t __int64_t;
typedef uint64_t __uint64_t;

/*
 * These types are 64 bits on disk but are either 32 or 64 bits in memory.
 * Disk based types:
 */
typedef __uint64_t xfs_dfsbno_t;   /* blockno in filesystem (agno|agbno) */
typedef __uint64_t xfs_drfsbno_t;  /* blockno in filesystem (raw) */
typedef __uint64_t xfs_drtbno_t;   /* extent (block) in realtime area */
typedef __uint64_t xfs_dfiloff_t;  /* block number in a file */
typedef __uint64_t xfs_dfilblks_t; /* number of blocks in a file */

typedef __uint32_t uid_t;
typedef __uint32_t gid_t;
typedef __uint16_t uid16_t;
typedef __uint16_t gid16_t;

typedef __int8_t __s8;
typedef __int16_t __s16;
typedef __int32_t __s32;

typedef __uint8_t __u8;
typedef __uint16_t __u16;
typedef __uint32_t __u32;
typedef __uint64_t xfs_daddr_t;
typedef __uint64_t xfs_ino_t;      /* <inode> type */
typedef __uint64_t xfs_agblock_t;  /* blockno in alloc. group */
typedef __uint64_t xfs_agino_t;    /* inode # within allocation grp */
typedef __uint64_t xfs_extlen_t;   /* extent length in blocks */
typedef __uint64_t xfs_agnumber_t; /* allocation group number */
typedef __uint64_t xfs_extnum_t;   /* # of extents in a file */
typedef __uint64_t xfs_aextnum_t;  /* # extents in an attribute fork */
typedef __uint64_t xfs_fsize_t;    /* bytes in a file */
typedef __uint64_t xfs_ufsize_t;   /* unsigned bytes in a file */
typedef __uint64_t xfs_suminfo_t;  /* type of bitmap summary info */
typedef __uint64_t xfs_rtword_t;   /* word type for bitmap manipulations */
typedef __uint64_t xfs_lsn_t;      /* log sequence number */
typedef __uint64_t xfs_tid_t;      /* transaction identifier */
typedef __uint64_t xfs_dablk_t;    /* dir/attr block number (in file) */
typedef __uint64_t xfs_dahash_t;   /* dir/attr hash value */

/*
 * Memory based types are conditional.
 */
#if XFS_BIG_BLKNOS
typedef __uint64_t xfs_fsblock_t;  /* blockno in filesystem (agno|agbno) */
typedef __uint64_t xfs_rfsblock_t; /* blockno in filesystem (raw) */
typedef __uint64_t xfs_rtblock_t;  /* extent (block) in realtime area */
typedef __int64_t xfs_srtblock_t;  /* signed version of xfs_rtblock_t */
#else
typedef __uint32_t xfs_fsblock_t;  /* blockno in filesystem (agno|agbno) */
typedef __uint32_t xfs_rfsblock_t; /* blockno in filesystem (raw) */
typedef __uint32_t xfs_rtblock_t;  /* extent (block) in realtime area */
typedef __int32_t xfs_srtblock_t;  /* signed version of xfs_rtblock_t */
#endif

typedef __uint64_t xfs_fileoff_t; /* block number in a file */
typedef __int64_t xfs_sfiloff_t;  /* signed block number in a file */
typedef __uint64_t xfs_filblks_t; /* number of blocks in a file */

typedef enum {
    XFS_BTNUM_BNOi,
    XFS_BTNUM_CNTi,
    XFS_BTNUM_BMAPi,
    XFS_BTNUM_INOi,
    XFS_BTNUM_FINOi,
    XFS_BTNUM_MAX
} xfs_btnum_t;

typedef struct {
    unsigned char __u_bits[16];
} xfs_uuid_t;

#define XFS_SB_MAGIC 0x58465342 /* 'XFSB' */
#define XFS_SB_VERSION_1 1      /* 5.3, 6.0.1, 6.1 */
#define XFS_SB_VERSION_2 2      /* 6.2 - attributes */
#define XFS_SB_VERSION_3 3      /* 6.2 - new inode version */
#define XFS_SB_VERSION_4 4      /* 6.2+ - bitmask version */
#define XFS_SB_VERSION_5 5      /* CRC enabled filesystem */
#define XFS_SB_VERSION_MOREBITSBIT 0x8000

/*
 * Superblock - in core version.  Must match the ondisk version below.
 * Must be padded to 64 bit alignment.
 */
typedef struct xfs_sb {
    __uint32_t sb_magicnum;    /* magic number == XFS_SB_MAGIC */
    __uint32_t sb_blocksize;   /* logical block size, bytes */
    xfs_drfsbno_t sb_dblocks;  /* number of data blocks */
    xfs_drfsbno_t sb_rblocks;  /* number of realtime blocks */
    xfs_drtbno_t sb_rextents;  /* number of realtime extents */
    xfs_uuid_t sb_uuid;        /* file system unique id */
    xfs_dfsbno_t sb_logstart;  /* starting block of log if internal */
    xfs_ino_t sb_rootino;      /* root inode number */
    xfs_ino_t sb_rbmino;       /* bitmap inode for realtime extents */
    xfs_ino_t sb_rsumino;      /* summary inode for rt bitmap */
    xfs_agblock_t sb_rextsize; /* realtime extent size, blocks */
    xfs_agblock_t sb_agblocks; /* size of an allocation group */
    xfs_agnumber_t sb_agcount; /* number of allocation groups */
    xfs_extlen_t sb_rbmblocks; /* number of rt bitmap blocks */
    xfs_extlen_t sb_logblocks; /* number of log blocks */
    __uint16_t sb_versionnum;  /* header version == XFS_SB_VERSION */
    __uint16_t sb_sectsize;    /* volume sector size, bytes */
    __uint16_t sb_inodesize;   /* inode size, bytes */
    __uint16_t sb_inopblock;   /* inodes per block */
    char sb_fname[12];         /* file system name */
    __uint8_t sb_blocklog;     /* log2 of sb_blocksize */
    __uint8_t sb_sectlog;      /* log2 of sb_sectsize */
    __uint8_t sb_inodelog;     /* log2 of sb_inodesize */
    __uint8_t sb_inopblog;     /* log2 of sb_inopblock */
    __uint8_t sb_agblklog;     /* log2 of sb_agblocks (rounded up) */
    __uint8_t sb_rextslog;     /* log2 of sb_rextents */
    __uint8_t sb_inprogress;   /* mkfs is in progress, don't mount */
    __uint8_t sb_imax_pct;     /* max % of fs for inode space */
                               /* statistics */
    /*
     * These fields must remain contiguous.  If you really
     * want to change their layout, make sure you fix the
     * code in xfs_trans_apply_sb_deltas().
     */
    __uint64_t sb_icount;    /* allocated inodes */
    __uint64_t sb_ifree;     /* free inodes */
    __uint64_t sb_fdblocks;  /* free data blocks */
    __uint64_t sb_frextents; /* free realtime extents */
    /*
     * End contiguous fields.
     */
    xfs_ino_t sb_uquotino;      /* user quota inode */
    xfs_ino_t sb_gquotino;      /* group quota inode */
    __uint16_t sb_qflags;       /* quota flags */
    __uint8_t sb_flags;         /* misc. flags */
    __uint8_t sb_shared_vn;     /* shared version number */
    xfs_extlen_t sb_inoalignmt; /* inode chunk alignment, fsblocks */
    __uint32_t sb_unit;         /* stripe or raid unit */
    __uint32_t sb_width;        /* stripe or raid width */
    __uint8_t sb_dirblklog;     /* log2 of dir block size (fsbs) */
    __uint8_t sb_logsectlog;    /* log2 of the log sector size */
    __uint16_t sb_logsectsize;  /* sector size for the log, bytes */
    __uint32_t sb_logsunit;     /* stripe unit size for the log */
    __uint32_t sb_features2;    /* additional feature bits */

    /*
     * bad features2 field as a result of failing to pad the sb
     * structure to 64 bits. Some machines will be using this field
     * for features2 bits. Easiest just to mark it bad and not use
     * it for anything else.
     */
    __uint32_t sb_bad_features2;

    /* version 5 superblock fields start here */

    /* feature masks */
    __uint32_t sb_features_compat;
    __uint32_t sb_features_ro_compat;
    __uint32_t sb_features_incompat;
    __uint32_t sb_features_log_incompat;

    __uint32_t sb_crc; /* superblock crc */
    __uint32_t sb_pad;

    xfs_ino_t sb_pquotino; /* project quota inode */
    xfs_lsn_t sb_lsn;      /* last write sequence */

    /* must be padded to 64 bit alignment */
} xfs_sb_t;

/*
 * Superblock - on disk version.  Must match the in core version above.
 * Must be padded to 64 bit alignment.
 */
typedef struct xfs_dsb {
    __be32 sb_magicnum;   /* magic number == XFS_SB_MAGIC */
    __be32 sb_blocksize;  /* logical block size, bytes */
    __beu64 sb_dblocks;    /* number of data blocks */
    __beu64 sb_rblocks;    /* number of realtime blocks */
    __beu64 sb_rextents;   /* number of realtime extents */
    xfs_uuid_t sb_uuid;   /* file system unique id */
    __beu64 sb_logstart;   /* starting block of log if internal */
    __beu64 sb_rootino;    /* root inode number */
    __beu64 sb_rbmino;     /* bitmap inode for realtime extents */
    __beu64 sb_rsumino;    /* summary inode for rt bitmap */
    __be32 sb_rextsize;   /* realtime extent size, blocks */
    __be32 sb_agblocks;   /* size of an allocation group */
    __be32 sb_agcount;    /* number of allocation groups */
    __be32 sb_rbmblocks;  /* number of rt bitmap blocks */
    __be32 sb_logblocks;  /* number of log blocks */
    __be16 sb_versionnum; /* header version == XFS_SB_VERSION */
    __be16 sb_sectsize;   /* volume sector size, bytes */
    __be16 sb_inodesize;  /* inode size, bytes */
    __be16 sb_inopblock;  /* inodes per block */
    char sb_fname[12];    /* file system name */
    __u8 sb_blocklog;     /* log2 of sb_blocksize */
    __u8 sb_sectlog;      /* log2 of sb_sectsize */
    __u8 sb_inodelog;     /* log2 of sb_inodesize */
    __u8 sb_inopblog;     /* log2 of sb_inopblock */
    __u8 sb_agblklog;     /* log2 of sb_agblocks (rounded up) */
    __u8 sb_rextslog;     /* log2 of sb_rextents */
    __u8 sb_inprogress;   /* mkfs is in progress, don't mount */
    __u8 sb_imax_pct;     /* max % of fs for inode space */
                          /* statistics */
    /*
     * These fields must remain contiguous.  If you really
     * want to change their layout, make sure you fix the
     * code in xfs_trans_apply_sb_deltas().
     */
    __beu64 sb_icount;    /* allocated inodes */
    __beu64 sb_ifree;     /* free inodes */
    __beu64 sb_fdblocks;  /* free data blocks */
    __beu64 sb_frextents; /* free realtime extents */
    /*
     * End contiguous fields.
     */
    __beu64 sb_uquotino;    /* user quota inode */
    __beu64 sb_gquotino;    /* group quota inode */
    __be16 sb_qflags;      /* quota flags */
    __u8 sb_flags;         /* misc. flags */
    __u8 sb_shared_vn;     /* shared version number */
    __be32 sb_inoalignmt;  /* inode chunk alignment, fsblocks */
    __be32 sb_unit;        /* stripe or raid unit */
    __be32 sb_width;       /* stripe or raid width */
    __u8 sb_dirblklog;     /* log2 of dir block size (fsbs) */
    __u8 sb_logsectlog;    /* log2 of the log sector size */
    __be16 sb_logsectsize; /* sector size for the log, bytes */
    __be32 sb_logsunit;    /* stripe unit size for the log */
    __be32 sb_features2;   /* additional feature bits */
    /*
     * bad features2 field as a result of failing to pad the sb
     * structure to 64 bits. Some machines will be using this field
     * for features2 bits. Easiest just to mark it bad and not use
     * it for anything else.
     */
    __be32 sb_bad_features2;

    /* version 5 superblock fields start here */

    /* feature masks */
    __be32 sb_features_compat;
    __be32 sb_features_ro_compat;
    __be32 sb_features_incompat;
    __be32 sb_features_log_incompat;

    __le32 sb_crc; /* superblock crc */
    __be32 sb_pad;
    __beu64 sb_pquotino; /* project quota inode */
    __beu64 sb_lsn;      /* last write sequence */

    /* must be padded to 64 bit alignment */
} xfs_dsb_t;

/*
 * Btree number 0 is bno, 1 is cnt.  This value gives the size of the
 * arrays below.
 */
#define XFS_BTNUM_AGF ((int)XFS_BTNUM_CNTi + 1)
#define XFS_AGF_MAGIC 0x58414746

/*
 * The second word of agf_levels in the first a.g. overlaps the EFS
 * superblock's magic number.  Since the magic numbers valid for EFS
 * are > 64k, our value cannot be confused for an EFS superblock's.
 */
typedef struct xfs_agf {
    /*
     * Common allocation group header information
     */
    __be32 agf_magicnum;   /* magic number == XFS_AGF_MAGIC */
    __be32 agf_versionnum; /* header version == XFS_AGF_VERSION */
    __be32 agf_seqno;      /* sequence # starting from 0 */
    __be32 agf_length;     /* size in blocks of a.g. */
    /*
     * Freespace information
     */
    __be32 agf_roots[XFS_BTNUM_AGF];  /* root blocks */
    __be32 agf_spare0;                /* spare field */
    __be32 agf_levels[XFS_BTNUM_AGF]; /* btree levels */
    __be32 agf_spare1;                /* spare field */
    __be32 agf_flfirst;               /* first freelist block's index */
    __be32 agf_fllast;                /* last freelist block's index */
    __be32 agf_flcount;               /* count of blocks in freelist */
    __be32 agf_freeblks;              /* total free blocks */

    __be32 agf_longest;   /* longest free space */
    __be32 agf_btreeblks; /* # of blocks held in AGF btrees */
    xfs_uuid_t agf_uuid;  /* uuid of filesystem */

    /*
     * reserve some contiguous space for future logged fields before we add
     * the unlogged fields. This makes the range logging via flags and
     * structure offsets much simpler.
     */
    __beu64 agf_spare64[16];

    /* unlogged fields, written during buffer writeback. */
    __beu64 agf_lsn; /* last write sequence */
    __be32 agf_crc; /* crc of agf sector */
    __be32 agf_spare2;

    /* structure must be padded to 64 bit alignment */
} xfs_agf_t;

/*
 * Size of the unlinked inode hash table in the agi.
 */
#define XFS_AGI_UNLINKED_BUCKETS 64

typedef struct xfs_agi {
    /*
     * Common allocation group header information
     */
    __be32 agi_magicnum;   /* magic number == XFS_AGI_MAGIC */
    __be32 agi_versionnum; /* header version == XFS_AGI_VERSION */
    __be32 agi_seqno;      /* sequence # starting from 0 */
    __be32 agi_length;     /* size in blocks of a.g. */

    /*
     * Inode information
     * Inodes are mapped by interpreting the inode number, so no
     * mapping data is needed here.
     */
    __be32 agi_count;     /* count of allocated inodes */
    __be32 agi_root;      /* root of inode btree */
    __be32 agi_level;     /* levels in inode btree */
    __be32 agi_freecount; /* number of free inodes */

    __be32 agi_newino; /* new inode just allocated */
    __be32 agi_dirino; /* last directory inode chunk */
    /*
     * Hash table of inodes which have been unlinked but are
     * still being referenced.
     */
    __be32 agi_unlinked[XFS_AGI_UNLINKED_BUCKETS];

    xfs_uuid_t agi_uuid; /* uuid of filesystem */
    __be32 agi_crc;      /* crc of agi sector */
    __be32 agi_pad32;
    __beu64 agi_lsn; /* last write sequence */

    /* structure must be padded to 64 bit alignment */
} xfs_agi_t;

typedef struct xfs_agfl {
    __be32 agfl_magicnum;
    __be32 agfl_seqno;
    xfs_uuid_t agfl_uuid;
    __beu64 agfl_lsn;
    __be32 agfl_crc;
    __be32 agfl_bno[0]; /* actually XFS_AGFL_SIZE(mp) */
} __attribute__((packed)) xfs_agfl_t;


#define XFS_DINODE_MAGIC 0x494e /* 'IN' */
#define XFS_DINODE_GOOD_VERSION(v) ((v) >= 1 && (v) <= 3)

typedef struct xfs_timestamp {
    __be32 t_sec;  /* timestamp seconds */
    __be32 t_nsec; /* timestamp nanoseconds */
} xfs_timestamp_t;

/*
 * On-disk inode structure.
 *
 * This is just the header or "dinode core", the inode is expanded to fill a
 * variable size the leftover area split into a data and an attribute fork.
 * The format of the data and attribute fork depends on the format of the
 * inode as indicated by di_format and di_aformat.  To access the data and
 * attribute use the XFS_DFORK_DPTR, XFS_DFORK_APTR, and XFS_DFORK_PTR macros
 * below.
 *
 * There is a very similar struct icdinode in xfs_inode which matches the
 * layout of the first 96 bytes of this structure, but is kept in native
 * format instead of big endian.
 */
typedef struct xfs_dinode {
    __be16 di_magic;          /* inode magic # = XFS_DINODE_MAGIC */
    __be16 di_mode;           /* mode and type of file */
    __u8 di_version;          /* inode version */
    __u8 di_format;           /* format of di_c data */
    __be16 di_onlink;         /* old number of links to file */
    __be32 di_uid;            /* owner's user id */
    __be32 di_gid;            /* owner's group id */
    __be32 di_nlink;          /* number of links to file */
    __be16 di_projid_lo;      /* lower part of owner's project id */
    __be16 di_projid_hi;      /* higher part owner's project id */
    __u8 di_pad[6];           /* unused, zeroed space */
    __be16 di_flushiter;      /* incremented on flush */
    xfs_timestamp_t di_atime; /* time last accessed */
    xfs_timestamp_t di_mtime; /* time last modified */
    xfs_timestamp_t di_ctime; /* time created/inode modified */
    __beu64 di_size;           /* number of bytes in file */
    __beu64 di_nblocks;        /* # of direct & btree blocks used */
    __be32 di_extsize;        /* basic/minimum extent size for file */
    __be32 di_nextents;       /* number of extents in data fork */
    __be16 di_anextents;      /* number of extents in attribute fork */
    __u8 di_forkoff;          /* attr fork offs, <<3 for 64b align */
    __s8 di_aformat;          /* format of attr fork's data */
    __be32 di_dmevmask;       /* DMIG event mask */
    __be16 di_dmstate;        /* DMIG state info */
    __be16 di_flags;          /* random flags, XFS_DIFLAG_... */
    __be32 di_gen;            /* generation number */

    /* di_next_unlinked is the only non-core field in the old dinode */
    __be32 di_next_unlinked; /* agi unlinked list ptr */

    /* start of the extended dinode, writable fields */
    __le32 di_crc;         /* CRC of the inode */
    __beu64 di_changecount; /* number of attribute changes */
    __beu64 di_lsn;         /* flush sequence */
    __beu64 di_flags2;      /* more random flags */
    __u8 di_pad2[16];      /* more padding for future expansion */

    /* fields only written to during inode creation */
    xfs_timestamp_t di_crtime; /* time created */
    __beu64 di_ino;             /* inode number */
    xfs_uuid_t di_uuid;        /* UUID of the filesystem */
    /* structure must be padded to 64 bit alignment */
} xfs_dinode_t;

/*
 * Values for di_format
 */
typedef enum xfs_dinode_fmt {
    XFS_DINODE_FMT_DEV,     /* xfs_dev_t */
    XFS_DINODE_FMT_LOCAL,   /* bulk data */
    XFS_DINODE_FMT_EXTENTS, /* struct xfs_bmbt_rec */
    XFS_DINODE_FMT_BTREE,   /* struct xfs_bmdr_block */
    XFS_DINODE_FMT_UUID     /* uuid_t */
} xfs_dinode_fmt_t;

#define XFS_BMAP_MAGIC 0x424d4150     /* 'BMAP' */
#define XFS_BMAP_CRC_MAGIC 0x424d4133 /* 'BMA3' */

/*
 * Bmap root header, on-disk form only.
 */
typedef struct xfs_bmdr_block {
    __be16 bb_level;   /* 0 is a leaf */
    __be16 bb_numrecs; /* current # of data records */
} xfs_bmdr_block_t;

struct xfs_btree_block_info {
    __be32 bb_magic;   /* magic number for block type */
    __be16 bb_level;   /* 0 is a leaf */
    __be16 bb_numrecs; /* current # of data records */
    __beu64 bb_leftsib;
    __beu64 bb_rightsib;
    /* rest */
};

/*
 * Bmap btree record and extent descriptor.
 * l0:63 is an extent flag (value 1 indicates non-normal).
 * l0:9-62 are startoff.
 * l0:0-8 and l1:21-63 are startblock.
 * l1:0-20 are blockcount.
 */
typedef struct xfs_bmbt_rec {
    __beu64 l0, l1;
} xfs_bmbt_rec_t;

typedef __uint64_t xfs_bmbt_rec_base_t; /* use this for casts */
typedef xfs_bmbt_rec_t xfs_bmdr_rec_t;

/*
 * Possible extent formats.
 */
typedef enum {
    XFS_EXTFMT_NOSTATE = 0,
    XFS_EXTFMT_HASSTATE
} xfs_exntfmt_t;


/*
 * Possible extent states.
 */
typedef enum {
    XFS_EXT_NORM,
    XFS_EXT_UNWRITTEN,
    XFS_EXT_DMAPI_OFFLINE,
    XFS_EXT_INVALID
} xfs_exntst_t;

/*
 * Incore version of above.
 */
typedef struct xfs_bmbt_irec {
    xfs_fileoff_t br_startoff;   /* starting file offset */
    xfs_fsblock_t br_startblock; /* starting block number */
    xfs_filblks_t br_blockcount; /* number of blocks */
    xfs_exntst_t br_state;       /* extent state */
} xfs_bmbt_irec_t;

/*
 * Key structure for non-leaf levels of the tree.
 */
typedef struct xfs_bmbt_key {
    __beu64 br_startoff; /* starting file offset */
} xfs_bmbt_key_t, xfs_bmdr_key_t;

/* btree pointer type */
typedef struct xfs_bmap_key {
    __beu64 br_ptr; /* starting file offset */
} xfs_bmbt_ptr_t, xfs_bmdr_ptr_t;

/*
 * There are two on-disk btrees, one sorted by blockno and one sorted
 * by blockcount and blockno.  All blocks look the same to make the code
 * simpler; if we have time later, we'll make the optimizations.
 */
#define XFS_ABTB_MAGIC 0x41425442     /* 'ABTB' for bno tree */
#define XFS_ABTB_CRC_MAGIC 0x41423342 /* 'AB3B' */
#define XFS_ABTC_MAGIC 0x41425443     /* 'ABTC' for cnt tree */
#define XFS_ABTC_CRC_MAGIC 0x41423343 /* 'AB3C' */

/*
 * Data record/key structure
 */
typedef struct xfs_alloc_rec {
    __be32 ar_startblock; /* starting block number */
    __be32 ar_blockcount; /* count of free blocks */
} xfs_alloc_rec_t, xfs_alloc_key_t;

typedef struct xfs_alloc_rec_incore {
    xfs_agblock_t ar_startblock; /* starting block number */
    xfs_extlen_t ar_blockcount;  /* count of free blocks */
} xfs_alloc_rec_incore_t;

/* btree pointer type */
typedef __be32 xfs_alloc_ptr_t;

/*
 * There is a btree for the inode map per allocation group.
 */
#define XFS_IBT_MAGIC 0x49414254     /* 'IABT' */
#define XFS_IBT_CRC_MAGIC 0x49414233 /* 'IAB3' */

typedef __uint64_t xfs_inofree_t;
/*
 * Data record structure
 */
typedef struct xfs_inobt_rec {
    __be32 ir_startino;  /* starting inode number */
    __be32 ir_freecount; /* count of free inodes (set bits) */
    __beu64 ir_free;      /* free inode mask */
} xfs_inobt_rec_t;

typedef struct xfs_inobt_rec_incore {
    xfs_agino_t ir_startino; /* starting inode number */
    __int32_t ir_freecount;  /* count of free inodes (set bits) */
    xfs_inofree_t ir_free;   /* free inode mask */
} xfs_inobt_rec_incore_t;

/*
 * Key structure
 */
typedef struct xfs_inobt_key {
    __be32 ir_startino; /* starting inode number */
} xfs_inobt_key_t;

/* btree pointer type */
typedef __be32 xfs_inobt_ptr_t;

/*
 * Generic btree header.
 *
 * This is a combination of the actual format used on disk for short and long
 * format btrees.  The first three fields are shared by both format, but the
 * pointers are different and should be used with care.
 *
 * To get the size of the actual short or long form headers please use the size
 * macros below.  Never use sizeof(xfs_btree_block).
 *
 * The blkno, crc, lsn, owner and uuid fields are only available in filesystems
 * with the crc feature bit, and all accesses to them must be conditional on
 * that flag.
 */
struct xfs_btree_block {
    __be32 bb_magic;   /* magic number for block type */
    __be16 bb_level;   /* 0 is a leaf */
    __be16 bb_numrecs; /* current # of data records */
    union {
        struct {
            __be32 bb_leftsib;
            __be32 bb_rightsib;
            __beu64 bb_blkno;
            __beu64 bb_lsn;
            xfs_uuid_t bb_uuid;
            __be32 bb_owner;
            __le32 bb_crc;
        } s; /* short form pointers */
        struct {
            __beu64 bb_leftsib;
            __beu64 bb_rightsib;
            __beu64 bb_blkno;
            __beu64 bb_lsn;
            xfs_uuid_t bb_uuid;
            __beu64 bb_owner;
            __le32 bb_crc;
            __be32 bb_pad; /* padding for alignment */
        } l;               /* long form pointers */
    } bb_u;                /* rest */
};

#define XFS_BTREE_SBLOCK_LEN 16 /* size of a short form block */
#define XFS_BTREE_LBLOCK_LEN 24 /* size of a long form block */

/* sizes of CRC enabled btree blocks */
#define XFS_BTREE_SBLOCK_CRC_LEN (XFS_BTREE_SBLOCK_LEN + 40)
#define XFS_BTREE_LBLOCK_CRC_LEN (XFS_BTREE_LBLOCK_LEN + 48)

/*
 * This structure is common to both leaf nodes and non-leaf nodes in the Btree.
 *
 * It is used to manage a doubly linked list of all blocks at the same
 * level in the Btree, and to identify which type of block this is.
 */
#define XFS_DA_NODE_MAGIC 0xfebe    /* magic number: non-leaf blocks */
#define XFS_ATTR_LEAF_MAGIC 0xfbee  /* magic number: attribute leaf blks */
#define XFS_DIR2_LEAF1_MAGIC 0xd2f1 /* magic number: v2 dirlf single blks */
#define XFS_DIR2_LEAFN_MAGIC 0xd2ff /* magic number: v2 dirlf multi blks */

typedef struct xfs_da_blkinfo {
    __be32 forw;  /* previous block in list */
    __be32 back;  /* following block in list */
    __be16 magic; /* validity check on block */
    __be16 pad;   /* unused */
} xfs_da_blkinfo_t;

/*
 * CRC enabled directory structure types
 *
 * The headers change size for the additional verification information, but
 * otherwise the tree layouts and contents are unchanged. Hence the da btree
 * code can use the struct xfs_da_blkinfo for manipulating the tree links and
 * magic numbers without modification for both v2 and v3 nodes.
 */
#define XFS_DA3_NODE_MAGIC 0x3ebe   /* magic number: non-leaf blocks */
#define XFS_ATTR3_LEAF_MAGIC 0x3bee /* magic number: attribute leaf blks */
#define XFS_DIR3_LEAF1_MAGIC 0x3df1 /* magic number: v2 dirlf single blks */
#define XFS_DIR3_LEAFN_MAGIC 0x3dff /* magic number: v2 dirlf multi blks */

struct xfs_da3_blkinfo {
    /*
     * the node link manipulation code relies on the fact that the first
     * element of this structure is the struct xfs_da_blkinfo so it can
     * ignore the differences in the rest of the structures.
     */
    struct xfs_da_blkinfo hdr;
    __be32 crc;      /* CRC of block */
    __beu64 blkno;    /* first block of the buffer */
    __beu64 lsn;      /* sequence number of last write */
    xfs_uuid_t uuid; /* filesystem we belong to */
    __beu64 owner;    /* inode that owns the block */
};

/*
 * This is the structure of the root and intermediate nodes in the Btree.
 * The leaf nodes are defined above.
 *
 * Entries are not packed.
 *
 * Since we have duplicate keys, use a binary search but always follow
 * all match in the block, not just the first match found.
 */
#define XFS_DA_NODE_MAXDEPTH 5 /* max depth of Btree */

typedef struct xfs_da_node_hdr {
    struct xfs_da_blkinfo info; /* block type, links, etc. */
    __be16 __count;             /* count of active entries */
    __be16 __level;             /* level above leaves (leaf == 0) */
} xfs_da_node_hdr_t;

struct xfs_da3_node_hdr {
    struct xfs_da3_blkinfo info; /* block type, links, etc. */
    __be16 __count;              /* count of active entries */
    __be16 __level;              /* level above leaves (leaf == 0) */
    __be32 __pad32;
};

typedef struct xfs_da_node_entry {
    __be32 hashval; /* hash value for this descendant */
    __be32 before;  /* Btree block before this key */
} xfs_da_node_entry_t;

typedef struct xfs_da_intnode {
    struct xfs_da_node_hdr hdr;
    struct xfs_da_node_entry __btree[0];
} xfs_da_intnode_t;

struct xfs_da3_intnode {
    struct xfs_da3_node_hdr hdr;
    struct xfs_da_node_entry __btree[0];
};

/*
 * In-core version of the node header to abstract the differences in the v2 and
 * v3 disk format of the headers. Callers need to convert to/from disk format as
 * appropriate.
 */
struct xfs_da3_icnode_hdr {
    __uint32_t forw;
    __uint32_t back;
    __uint16_t magic;
    __uint16_t count;
    __uint16_t level;
};

/*
 * Directory version 2.
 *
 * There are 4 possible formats:
 * - shortform - embedded into the inode
 * - single block - data with embedded leaf at the end
 * - multiple data blocks, single leaf+freeindex block
 * - data blocks, node and leaf blocks (btree), freeindex blocks
 *
 * Note: many node blocks structures and constants are shared with the attr
 * code and defined in xfs_da_btree.h.
 */

#define XFS_DIR2_BLOCK_MAGIC 0x58443242 /* XD2B: single block dirs */
#define XFS_DIR2_DATA_MAGIC 0x58443244  /* XD2D: multiblock dirs */
#define XFS_DIR2_FREE_MAGIC 0x58443246  /* XD2F: free index blocks */

/*
 * Directory Version 3 With CRCs.
 *
 * The tree formats are the same as for version 2 directories.  The difference
 * is in the block header and dirent formats. In many cases the v3 structures
 * use v2 definitions as they are no different and this makes code sharing much
 * easier.
 *
 * Also, the xfs_dir3_*() functions handle both v2 and v3 formats - if the
 * format is v2 then they switch to the existing v2 code, or the format is v3
 * they implement the v3 functionality. This means the existing dir2 is a mix of
 * xfs_dir2/xfs_dir3 calls and functions. The xfs_dir3 functions are called
 * where there is a difference in the formats, otherwise the code is unchanged.
 *
 * Where it is possible, the code decides what to do based on the magic numbers
 * in the blocks rather than feature bits in the superblock. This means the code
 * is as independent of the external XFS code as possible as doesn't require
 * passing struct xfs_mount pointers into places where it isn't really
 * necessary.
 *
 * Version 3 includes:
 *
 * - a larger block header for CRC and identification purposes and so the
 * offsets of all the structures inside the blocks are different.
 *
 * - new magic numbers to be able to detect the v2/v3 types on the fly.
 */

#define XFS_DIR3_BLOCK_MAGIC 0x58444233 /* XDB3: single block dirs */
#define XFS_DIR3_DATA_MAGIC 0x58444433  /* XDD3: multiblock dirs */
#define XFS_DIR3_FREE_MAGIC 0x58444633  /* XDF3: free index blocks */

/*
 * Normalized offset (in a data block) of the entry, really xfs_dir2_data_off_t.
 * Only need 16 bits, this is the byte offset into the single block form.
 */
typedef struct {
    __uint8_t i[2];
} xfs_dir2_sf_off_t;

/*
 * Inode number stored as 8 8-bit values.
 */
typedef struct {
    __uint8_t i[8];
} xfs_dir2_ino8_t;

/*
 * Inode number stored as 4 8-bit values.
 * Works a lot of the time, when all the inode numbers in a directory
 * fit in 32 bits.
 */
typedef struct {
    __uint8_t i[4];
} xfs_dir2_ino4_t;

typedef union {
    xfs_dir2_ino8_t i8;
    xfs_dir2_ino4_t i4;
} xfs_dir2_inou_t;

/*
 * Directory layout when stored internal to an inode.
 *
 * Small directories are packed as tightly as possible so as to fit into the
 * literal area of the inode.  These "shortform" directories consist of a
 * single xfs_dir2_sf_hdr header followed by zero or more xfs_dir2_sf_entry
 * structures.  Due the different inode number storage size and the variable
 * length name field in the xfs_dir2_sf_entry all these structure are
 * variable length, and the accessors in this file should be used to iterate
 * over them.
 */
typedef struct xfs_dir2_sf_hdr {
    __uint8_t count;        /* count of entries */
    __uint8_t i8count;      /* count of 8-byte inode #s */
    xfs_dir2_inou_t parent; /* parent dir inode number */
} xfs_dir2_sf_hdr_t;

typedef struct xfs_dir2_sf_entry {
    __u8 namelen;             /* actual name length */
    xfs_dir2_sf_off_t offset; /* saved offset */
    __u8 name[0];             /* name, variable size */
    /*
     * A xfs_dir2_ino8_t or xfs_dir2_ino4_t follows here, at a
     * variable offset after the name.
     */
} xfs_dir2_sf_entry_t;

typedef struct xfs_dir2_info {
    __u8 filetype; /* type of inode we point to */
    xfs_dir2_inou_t xfs_inou;
} xfs_dir2_info_t;

typedef struct xfs_dir2_info_f {
    xfs_dir2_inou_t xfs_inou;
} xfs_dir2_info_f_t;

/* * Data block structures.
 *
 * A pure data block looks like the following drawing on disk:
 *
 * +-------------------------------------------------+
 * | xfs_dir2_data_hdr_t                             |
 * +-------------------------------------------------+
 * | xfs_dir2_data_entry_t OR xfs_dir2_data_unused_t |
 * | xfs_dir2_data_entry_t OR xfs_dir2_data_unused_t |
 * | xfs_dir2_data_entry_t OR xfs_dir2_data_unused_t |
 * | ...                                             |
 * +-------------------------------------------------+
 * | unused space                                    |
 * +-------------------------------------------------+
 *
 * As all the entries are variable size structures the accessors below should
 * be used to iterate over them.
 *
 * In addition to the pure data blocks for the data and node formats,
 * most structures are also used for the combined data/freespace "block"
 * format below.
 */
/*
 * Describe a free area in the data block.
 *
 * The freespace will be formatted as a xfs_dir2_data_unused_t.
 */
#define XFS_DIR2_DATA_FREE_TAG 0xffff
#define XFS_DIR2_DATA_FD_COUNT 3

typedef struct xfs_dir2_data_free {
    __be16 offset; /* start of freespace */
    __be16 length; /* length of freespace */
} xfs_dir2_data_free_t;

/*
 * Header for the data blocks.
 *
 * The code knows that XFS_DIR2_DATA_FD_COUNT is 3.
 */
typedef struct xfs_dir2_data_hdr {
    __be32 magic; /* XFS_DIR2_DATA_MAGIC or XFS_DIR2_BLOCK_MAGIC */
    xfs_dir2_data_free_t bestfree[XFS_DIR2_DATA_FD_COUNT];
} xfs_dir2_data_hdr_t;

/*
 * define a structure for all the verification fields we are adding to the
 * directory block structures. This will be used in several structures.
 * The magic number must be the first entry to align with all the dir2
 * structures so we determine how to decode them just by the magic number.
 */
struct xfs_dir3_blk_hdr {
    __be32 magic;    /* magic number */
    __be32 crc;      /* CRC of block */
    __beu64 blkno;    /* first block of the buffer */
    __beu64 lsn;      /* sequence number of last write */
    xfs_uuid_t uuid; /* filesystem we belong to */
    __beu64 owner;    /* inode that owns the block */
};

struct xfs_dir3_data_hdr {
    struct xfs_dir3_blk_hdr hdr;
    xfs_dir2_data_free_t best_free[XFS_DIR2_DATA_FD_COUNT];
    __be32 pad; /* 64 bit alignment */
};

/*
 * Active entry in a data block.
 *
 * Aligned to 8 bytes.  After the variable length name field there is a
 * 2 byte tag field, which can be accessed using xfs_dir2_data_entry_tag_p.
 */
typedef struct xfs_dir2_data_entry {
    __beu64 inumber;            /* inode number */
    __u8 namelen;              /* name length */
    __u8 name[0];              /* name bytes, no NULL */
    /* __be16          tag; */ /* starting offset of us */
} xfs_dir2_data_entry_t;

typedef struct xfs_dir2_entry_inf_tag {
    __be16 tag; /* starting offset of us */
} xfs_dir2_entry_inf_tag_t;

/*
 * Unused entry in a data block.
 *
 * Aligned to 8 bytes.  Tag appears as the last 2 bytes and must be accessed
 * using xfs_dir2_data_unused_tag_p.
 */
typedef struct xfs_dir2_data_unused {
    __be16 freetag; /* XFS_DIR2_DATA_FREE_TAG */
    __be16 length;  /* total free length */
                    /* variable offset */
    __be16 tag;     /* starting offset of us */
} xfs_dir2_data_unused_t;

/*
 * Leaf block structures.
 *
 * A pure leaf block looks like the following drawing on disk:
 *
 * +---------------------------+
 * | xfs_dir2_leaf_hdr_t       |
 * +---------------------------+
 * | xfs_dir2_leaf_entry_t     |
 * | xfs_dir2_leaf_entry_t     |
 * | xfs_dir2_leaf_entry_t     |
 * | xfs_dir2_leaf_entry_t     |
 * | ...                       |
 * +---------------------------+
 * | xfs_dir2_data_off_t       |
 * | xfs_dir2_data_off_t       |
 * | xfs_dir2_data_off_t       |
 * | ...                       |
 * +---------------------------+
 * | xfs_dir2_leaf_tail_t      |
 * +---------------------------+
 *
 * The xfs_dir2_data_off_t members (bests) and tail are at the end of the block
 * for single-leaf (magic = XFS_DIR2_LEAF1_MAGIC) blocks only, but not present
 * for directories with separate leaf nodes and free space blocks
 * (magic = XFS_DIR2_LEAFN_MAGIC).
 *
 * As all the entries are variable size structures the accessors below should
 * be used to iterate over them.
 */
/*
 * Leaf block header.
 */
typedef struct xfs_dir2_leaf_hdr {
    xfs_da_blkinfo_t info; /* header for da routines */
    __be16 count;          /* count of entries */
    __be16 stale;          /* count of stale entries */
} xfs_dir2_leaf_hdr_t;

struct xfs_dir3_leaf_hdr {
    struct xfs_da3_blkinfo info; /* header for da routines */
    __be16 count;                /* count of entries */
    __be16 stale;                /* count of stale entries */
    __be32 pad;                  /* 64 bit alignment */
};

struct xfs_dir3_icleaf_hdr {
    __uint32_t forw;
    __uint32_t back;
    __uint16_t magic;
    __uint16_t count;
    __uint16_t stale;
};

/*
 * Leaf block entry.
 */
typedef struct xfs_dir2_leaf_entry {
    __be32 hashval; /* hash value of name */
    __be32 address; /* address of data entry */
} xfs_dir2_leaf_entry_t;

/*
 * Leaf block tail.
 */
typedef struct xfs_dir2_leaf_tail {
    __be32 bestcount;
} xfs_dir2_leaf_tail_t;

/*
 * Leaf block.
 */
typedef struct xfs_dir2_leaf {
    xfs_dir2_leaf_hdr_t hdr;         /* leaf header */
    xfs_dir2_leaf_entry_t __ents[0]; /* entries */
} xfs_dir2_leaf_t;

struct xfs_dir3_leaf {
    struct xfs_dir3_leaf_hdr hdr;         /* leaf header */
    struct xfs_dir2_leaf_entry __ents[0]; /* entries */
};

typedef struct xfs_dir2_free_hdr {
    __be32 magic;   /* XFS_DIR2_FREE_MAGIC */
    __be32 firstdb; /* db of first entry */
    __be32 nvalid;  /* count of valid entries */
    __be32 nused;   /* count of used entries */
} xfs_dir2_free_hdr_t;

typedef struct xfs_dir2_free {
    xfs_dir2_free_hdr_t hdr; /* block header */
    __be16 bests[0];         /* best free counts */
                             /* unused entries are -1 */
} xfs_dir2_free_t;

struct xfs_dir3_free_hdr {
    struct xfs_dir3_blk_hdr hdr;
    __be32 firstdb; /* db of first entry */
    __be32 nvalid;  /* count of valid entries */
    __be32 nused;   /* count of used entries */
    __be32 pad;     /* 64 bit alignment */
};

struct xfs_dir3_free {
    struct xfs_dir3_free_hdr hdr;
    __be16 bests[0]; /* best free counts */
                     /* unused entries are -1 */
};

/*
 * In core version of the free block header, abstracted away from on-disk format
 * differences. Use this in the code, and convert to/from the disk version using
 * xfs_dir3_free_hdr_from_disk/xfs_dir3_free_hdr_to_disk.
 */
struct xfs_dir3_icfree_hdr {
    __uint32_t magic;
    __uint32_t firstdb;
    __uint32_t nvalid;
    __uint32_t nused;
};

/*
 * Single block format.
 *
 * The single block format looks like the following drawing on disk:
 *
 * +-------------------------------------------------+
 * | xfs_dir2_data_hdr_t                             |
 * +-------------------------------------------------+
 * | xfs_dir2_data_entry_t OR xfs_dir2_data_unused_t |
 * | xfs_dir2_data_entry_t OR xfs_dir2_data_unused_t |
 * | xfs_dir2_data_entry_t OR xfs_dir2_data_unused_t :
 * | ...                                             |
 * +-------------------------------------------------+
 * | unused space                                    |
 * +-------------------------------------------------+
 * | ...                                             |
 * | xfs_dir2_leaf_entry_t                           |
 * | xfs_dir2_leaf_entry_t                           |
 * +-------------------------------------------------+
 * | xfs_dir2_block_tail_t                           |
 * +-------------------------------------------------+
 *
 * As all the entries are variable size structures the accessors below should
 * be used to iterate over them.
 */
typedef struct xfs_dir2_block_tail {
    __be32 count; /* count of leaf entries */
    __be32 stale; /* count of stale lf entries */
} xfs_dir2_block_tail_t;

/*
 * Generic single-block structure, for xfs_db.
 */
typedef union {
    xfs_dir2_data_entry_t entry;
    xfs_dir2_data_unused_t unused;
} xfs_dir2_data_union_t;

typedef struct xfs_dir2_block {
    xfs_dir2_data_hdr_t hdr; /* magic XFS_DIR2_BLOCK_MAGIC */
    xfs_dir2_data_union_t u[1];
    xfs_dir2_leaf_entry_t leaf[1];
    xfs_dir2_block_tail_t tail;
} xfs_dir2_block_t;

struct xfs_attr_sf_hdr { /* constant-structure header block */
    __be16 totsize;      /* total bytes in shortform list */
    __u8 count;          /* count of active entries */
};

struct xfs_attr_sf_entry {
    __uint8_t namelen;    /* actual length of name (no NULL) */
    __uint8_t valuelen;   /* actual length of value (no NULL) */
    __uint8_t flags;      /* flags bits (see xfs_attr_leaf.h) */
    __uint8_t nameval[1]; /* name & value bytes concatenated */
};                        /* variable sized array */

typedef struct xfs_attr_sf_hdr xfs_attr_sf_hdr_t;
typedef struct xfs_attr_sf_entry xfs_attr_sf_entry_t;

typedef struct xfs_attr_shortform {
    xfs_attr_sf_hdr_t hdr;
    xfs_attr_sf_entry_t list[1]; /* variable sized array */
} xfs_attr_shortform_t;

// 磁盘extents压缩数据，结构
struct my_rec {
    __uint64_t extentflag;
    __uint64_t startoff;
    union {
        struct {
            __uint64_t startblock2 : 43;
            __uint64_t startblock1 : 9;
            __uint64_t : 12; // 预留的，但是必须初始化
        } u;
        __uint64_t startblock;
    } uname;

    __uint64_t blockcount;
};

struct my_rec_use_read {
    union {
        struct {
            __uint64_t startblock1 : 9;
            __uint64_t startoff : 54;
            __uint64_t extentflag : 1; // 切记，这个是第一位
        } u1;
        __uint64_t x1; // 用于转换字节序，避免来回指针转换
    } my_union1;

    union {
        struct {
            __uint64_t blockcount : 21;
            __uint64_t startblock2 : 43;
        } u2;
        __uint64_t x2;
    } my_union2;
};

typedef struct my_info_use {
    __uint32_t sb_magicnum;  /* magic number == XFS_SB_MAGIC */
    __uint32_t sb_blocksize; /* logical block size, bytes */
    __uint64_t sb_dblocks;   /* number of data blocks */
    __uint32_t sb_agblocks;  /* size of an allocation group */
    __uint32_t sb_agcount;   /* number of allocation groups */
    __uint16_t sb_sectsize;
    __uint32_t crc_flag;     /* superblock crc */
    __uint32_t agf_magicnum; /* magic number == XFS_AGF_MAGIC */
    __uint32_t agf_seqno;    /* sequence # starting from 0 */
    __uint32_t abtb_root;    /* root blocks */
    __uint32_t abtc_root;    /* root blocks */
    __uint32_t rootnum;
    __uint32_t abtb_magic;   /* magic number for block type */
    __uint16_t abtb_depth;   /* 0 is a leaf */
    __uint16_t abtb_numrecs; /* current # of data records */
} my_info_use_t;

struct headmsg {
    __uint8_t msg[2];
};

struct head_sdb_msg {
    __uint8_t msg[4];
};

class xfsHandler : public filesystemHandler {
public:
    xfsHandler()
    {
        setObjType(OBJ_TYPE_FILESYSTEM);
        setMagic("xfs");
        setType((int32_t)AFS_FILESYSTEM_XFS);

        m_inode_8bit_flag = 0; // inode是否占8字节存储
        m_filetype_flag = 0;   // 文件类型
        m_inopblog = 0;
        m_inodelog = 0;
        m_agblklog = 0;
        m_agblocks = 0;
        m_agcount = 0;
        m_blkbb_log = 0;
        m_agino_log = 0;
        m_dir_inode = 0;
        m_tag_inode = 0;
        m_block_number = 0;
        m_target_dir_flag = 0;
        m_flag = 0;
        m_crc = 0;
        m_rootino = 0;
        m_inopblock = 0;
        m_blocksize = 0;
        m_inodesize = 0;
        m_offset_hdr = 0;
        m_RootPos = 0;
        m_offset_tree = 0;
        m_tag = 0;
        m_prt_off = 0;
        m_has_filetype = 0;
        m_found_target_file = 0;
    }

    ~xfsHandler() {}

    int getBitmap(vector<BitMap *> &bitmap_vect);
    int getFile(const char *file_path, vector<BitMap *> &bitmap_vect);

    static afsObject *CreateObject()
    {
        return new xfsHandler();
    }

private:
    uint8_t m_inode_8bit_flag; // inode是否占8字节存储
    uint8_t m_filetype_flag;
    uint8_t m_agino_log;
    uint8_t m_blkbb_log;
    uint8_t m_target_dir_flag;
    uint8_t m_flag;
    uint8_t m_inopblog;
    uint8_t m_agblklog;
    uint8_t m_inodelog;
    uint8_t m_found_target_file;

    uint16_t m_inopblock;
    uint16_t m_inodesize;

    uint32_t m_agcount;
    uint64_t m_agblocks;
    uint32_t m_crc;
    uint32_t m_blocksize;
    uint32_t m_has_filetype;

    uint64_t m_RootPos;
    uint64_t m_offset_hdr;
    uint64_t m_offset_tree;
    uint64_t m_tag;
    uint64_t m_dir_inode;
    uint64_t m_tag_inode;
    uint64_t m_rootino;
    uint64_t m_block_number;
    uint64_t m_prt_off;

    queue<uint64_t> m_que_xd2d_file_type;
    queue<uint64_t> m_que_tree_blknum;
    queue<uint64_t> m_que_target_inode;
    queue<uint64_t> m_que_save_inode;

    queue<uint64_t> m_que_bmap_sta;
    queue<uint64_t> m_que_bmap_cnt;

    queue<uint64_t> m_que_block_sta;
    queue<uint64_t> m_que_block_cnt;

    queue<uint64_t> m_que_ret_bm_msg_pos;
    queue<uint64_t> m_que_ret_bm_msg_len;

    queue<string> m_file_path_arr;

    void xfs_splitPath(string &str);

    uint16_t xfs_byteDirInfo_16b(uint8_t *i);
    uint32_t xfs_byteDirInfo_32b(uint8_t *i);
    uint64_t xfs_byteDirInfo_64b(uint8_t *i);
    uint64_t xfs_calcInodeValue(xfs_dir2_inou_t inode_info);
    // 检查文件类型
    int32_t xfs_isFileType(imgReader *img_reader, uint64_t offset_ret);
    void xfs_freeQueue();
    uint64_t xfs_getOffset(xfs_ino_t ino, uint8_t);
    uint64_t xfs_getFilenameLen(uint64_t i8count, uint8_t i8count_p);
    int32_t xfs_msgBitmap(imgReader *img_reader, uint64_t offset_read);
    int32_t xfs_msgBitmapExtents(imgReader *img_reader, xfs_dinode_t *root_inode, uint8_t *target_inode);
    int32_t xfs_msgBitmapExtents_if(imgReader *img_reader, uint64_t &offset_blk, xfs_bmbt_irec_t *ret_irc,
        head_sdb_msg *sdb_msg);
    int32_t xfs_msgBitmapExtentswhile(imgReader *img_reader);
    int32_t xfs_analyzeData(my_rec_use_read *rec_read, xfs_bmbt_irec_t *m_bmbt_irec);
    int32_t xfs_analyzeBlockDir(imgReader *img_reader, uint32_t start_blk, uint64_t count_blk);
    int32_t xfs_analyzeBlockDirbestfree(imgReader *img_reader, uint8_t *block_msg, uint32_t &file_postion,
        uint16_t &single_block_dir_offset);
    int32_t xfs_readBlockInfo(imgReader *img_reader, uint64_t offset_ret);
    int32_t xfs_readBlockInfo_crc(imgReader *img_reader, uint8_t *block_msg, uint16_t &blk_num, uint32_t &file_postion,
        uint16_t &tree_level);
    int32_t xfs_readInodeInfo(imgReader *img_reader, uint64_t offset_read);
    int32_t xfs_analyzeBlockDirWhile(imgReader *img_reader, uint32_t &file_postion, uint8_t *block_msg);
    int32_t xfs_analyzeBlockDirWhilestr(imgReader *img_reader, char *str_cp, char *str,
        xfs_dir2_data_entry_t *xd2d_dir_info);
    int32_t xfs_readBlockInfoLeaf(imgReader *img_reader, uint64_t offset_ret, uint16_t blk_num, uint32_t file_postion,
        uint8_t *block_msg);
    int32_t xfs_readBlockInfoMid(imgReader *reader, uint64_t offset_ret, uint16_t blk_num, uint8_t *block_msg);
    int32_t xfs_readInodeInfoBtree(imgReader *img_reader, uint64_t offset_ret, uint8_t *root);
    int32_t xfs_readInodeInfoBtreelevel(imgReader *img_reader, uint8_t *root, uint16_t numrecs);
    int32_t xfs_readInodeInfoLocal(imgReader *reader, uint64_t file_postion, uint8_t i8count_p, uint8_t *root,
        xfs_dir2_sf_hdr_t *head_info);
    int32_t xfs_readInodeInfoLocalIf(char *str, char *str_cp, xfs_dir2_info_t *xfs_file_type,
        xfs_dir2_info_f_t *xfs_no_file_type);
    int32_t xfs_readInodeInfoLocalif_1(char *str, xfs_dir2_info_t *xfs_file_type, xfs_dir2_info_f_t *xfs_no_file_type);

    // Bitmap转换
    int32_t xfs_getBitmapBitmapConvert(BitMap *bitmap, uint64_t position, uint64_t len);
    int32_t xfs_getBitmapByReadBlkInfo_depth(imgReader *img_reader, uint32_t &ptr_position, uint8_t *btree_blk_info,
        xfs_alloc_key_t *recs, my_info_use_t &getB_info, BitMap *bitmap);
    int32_t xfs_getBitmapByReadBlkInfo_1(imgReader *img_reader, my_info_use_t &getB_info, uint32_t &ptr_position,
        xfs_alloc_key_t **recs, uint8_t *btree_blk_info);
    int32_t xfs_getBitmapByReadBlkInfo(imgReader *img_reader, my_info_use_t &getB_info, BitMap *bitmap);
    int32_t xfs_getBitmapByReadAGFInfo(imgReader *img_reader, uint64_t offset, my_info_use_t &getB_info);
    int32_t xfs_getBitmapByReadSBInfo(imgReader *img_reader, my_info_use_t &getB_info);

    int32_t getFileParseSB();
    int32_t getFileFiltFile(vector<BitMap *> &bitmap_vect);
    int32_t getFileFiltFile_1(imgReader *reader);
    int32_t xfs_readInodeInfoDoMode(uint8_t *root, xfs_dinode_t *root_inode, uint64_t offset_ret);
    int32_t xfs_readInodeInfoDoModeExt(uint8_t *root, xfs_dinode_t *root_inode);
    int32_t xfs_readInodeInfoLocalIfLoopDir(const char *file_path);
    int32_t xfs_readInodeInfoLocalDoFile0(uint8_t *root, uint64_t &file_postion);
    int32_t xfs_riilDoFile(xfs_dir2_sf_entry_t *xfs_file_info, xfs_dir2_info_t *xfs_file_type,
        xfs_dir2_info_f_t *xfs_no_file_type);
    uint32_t xfs_sb_version_hasftype(struct xfs_dsb *sbp);
    uint16_t afs_bswap_16(uint16_t x);
    uint32_t afs_bswap_32(uint32_t x);
    uint64_t afs_bswap_64(uint64_t x);
};

#endif // XFS_H
