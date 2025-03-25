/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file Ext4FS.h
 * @brief Afs - Analyze EXT file system.
 *
 */

#ifndef EXT4FS_H
#define EXT4FS_H

#include <vector>
#include <queue>
#include "afs/Ext4Hash.h"
#include "afs/FileSystem.h"
#include "afs/AfsObject.h"
#include "afs/ImgReader.h"
#include "afs/Bitmap.h"
#include "afs/LogMsg.h"

const int BASE_BLOCK_SIZE_BITS = 10;
const int MAX_DIR_NAME = 4096;
const int MAX_EE_LEN = 32768;
const int SECTOR_SIZE_BITS = 9;
const int I_BLOCK_SIZE = 60;
const int ROOT_INODE_NUM = 2;
const int THE_BLOCK_MAP_END_FLAG = 318;

#define EXT4_NDIR_BLOCKS 12
#define EXT4_IND_BLOCK EXT4_NDIR_BLOCKS
#define EXT4_DIND_BLOCK (EXT4_IND_BLOCK + 1)
#define EXT4_TIND_BLOCK (EXT4_DIND_BLOCK + 1)
#define EXT4_N_BLOCKS (EXT4_TIND_BLOCK + 1)

#define EXT3_4_FEATURE_COMPAT 0x0000003C

/*
 * Structure of the super block
 */
struct ext4_super_block {
    /* 00 */ le32 s_inodes_count;      /* Inodes count */
    le32 s_blocks_count_lo;            /* Blocks count */
    le32 s_r_blocks_count_lo;          /* Reserved blocks count */
    le32 s_free_blocks_count_lo;       /* Free blocks count */
    /* 10 */ le32 s_free_inodes_count; /* Free inodes count */
    le32 s_first_data_block;           /* First Data Block */
    le32 s_log_block_size;             /* Block size */
    le32 s_log_cluster_size;           /* Allocation cluster size */
    /* 20 */ le32 s_blocks_per_group;  /* # Blocks per group */
    le32 s_clusters_per_group;         /* # Clusters per group */
    le32 s_inodes_per_group;           /* # Inodes per group */
    le32 s_mtime;                      /* Mount time */
    /* 30 */ le32 s_wtime;             /* Write time */
    le16 s_mnt_count;                  /* Mount count */
    le16 s_max_mnt_count;              /* Maximal mount count */
    le16 s_magic;                      /* Magic signature */
    le16 s_state;                      /* File system state */
    le16 s_errors;                     /* Behaviour when detecting errors */
    le16 s_minor_rev_level;            /* minor revision level */
    /* 40 */ le32 s_lastcheck;         /* time of last check */
    le32 s_checkinterval;              /* max. time between checks */
    le32 s_creator_os;                 /* OS */
    le32 s_rev_level;                  /* Revision level */
    /* 50 */ le16 s_def_resuid;        /* Default uid for reserved blocks */
    le16 s_def_resgid;                 /* Default gid for reserved blocks */
    /*
     * These fields are for EXT4_DYNAMIC_REV superblocks only.
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
    le32 s_first_ino;                       /* First non-reserved inode */
    le16 s_inode_size;                      /* size of inode structure */
    le16 s_block_group_nr;                  /* block group # of this superblock */
    le32 s_feature_compat;                  /* compatible feature set */
    /* 60 */ le32 s_feature_incompat;       /* incompatible feature set */
    le32 s_feature_ro_compat;               /* readonly-compatible feature set */
    /* 68 */ u8 s_uuid[16];                 /* 128-bit uuid for volume */
    /* 78 */ char s_volume_name[16];        /* volume name */
    /* 88 */ char s_last_mounted[64];       /* directory where last mounted */
    /* C8 */ le32 s_algorithm_usage_bitmap; /* For compression */
    /*
     * Performance hints.  Directory preallocation should only
     * happen if the EXT4_FEATURE_COMPAT_DIR_PREALLOC flag is on.
     */
    u8 s_prealloc_blocks;           /* Nr of blocks to try to preallocate */
    u8 s_prealloc_dir_blocks;       /* Nr to preallocate for dirs */
    le16 s_reserved_gdt_blocks;     /* Per group desc for online growth */
                                    /*
                                     * Journaling support valid if EXT4_FEATURE_COMPAT_HAS_JOURNAL set.
                                     */
    /* D0 */ u8 s_journal_uuid[16]; /* uuid of journal superblock */
    /* E0 */ le32 s_journal_inum;   /* inode number of journal file */
    le32 s_journal_dev;             /* device number of journal file */
    le32 s_last_orphan;             /* start of list of inodes to delete */
    le32 s_hash_seed[4];            /* HTREE hash seed */
    u8 s_def_hash_version;          /* Default hash version to use */
    u8 s_jnl_backup_type;
    le16 s_desc_size; /* size of group descriptor */
    /* 100 */ le32 s_default_mount_opts;
    le32 s_first_meta_bg;             /* First metablock block group */
    le32 s_mkfs_time;                 /* When the filesystem was created */
    le32 s_jnl_blocks[17];            /* Backup of the journal inode */
                                      /* 64bit support valid if EXT4_FEATURE_COMPAT_64BIT */
    /* 150 */ le32 s_blocks_count_hi; /* Blocks count */
    le32 s_r_blocks_count_hi;         /* Reserved blocks count */
    le32 s_free_blocks_count_hi;      /* Free blocks count */
    le16 s_min_extra_isize;           /* All inodes have at least # bytes */
    le16 s_want_extra_isize;          /* New inodes should reserve # bytes */
    le32 s_flags;                     /* Miscellaneous flags */
    le16 s_raid_stride;               /* RAID stride */
    le16 s_mmp_update_interval;       /* # seconds to wait in MMP checking */
    le64 s_mmp_block;                 /* Block for multi-mount protection */
    le32 s_raid_stripe_width;         /* blocks on all data disks (N*stride) */
    u8 s_log_groups_per_flex;         /* FLEX_BG group size */
    u8 s_checksum_type;               /* metadata checksum algorithm used */
    u8 s_encryption_level;            /* versioning level for encryption */
    u8 s_reserved_pad;                /* Padding to next 32bits */
    le64 s_kbytes_written;            /* nr of lifetime kilobytes written */
    le32 s_snapshot_inum;             /* Inode number of active snapshot */
    le32 s_snapshot_id;               /* sequential ID of active snapshot */
    le64 s_snapshot_r_blocks_count;   /* reserved blocks for active
                                         snapshot's future use */
    le32 s_snapshot_list;             /* inode number of the head of the
                                         on-disk snapshot list */
    le32 s_error_count;               /* number of fs errors */
    le32 s_first_error_time;          /* first time an error happened */
    le32 s_first_error_ino;           /* inode involved in first error */
    le64 s_first_error_block;         /* block involved of first error */
    u8 s_first_error_func[32];        /* function where the error happened */
    le32 s_first_error_line;          /* line number where error happened */
    le32 s_last_error_time;           /* most recent time of an error */
    le32 s_last_error_ino;            /* inode involved in last error */
    le32 s_last_error_line;           /* line number where error happened */
    le64 s_last_error_block;          /* block involved of last error */
    u8 s_last_error_func[32];         /* function where the error happened */
    u8 s_mount_opts[64];
    le32 s_usr_quota_inum;    /* inode for tracking user quota */
    le32 s_grp_quota_inum;    /* inode for tracking group quota */
    le32 s_overhead_clusters; /* overhead blocks/clusters in fs */
    le32 s_backup_bgs[2];     /* groups with sparse_super2 SBs */
    u8 s_encrypt_algos[4];    /* Encryption algorithms in use  */
    u8 s_encrypt_pw_salt[16]; /* Salt used for string2key algorithm */
    le32 s_lpf_ino;           /* Location of the lost+found inode */
    le32 s_prj_quota_inum;    /* inode for tracking project quota */
    le32 s_checksum_seed;     /* crc32c(uuid) if csum_seed set */
    le32 s_reserved[98];      /* Padding to the end of the block */
    le32 s_checksum;          /* crc32c(superblock) */
};

/*
 * Structure of a blocks group descriptor
 */
struct ext4_group_desc {
    le32 bg_block_bitmap_lo;      /* Blocks bitmap block */
    le32 bg_inode_bitmap_lo;      /* Inodes bitmap block */
    le32 bg_inode_table_lo;       /* Inodes table block */
    le16 bg_free_blocks_count_lo; /* Free blocks count */
    le16 bg_free_inodes_count_lo; /* Free inodes count */
    le16 bg_used_dirs_count_lo;   /* Directories count */
    le16 bg_flags;                /* EXT4_BG_flags (INODE_UNINIT, etc) */
    le32 bg_exclude_bitmap_lo;    /* Exclude bitmap for snapshots */
    le16 bg_block_bitmap_csum_lo; /* crc32c(s_uuid+grp_num+bbitmap) LE */
    le16 bg_inode_bitmap_csum_lo; /* crc32c(s_uuid+grp_num+ibitmap) LE */
    le16 bg_itable_unused_lo;     /* Unused inodes count */
    le16 bg_checksum;             /* crc16(sb_uuid+group+desc) */
    le32 bg_block_bitmap_hi;      /* Blocks bitmap block MSB */
    le32 bg_inode_bitmap_hi;      /* Inodes bitmap block MSB */
    le32 bg_inode_table_hi;       /* Inodes table block MSB */
    le16 bg_free_blocks_count_hi; /* Free blocks count MSB */
    le16 bg_free_inodes_count_hi; /* Free inodes count MSB */
    le16 bg_used_dirs_count_hi;   /* Directories count MSB */
    le16 bg_itable_unused_hi;     /* Unused inodes count MSB */
    le32 bg_exclude_bitmap_hi;    /* Exclude bitmap block MSB */
    le16 bg_block_bitmap_csum_hi; /* crc32c(s_uuid+grp_num+bbitmap) BE */
    le16 bg_inode_bitmap_csum_hi; /* crc32c(s_uuid+grp_num+ibitmap) BE */
    u32 bg_reserved;
};

/*
 * Structure of an inode on the disk
 */
struct ext4_inode {
    le16 i_mode;        /* File mode */
    le16 i_uid;         /* Low 16 bits of Owner Uid */
    le32 i_size_lo;     /* Size in bytes */
    le32 i_atime;       /* Access time */
    le32 i_ctime;       /* Inode Change time */
    le32 i_mtime;       /* Modification time */
    le32 i_dtime;       /* Deletion Time */
    le16 i_gid;         /* Low 16 bits of Group Id */
    le16 i_links_count; /* Links count */
    le32 i_blocks_lo;   /* Blocks count */
    le32 i_flags;       /* File flags */
    union {
        struct {
            le32 l_i_version;
        } linux1;
        struct {
            u32 h_i_translator;
        } hurd1;
        struct {
            u32 m_i_reserved1;
        } masix1;
    } osd1;                      /* OS dependent 1 */
    le32 i_block[EXT4_N_BLOCKS]; /* Pointers to blocks */
    le32 i_generation;           /* File version (for NFS) */
    le32 i_file_acl_lo;          /* File ACL */
    le32 i_size_high;
    le32 i_obso_faddr; /* Obsoleted fragment address */
    union {
        struct {
            le16 l_i_blocks_high; /* were l_i_reserved1 */
            le16 l_i_file_acl_high;
            le16 l_i_uid_high;    /* these 2 fields */
            le16 l_i_gid_high;    /* were reserved2[0] */
            le16 l_i_checksum_lo; /* crc32c(uuid+inum+inode) LE */
            le16 l_i_reserved;
        } linux2;
        struct {
            le16 h_i_reserved1; /* Obsoleted fragment number/size which are removed in ext4 */
            u16 h_i_mode_high;
            u16 h_i_uid_high;
            u16 h_i_gid_high;
            u32 h_i_author;
        } hurd2;
        struct {
            le16 h_i_reserved1; /* Obsoleted fragment number/size which are removed in ext4 */
            le16 m_i_file_acl_high;
            u32 m_i_reserved2[2];
        } masix2;
    } osd2; /* OS dependent 2 */
    le16 i_extra_isize;
    le16 i_checksum_hi;  /* crc32c(uuid+inum+inode) BE */
    le32 i_ctime_extra;  /* extra Change time      (nsec << 2 | epoch) */
    le32 i_mtime_extra;  /* extra Modification time(nsec << 2 | epoch) */
    le32 i_atime_extra;  /* extra Access time      (nsec << 2 | epoch) */
    le32 i_crtime;       /* File Creation time */
    le32 i_crtime_extra; /* extra FileCreationtime (nsec << 2 | epoch) */
    le32 i_version_hi;   /* high 32 bits for 64-bit version */
    le32 i_projid;       /* Project ID */
};

/*
 * Structure of a directory entry
 */
#define EXT4_NAME_LEN 255

struct ext4_dir_entry {
    le32 inode;               /* Inode number */
    le16 rec_len;             /* Directory entry length */
    le16 name_len;            /* Name length */
    char name[EXT4_NAME_LEN]; /* File name */
};

/*
 * The new version of the directory entry.  Since EXT4 structures are
 * stored in intel byte order, and the name_len field could never be
 * bigger than 255 chars, it's safe to reclaim the extra byte for the
 * file_type field.
 */
struct ext4_dir_entry_2 {
    le32 inode;   /* Inode number */
    le16 rec_len; /* Directory entry length */
    u8 name_len;  /* Name length */
    u8 file_type;
    char name[EXT4_NAME_LEN]; /* File name */
};


struct fake_dirent {
    le32 inode;
    le16 rec_len;
    u8 name_len;
    u8 file_type;
};

struct dx_entry {
    le32 hash;
    le32 block;
};

struct dx_root {
    struct fake_dirent dot;
    char dot_name[4];
    struct fake_dirent dotdot;
    char dotdot_name[4];
    struct dx_root_info {
        le32 reserved_zero;
        u8 hash_version;
        u8 info_length; /* 8 */
        u8 indirect_levels;
        u8 unused_flags;
    } info;
    struct dx_entry entries[0];
};

struct dx_root_2 {
    struct fake_dirent dot;
    char dot_name[4];
    struct fake_dirent dotdot;
    char dotdot_name[4];
    struct dx_root_info_2 {
        le32 reserved_zero;
        u8 hash_version;
        u8 info_length; /* 8 */
        u8 indirect_levels;
        u8 unused_flags;
    } info;
    le16 limit;
    le16 count;
    le32 block;
};

struct dx_node {
    struct fake_dirent fake;
    struct dx_entry entries[0];
};

struct dx_node_2 {
    struct fake_dirent fake;
    le16 limit;
    le16 count;
    le32 block;
};

/*
 * This is the extent on-disk structure.
 * It's used at the bottom of the tree.
 */
struct ext4_extent {
    le32 ee_block;    /* first logical block extent covers */
    le16 ee_len;      /* number of blocks covered by extent */
    le16 ee_start_hi; /* high 16 bits of physical block */
    le32 ee_start_lo; /* low 32 bits of physical block */
};

/*
 * This is index on-disk structure.
 * It's used at all the levels except the bottom.
 */
struct ext4_extent_idx {
    le32 ei_block;   /* index covers logical blocks from 'block' */
    le32 ei_leaf_lo; /* pointer to the physical block of the next *
                      * level. leaf or next index could be there */
    le16 ei_leaf_hi; /* high 16 bits of physical block */
    u16 ei_unused;
};

/*
 * Each block (leaves and indexes), even inode-stored has header.
 */
struct ext4_extent_header {
    le16 eh_magic;      /* probably will support different formats */
    le16 eh_entries;    /* number of valid entries */
    le16 eh_max;        /* capacity of store in entries */
    le16 eh_depth;      /* has tree real underlying blocks? */
    le32 eh_generation; /* generation of the tree */
};


#define EXT4_FEATURE_INCOMPAT_COMPRESSION 0x0001
#define EXT4_FEATURE_INCOMPAT_FILETYPE 0x0002
#define EXT4_FEATURE_INCOMPAT_RECOVER 0x0004     /* Needs recovery */
#define EXT4_FEATURE_INCOMPAT_JOURNAL_DEV 0x0008 /* Journal device */
#define EXT4_FEATURE_INCOMPAT_META_BG 0x0010
#define EXT4_FEATURE_INCOMPAT_EXTENTS 0x0040 /* extents support */
#define EXT4_FEATURE_INCOMPAT_64BIT 0x0080
#define EXT4_FEATURE_INCOMPAT_MMP 0x0100
#define EXT4_FEATURE_INCOMPAT_FLEX_BG 0x0200
#define EXT4_FEATURE_INCOMPAT_EA_INODE 0x0400 /* EA in inode */
#define EXT4_FEATURE_INCOMPAT_DIRDATA 0x1000  /* data in dirent */
#define EXT4_FEATURE_INCOMPAT_CSUM_SEED 0x2000
#define EXT4_FEATURE_INCOMPAT_LARGEDIR 0x4000    /* >2GB or 3-lvl htree */
#define EXT4_FEATURE_INCOMPAT_INLINE_DATA 0x8000 /* data in inode */
#define EXT4_FEATURE_INCOMPAT_ENCRYPT 0x10000

/*
 * Inode flags
 */
#define EXT4_SECRM_FL 0x00000001     /* Secure deletion */
#define EXT4_UNRM_FL 0x00000002      /* Undelete */
#define EXT4_COMPR_FL 0x00000004     /* Compress file */
#define EXT4_SYNC_FL 0x00000008      /* Synchronous updates */
#define EXT4_IMMUTABLE_FL 0x00000010 /* Immutable file */
#define EXT4_APPEND_FL 0x00000020    /* writes to file may only append */
#define EXT4_NODUMP_FL 0x00000040    /* do not dump file */
#define EXT4_NOATIME_FL 0x00000080   /* do not update atime */
/* Reserved for compression usage... */
#define EXT4_DIRTY_FL 0x00000100
#define EXT4_COMPRBLK_FL 0x00000200 /* One or more compressed clusters */
#define EXT4_NOCOMPR_FL 0x00000400  /* Don't compress */
/* nb: was previously EXT2_ECOMPR_FL */
#define EXT4_ENCRYPT_FL 0x00000800 /* encrypted file */
/* End compression flags --- maybe not all used */
#define EXT4_INDEX_FL 0x00001000        /* hash-indexed directory */
#define EXT4_IMAGIC_FL 0x00002000       /* AFS directory */
#define EXT4_JOURNAL_DATA_FL 0x00004000 /* file data should be journaled */
#define EXT4_NOTAIL_FL 0x00008000       /* file tail should not be merged */
#define EXT4_DIRSYNC_FL 0x00010000      /* dirsync behaviour (directories only) */
#define EXT4_TOPDIR_FL 0x00020000       /* Top of directory hierarchies */
#define EXT4_HUGE_FILE_FL 0x00040000    /* Set to each huge file */
#define EXT4_EXTENTS_FL 0x00080000      /* Inode uses extents */
#define EXT4_EA_INODE_FL 0x00200000     /* Inode used for large EA */
#define EXT4_EOFBLOCKS_FL 0x00400000    /* Blocks allocated beyond EOF */
#define EXT4_INLINE_DATA_FL 0x10000000  /* Inode has inline data. */
#define EXT4_PROJINHERIT_FL 0x20000000  /* Create with parents projid */
#define EXT4_RESERVED_FL 0x80000000     /* reserved for ext4 lib */

#define EXT4_FL_USER_VISIBLE 0x004BDFFF    /* User visible flags */
#define EXT4_FL_USER_MODIFIABLE 0x004380FF /* User modifiable flags */

#define S_IFLINK_AFS 0xA000
#define S_IFDIR_AFS 0x4000

#define SECTOR_SIZE 512
#define SUPER_BLOCK_PADDING 1024
#define EXT4_MIN_BLOCK 1024
#define BITS_PER_BYTE 8

/**
 * @brief 存储已经计算，可直接使用的信息
 */
struct ext4_brief_info {
    uint32_t block_size;        // 块大小(单位：字节)(最大值：在linux内核EXT4的代码中，已经设置了block
                                // size的最小(1024)和最大值(65535))
    uint64_t block_num;         // 块数量(最大值：ext4块地址为48位)
    uint64_t flex_bg_size;      // 灵活块组大小(单位：组)，即每个灵活块组中组的数量(最大值：只能确定小于块数量)
    uint32_t need_bitmap_bytes; // bitmap所需字节数(最大值：每组的块数(内核定义为__le32)/8)
    uint32_t group_desc_size;   // 组描述符大小(单位：字节)(最大值：64)
    uint64_t group_total_num;   // 组总数(最大值：只能确定小于块数量)
    uint32_t group_offset;      // 第0组前的偏移(填充字节数)(最大值：block_size的最大值(65535)+1024)
    uint32_t inode_size;        // 索引节点大小(单位：字节)(最大值：内核定义为__le16)
    uint32_t inodes_per_group;  // 每组中索引节点数量(最大值：内核定义为__le32)
    uint32_t feature_incompat;  // 不兼容功能的启用标志(最大值：内核定义为__le32)
    le32 hash_seed[4];          // hash种子
    char last_mounted[64];      // 系统最后一次挂载的目录
    bool is_filetype;           // 索引节点中是否保存了文件类型
};

/**
 * @brief 处理ext4文件系统
 */
class ext4Handler : public filesystemHandler {
public:
    ext4Handler()
    {
        setObjType(OBJ_TYPE_FILESYSTEM);
        setMagic("ext4");
        setType((int32_t)AFS_FILESYSTEM_EXT4);
    }
    ~ext4Handler() {}
    int getBitmap(vector<BitMap *> &bitmap_vect);
    int getFile(const char *file_path, vector<BitMap *> &bitmap_vect);
    static afsObject *CreateObject()
    {
        return new ext4Handler();
    }
    queue<uint64_t> extent_tree;

#ifdef CPPUNIT_MAIN
protected:
#else
private:
#endif
    char *ext_strnCopy(char *dest, size_t destMax, const char *src, size_t n);
    void ext_split(string &str, const string &pattern, vector<string> &resVec);

    int32_t ext_analyzeFileDir(imgReader *img_reader, struct ext4_brief_info &brief_info, vector<BitMap *> &bitmap_vect,
        unsigned char *group, const char *file_path);
    int32_t ext_analyzeFileDir_realAnalyze(imgReader *img_reader, struct ext4_brief_info &brief_info,
        vector<BitMap *> &bitmap_vect, struct ext4_inode *inode, unsigned char *group, vector<string> &filePath,
        int32_t &current_inode_num);
    int32_t ext_analyzeSuperBlock(imgReader *img_reader, struct ext4_brief_info &brief_info);
    int32_t ext_findFileByInode(imgReader *img_reader, struct ext4_brief_info &brief_info,
        vector<BitMap *> &bitmap_vect, unsigned char *group, struct ext4_inode *inode);
    int32_t ext_findInodeByDir(imgReader *img_reader, struct ext4_brief_info &brief_info, vector<BitMap *> &bitmap_vect,
        unsigned char *group, struct ext4_inode *inode, char *current_dir, bool find_all);
    int32_t ext_findInodeByDX(imgReader *img_reader, struct ext4_brief_info &brief_info, vector<BitMap *> &bitmap_vect,
        unsigned char *group, vector<uint64_t> &blocks, char *current_dir, bool find_all);
    int32_t ext_findInodeByDX_prepare(struct ext4_brief_info &brief_info, vector<uint64_t> &blocks, char *current_dir,
        bool find_all, struct dx_root_2 *dxroot, struct dx_hash_info &hinfo, vector<uint64_t> &leaf_blocks,
        uint64_t &real_leaf_block);
    int32_t ext_findInodeByDX_realFind(uint32_t &block_size, vector<uint64_t> &blocks, struct dx_hash_info &hinfo,
        vector<uint64_t> &leaf_blocks, uint64_t &real_leaf_block, struct dx_node_2 *dnode, uint32_t &level);
    int32_t ext_findInodeByDX_realFindAll(uint32_t &block_size, vector<uint64_t> &blocks, vector<uint64_t> &leaf_blocks,
        struct dx_node_2 *dnode, uint32_t &level);
    int32_t ext_findInodeByLinear(imgReader *img_reader, struct ext4_brief_info &brief_info,
        vector<BitMap *> &bitmap_vect, unsigned char *group, vector<uint64_t> &blocks, char *current_dir,
        bool find_all);
    int32_t ext_getInode(imgReader *img_reader, struct ext4_brief_info &brief_info, unsigned char *group,
        struct ext4_inode *inode, int32_t current_inode_num);
    int32_t ext_getExtentLeafInode(imgReader *img_reader, struct ext4_inode *inode, uint32_t block_size,
        vector<uint64_t> &blocks);
    void ext_getExtentInode_MidOrLeaf(struct ext4_extent_header *extent_header, vector<uint64_t> &blocks);
    int32_t ext_getLeafInode(imgReader *img_reader, struct ext4_inode *inode, uint32_t temp_block_size,
        vector<uint64_t> &blocks);
    int32_t ext_blockMap(imgReader *img_reader, uint32_t temp_block_size, vector<uint64_t> &blocks, uint64_t block_no,
        int32_t depth);
    void ext_find_all(imgReader *img_reader, struct ext4_brief_info &brief_info, unsigned char *group,
        struct ext4_inode *inode, struct ext4_dir_entry_2 *dir_entry, char *valid_dir, vector<BitMap *> &bitmap_vect);
    int32_t ext_findInodeByLinear_realGet(imgReader *img_reader, struct ext4_brief_info &brief_info,
        vector<BitMap *> &bitmap_vect, unsigned char *group, vector<uint64_t> &blocks, char *current_dir, bool find_all,
        struct ext4_inode *inode, unsigned char *buf, char *valid_dir, size_t valid_dir_len);
    int32_t ext_getBitmap_realGet(vector<BitMap *> &bitmap_vect, BitMap &fsbitmap, struct ext4_brief_info &brief_info,
        unsigned char *group);
    int32_t ext_getBitmap_alterBitmap(vector<BitMap *> &bitmap_vect, BitMap &fsbitmap,
        struct ext4_brief_info &brief_info);
    int32_t ext_getFile_realGet(const char *file_path, vector<BitMap *> &bitmap_vect, imgReader *img_reader,
        struct ext4_brief_info &brief_info, unsigned char *group, struct ext4_inode *inode);
    int32_t ext_findFileByInode_writeBitmap(imgReader *img_reader, struct ext4_brief_info &brief_info,
        vector<BitMap *> &bitmap_vect, unsigned char *group, struct ext4_inode *inode);
    int32_t ext_setFilterBitmap(imgReader *img_reader, uint32_t block_size, vector<uint64_t> &blocks,
        vector<BitMap *> &bitmap_vect);
};

int64_t ext_getLongValue(int feature_incompat, int gd_size, int high_bits, int low_bits);

/**
 * @brief 在64位模式下，合并数据的高32位与低32位
 *
 * @param feature_incompat int 不兼容功能标志（指示了是否开启64位功能）
 * @param gd_size int 组描述符大小（有时需组描述符大小为64才能合并）
 * @param low_bits int 数据的低32位
 * @param high_bits int 数据的高32位
 * @return int64_t 合并后的64位数据
 *
 */
inline int64_t ext_getLongValue(int feature_incompat, int gd_size, int high_bits, int low_bits)
{
    AFS_LARGEINTEGER temp;
    CHECK_MEMSET_S_OK(&temp, sizeof(AFS_LARGEINTEGER), 0, sizeof(AFS_LARGEINTEGER));

    if ((feature_incompat & EXT4_FEATURE_INCOMPAT_64BIT) != 0 && (gd_size == 64)) {
        temp.u.HighPart = high_bits;
        temp.u.LowPart = (uint32_t)low_bits;
    } else {
        temp.u.HighPart = 0;
        temp.u.LowPart = (uint32_t)low_bits;
    }

    return temp.QuadPart;
}

#endif // EXT4FS_H
