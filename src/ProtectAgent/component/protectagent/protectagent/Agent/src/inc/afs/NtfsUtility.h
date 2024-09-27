/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file NtfsUtility.h
 * @brief Analyze NTFS file system.
 *
 */

#ifndef NTFS_UTILITY_H_INCLUDED
#define NTFS_UTILITY_H_INCLUDED

#include <queue>

#include "afs/NtfsCommon.h"
#include "afs/Bitmap.h"
#include "securec.h"

#define __force

#define __constant_cpu_to_le16(x) ((le16)(x))
#define __constant_cpu_to_le32(x) ((le32)(x))
#define __constant_cpu_to_le64(x) ((le64)(x))

/*
 * Constant endianness conversion defines.
 */
#define const_cpu_to_le16(x) (le16)__constant_cpu_to_le16((u16)(x))
#define const_cpu_to_le32(x) (le32)__constant_cpu_to_le32((u32)(x))
#define const_cpu_to_le64(x) (le64)__constant_cpu_to_le64((u64)(x))

typedef u64 MFT_REF;
typedef u16 ntfschar; // 2-byte Unicode character type

const uint8_t NTFS_MAJOR_VER = 3; // 可支持的NTFS主版本号
const uint8_t NTFS_MINOR_VER = 1; // 可支持的NTFS次版本号

const uint8_t NTFS_RUNLIST_LENGTH = 50; // runlist的长度
const uint32_t NTFS_BLOCK_SIZE = 512;   // NTFS 数据块大小
const uint8_t NTFS_BLOCK_SIZE_BITS = 9;
const uint32_t NTFS_PROCESS_BYTE = 131072; // 每次读写磁盘的最大字节数(128K)
const uint32_t NTFS_MAX_PATH = 1024;
const uint32_t NTFS_MAX_NAME_LEN = 128;

const std::string NTFS_REPARSE_VOLUME = "Volume{";

#define MFT_REF_MASK_CPU 0x0000ffffffffffffULL
#define MREF(x) ((u64)((x)&MFT_REF_MASK_CPU))

const uint32_t TAG_SYMLINKD_OFFSET = 4;
const uint32_t TAG_JUNCTION_OFFSET = 8;

typedef std::map<uint64_t, uint32_t> mft_map;

#pragma pack(push) //
#pragma pack(1) //

/**
 * enum NTFS_SYSTEM_FILES - System files mft record numbers.
 *
 * All these files are always marked as used in the bitmap attribute of the
 * mft; presumably in order to avoid accidental allocation for random other
 * mft records. Also, the sequence number for each of the system files is
 * always equal to their mft record number and it is never modified.
 */
typedef enum {
    FILE_MFT = 0,         /* Master file table (mft). Data attribute
                    contains the entries and bitmap attribute
                    records which ones are in use (bit==1). */
    FILE_MFTMirr = 1,     /* Mft mirror: copy of first four mft records
                    in data attribute. If cluster size > 4kiB,
                    copy of first N mft records, with
                     N = cluster_size / mft_record_size. */
    FILE_LogFile = 2,     /* Journalling log in data attribute. */
    FILE_Volume = 3,      /* Volume name attribute and volume information
                    attribute (flags and ntfs version). Windows
                    refers to this file as volume DASD (Direct
                    Access Storage Device). */
    FILE_AttrDef = 4,     /* Array of attribute definitions in data
                    attribute. */
    FILE_root = 5,        /* Root directory. */
    FILE_Bitmap = 6,      /* Allocation bitmap of all clusters (lcns) in
                    data attribute. */
    FILE_Boot = 7,        /* Boot sector (always at cluster 0) in data
                    attribute. */
    FILE_BadClus = 8,     /* Contains all bad clusters in the non-resident
                    data attribute. */
    FILE_Secure = 9,      /* Shared security descriptors in data attribute
                    and two indexes into the descriptors.
                    Appeared in Windows 2000. Before that, this
                    file was named $Quota but was unused. */
    FILE_UpCase = 10,     /* Uppercase equivalents of all 65536 Unicode
                   characters in data attribute. */
    FILE_Extend = 11,     /* Directory containing other system files (eg.
                   $ObjId, $Quota, $Reparse and $UsnJrnl). This
                   is new to NTFS3.0. */
    FILE_reserved12 = 12, /* Reserved for future use (records 12-15). */
    FILE_reserved13 = 13,
    FILE_reserved14 = 14,
    FILE_reserved15 = 15,
    FILE_first_user = 16, /* First user file, used as test limit for
             whether to allow opening a file or not. */
} ntfs_system_files;

/**
 * enum ATTR_TYPES - System defined attributes (32-bit).
 *
 * Each attribute type has a corresponding attribute name (Unicode string of
 * maximum 64 character length) as described by the attribute definitions
 * present in the data attribute of the $AttrDef system file.
 *
 * On NTFS 3.0 volumes the names are just as the types are named in the below
 * enum exchanging AT_ for the dollar sign ($). If that isn't a revealing
 * choice of symbol... (-;
 */
typedef enum {
    AT_UNUSED = const_cpu_to_le32(0),
    AT_STANDARD_INFORMATION = const_cpu_to_le32(0x10),
    AT_ATTRIBUTE_LIST = const_cpu_to_le32(0x20),
    AT_FILE_NAME = const_cpu_to_le32(0x30),
    AT_OBJECT_ID = const_cpu_to_le32(0x40),
    AT_SECURITY_DESCRIPTOR = const_cpu_to_le32(0x50),
    AT_VOLUME_NAME = const_cpu_to_le32(0x60),
    AT_VOLUME_INFORMATION = const_cpu_to_le32(0x70),
    AT_DATA = const_cpu_to_le32(0x80),
    AT_INDEX_ROOT = const_cpu_to_le32(0x90),
    AT_INDEX_ALLOCATION = const_cpu_to_le32(0xa0),
    AT_BITMAP = const_cpu_to_le32(0xb0),
    AT_REPARSE_POINT = const_cpu_to_le32(0xc0),
    AT_EA_INFORMATION = const_cpu_to_le32(0xd0),
    AT_EA = const_cpu_to_le32(0xe0),
    AT_PROPERTY_SET = const_cpu_to_le32(0xf0),
    AT_LOGGED_UTILITY_STREAM = const_cpu_to_le32(0x100),
    AT_FIRST_USER_DEFINED_ATTRIBUTE = const_cpu_to_le32(0x1000),
    AT_END = const_cpu_to_le32(0xffffffff),
} ntfs_attr_types;

typedef enum {
    REPARSE_TAG_SYMLINKD = const_cpu_to_le32(0xA000000C),
    REPARSE_TAG_JUNCTION = const_cpu_to_le32(0xA0000003),
} ntfs_reparse_tags;

typedef struct {
    u32 security_id; /* The security_id assigned to the descriptor. */
} __attribute__((__packed__)) SII_INDEX_KEY;

typedef struct {
    u32 hash;        /* Hash of the security descriptor. */
    u32 security_id; /* The security_id assigned to the descriptor. */
} __attribute__((__packed__)) SDH_INDEX_KEY;

typedef struct {
    u32 data1;   /* The first eight hexadecimal digits of the GUID. */
    u16 data2;   /* The first group of four hexadecimal digits. */
    u16 data3;   /* The second group of four hexadecimal digits. */
    u8 data4[8]; /* The first two bytes are the third group of four
            hexadecimal digits. The remaining six bytes are the
            final 12 hexadecimal digits. */
} __attribute__((__packed__)) NTFS_GUID;

typedef struct {
    u32 reparse_tag; /* Reparse point type (inc. flags). */
    MFT_REF file_id; /* Mft record of the file containing the
                reparse point attribute. */
} __attribute__((__packed__)) NTFS_REPARSE_INDEX_KEY;

typedef union {
    struct {
        u16 high_part; /* High 16-bits. */
        u32 low_part;  /* Low 32-bits. */
    } __attribute__((__packed__)) spart;
    u8 value[6]; /* Value as individual bytes. */
} __attribute__((__packed__)) NTFS_SID_IDENTIFIER_AUTHORITY;

typedef struct {
    u8 revision;
    u8 sub_authority_count;
    NTFS_SID_IDENTIFIER_AUTHORITY identifier_authority;
    u32 sub_authority[1]; /* At least one sub_authority. */
} __attribute__((__packed__)) NTFS_SID;

/**
 * enum ATTR_FLAGS - Attribute flags (16-bit).
 */
typedef enum {
    ATTR_IS_COMPRESSED = const_cpu_to_le16(0x0001),
    ATTR_COMPRESSION_MASK = const_cpu_to_le16(0x00ff), /* Compression method mask. Also, first
                                                          illegal value. */
    ATTR_IS_ENCRYPTED = const_cpu_to_le16(0x4000),
    ATTR_IS_SPARSE = const_cpu_to_le16(0x8000),
} __attribute__((__packed__)) ntfs_attr_flags;

/**
 * enum ntfs_index_header_flags - Index header flags (8-bit).
 */
typedef enum {
    /* When index header is in an index root attribute: */
    SMALL_INDEX = 0, /* The index is small enough to fit inside the
             index root attribute and there is no index
             allocation attribute present. */
    LARGE_INDEX = 1, /* The index is too large to fit in the index
             root attribute and/or an index allocation
             attribute is present. */
    /*
     * When index header is in an index block, i.e. is part of index
     * allocation attribute:
     */
    LEAF_NODE = 0,  /* This is a leaf node, i.e. there are no more
              nodes branching off it. */
    INDEX_NODE = 1, /* This node indexes other nodes, i.e. is not a
             leaf node. */
    NODE_MASK = 1,  /* Mask for accessing the *_NODE bits. */
} __attribute__((__packed__)) ntfs_index_header_flags;

typedef enum {
    INDEX_ENTRY_NODE = const_cpu_to_le16(1), /* This entry contains a
                    sub-node, i.e. a reference to an index
                    block in form of a virtual cluster
                    number (see below). */
    INDEX_ENTRY_END = const_cpu_to_le16(2),  /* This signifies the last
                    entry in an index block. The index
                    entry does not represent a file but it
                    can point to a sub-node. */
    INDEX_ENTRY_SPACE_FILLER = 0xffff,       /* Just to force 16-bit width. */
} __attribute__((__packed__)) ntfs_index_entry_flags;

typedef enum {
    CASE_SENSITIVE = 0,
    IGNORE_CASE = 1,
} IGNORE_CASE_BOOL;

typedef enum {
    FILE_NAME_POSIX = 0x00,
    /* This is the largest namespace. It is case sensitive and
       allows all Unicode characters except for: '\0' and '/'.
       Beware that in WinNT/2k files which eg have the same name
       except for their case will not be distinguished by the
       standard utilities and thus a "del filename" will delete
       both "filename" and "fileName" without warning. */
    FILE_NAME_WIN32 = 0x01,
    /* The standard WinNT/2k NTFS long filenames. Case insensitive.
       All Unicode chars except: '\0', '"', '*', '/', ':', '<',
       '>', '?', '\' and '|'. Further, names cannot end with a '.'
       or a space. */
    FILE_NAME_DOS = 0x02,
    /* The standard DOS filenames (8.3 format). Uppercase only.
       All 8-bit characters greater space, except: '"', '*', '+',
       ',', '/', ':', ';', '<', '=', '>', '?' and '\'. */
    FILE_NAME_WIN32_AND_DOS = 0x03,
    /* 3 means that both the Win32 and the DOS filenames are
       identical and hence have been saved in this single filename
       record. */
} __attribute__((__packed__)) ntfs_file_name_type_flags;

typedef enum {
    // These flags are only present in the STANDARD_INFORMATION attribute
    // (in the field file_attributes).
    FILE_ATTR_READONLY = const_cpu_to_le32(0x00000001),
    FILE_ATTR_HIDDEN = const_cpu_to_le32(0x00000002),
    FILE_ATTR_SYSTEM = const_cpu_to_le32(0x00000004),
    // // Old DOS volid. Unused in NT.    = cpu_to_le32(0x00000008),

    FILE_ATTR_DIRECTORY = const_cpu_to_le32(0x00000010),
    // FILE_ATTR_DIRECTORY is not considered valid in NT. It is reserved
    // for the DOS SUBDIRECTORY flag.
    FILE_ATTR_ARCHIVE = const_cpu_to_le32(0x00000020),
    FILE_ATTR_DEVICE = const_cpu_to_le32(0x00000040),
    FILE_ATTR_NORMAL = const_cpu_to_le32(0x00000080),

    FILE_ATTR_TEMPORARY = const_cpu_to_le32(0x00000100),
    FILE_ATTR_SPARSE_FILE = const_cpu_to_le32(0x00000200),
    FILE_ATTR_REPARSE_POINT = const_cpu_to_le32(0x00000400),
    FILE_ATTR_COMPRESSED = const_cpu_to_le32(0x00000800),

    FILE_ATTR_OFFLINE = const_cpu_to_le32(0x00001000),
    FILE_ATTR_NOT_CONTENT_INDEXED = const_cpu_to_le32(0x00002000),
    FILE_ATTR_ENCRYPTED = const_cpu_to_le32(0x00004000),

    FILE_ATTR_VALID_FLAGS = const_cpu_to_le32(0x00007fb7),
    FILE_ATTR_VALID_SET_FLAGS = const_cpu_to_le32(0x000031a7),
    FILE_ATTR_I30_INDEX_PRESENT = const_cpu_to_le32(0x10000000),
    FILE_ATTR_VIEW_INDEX_PRESENT = const_cpu_to_le32(0x20000000),
} __attribute__((__packed__)) ntfs_file_attr_flags;

typedef enum {
    /* Found in $MFT/$DATA. */
    magic_FILE = const_cpu_to_le32(0x454c4946), /* Mft entry. */
    magic_INDX = const_cpu_to_le32(0x58444e49), /* Index buffer. */
    magic_HOLE = const_cpu_to_le32(0x454c4f48), /* ? (NTFS 3.0+?) */

    /* Found in $LogFile/$DATA. */
    magic_RSTR = const_cpu_to_le32(0x52545352), /* Restart page. */
    magic_RCRD = const_cpu_to_le32(0x44524352), /* Log record page. */

    /* Found in $LogFile/$DATA.  (May be found in $MFT/$DATA, also?) */
    magic_CHKD = const_cpu_to_le32(0x444b4843), /* Modified by chkdsk. */

    /* Found in all ntfs record containing records. */
    magic_BAAD = const_cpu_to_le32(0x44414142), /* Failed multi sector
                               transfer was detected. */

    /*
     * Found in $LogFile/$DATA when a page is full or 0xff bytes and is
     * thus not initialized.  User has to initialize the page before using
     * it.
     */
    magic_empty = const_cpu_to_le32(0xffffffff), /* Record is empty and has
                                to be initialized before
                                it can be used. */
} ntfs_record_types;

typedef enum {
    COLLATION_BINARY = const_cpu_to_le32(0),         /* Collate by binary
                        compare where the first byte is most
                        significant. */
    COLLATION_FILE_NAME = const_cpu_to_le32(1),      /* Collate file names
                     as Unicode strings. */
    COLLATION_UNICODE_STRING = const_cpu_to_le32(2), /* Collate Unicode
                    strings by comparing their binary
                    Unicode values, except that when a
                    character can be uppercased, the upper
                    case value collates before the lower
                    case one. */
    COLLATION_NTOFS_ULONG = const_cpu_to_le32(16),
    COLLATION_NTOFS_SID = const_cpu_to_le32(17),
    COLLATION_NTOFS_SECURITY_HASH = const_cpu_to_le32(18),
    COLLATION_NTOFS_ULONGS = const_cpu_to_le32(19),
} ntfs_collation_rules;

/*
 * BIOS parameter block (bpb) structure.
 */
typedef struct {
    le16 bytes_per_sector;  /* Size of a sector in bytes. */
    u8 sectors_per_cluster; /* Size of a cluster in sectors. */
    le16 reserved_sectors;  /* zero */
    u8 fats;                /* zero */
    le16 root_entries;      /* zero */
    le16 sectors;           /* zero */
    u8 media_type;          /* 0xf8 = hard disk */
    le16 sectors_per_fat;   /* zero */
    le16 sectors_per_track; /* Required to boot Windows. */
    le16 heads;             /* Required to boot Windows. */
    le32 hidden_sectors;    /* Offset to the start of the partition
                               relative to the disk in sectors.
                               Required to boot Windows. */
    le32 large_sectors;     /* zero */
} __attribute__((__packed__)) ntfs_parameter_block;

/*
 * struct NTFS_BOOT_SECTOR - NTFS boot sector structure.
 */
typedef struct {
    u8 jump[3];                   /* Irrelevant (jump to boot up code). */
    le64 oem_id;                  /* Magic "NTFS    ". */
    ntfs_parameter_block bpb;     /* See BIOS_PARAMETER_BLOCK. */
    u8 physical_drive;            /* 0x00 floppy, 0x80 hard disk */
    u8 current_head;              /* zero */
    u8 extended_boot_signature;   /* 0x80 */
    u8 reserved2;                 /* zero */
    sle64 number_of_sectors;      /* Number of sectors in volume. Gives
                                     maximum volume size of 2^63 sectors.
                                     Assuming standard sector size of 512
                                     bytes, the maximum byte size is
                                     approx. 4.7x10^21 bytes. (-; */
    sle64 mft_lcn;                /* Cluster location of mft data. */
    sle64 mftmirr_lcn;            /* Cluster location of copy of mft. */
    s8 clusters_per_mft_record;   /* Mft record size in clusters. */
    u8 reserved0[3];              /* zero */
    s8 clusters_per_index_record; /* Index block size in clusters. */
    u8 reserved1[3];              /* zero */
    le64 volume_serial_number;    /* Irrelevant (serial number). */
    le32 checksum;                /* Boot sector checksum. */
    u8 bootstrap[426];            /* Irrelevant (boot up code). */
    le16 end_of_sector_marker;    /* End of boot sector magic. Always is
                                     0xaa55 in little endian. */
                                  /* sizeof() = 512 (0x200) bytes */
} __attribute__((__packed__)) ntfs_boot_sector;

/*
 * The mft record header present at the beginning of every record in the mft.
 * This is followed by a sequence of variable length attribute records which
 * is terminated by an attribute of type AT_END which is a truncated attribute
 * in that it only consists of the attribute type code AT_END and none of the
 * other members of the attribute structure are present.
 */
typedef struct {
    /* Ofs */
    /*  0   NTFS_RECORD; -- Unfolded here as gcc doesn't like unnamed structs. */
    le32 magic;     /* Usually the magic is "FILE". */
    le16 usa_ofs;   /* See NTFS_RECORD definition above. */
    le16 usa_count; /* See NTFS_RECORD definition above. */

    /* 8 */ le64 lsn;              /* $LogFile sequence number for this record.
                                     Changed every time the record is modified. */
    /* 16 */ le16 sequence_number; /* Number of times this mft record has been
       reused. (See description for MFT_REF
       above.) NOTE: The increment (skipping zero)
       is done when the file is deleted. NOTE: If
       this is zero it is left zero. */
    /* 18 */ le16 link_count;      /* Number of hard links, i.e. the number of
       directory entries referencing this record.
       NOTE: Only used in mft base records.
       NOTE: When deleting a directory entry we
       check the link_count and if it is 1 we
       delete the file. Otherwise we delete the
       FILE_NAME_ATTR being referenced by the
       directory entry from the mft record and
       decrement the link_count.
       FIXME: Careful with Win32 + DOS names! */
    /* 20 */ le16 attrs_offset;    /* Byte offset to the first attribute in this
       mft record from the start of the mft record.
       NOTE: Must be aligned to 8-byte boundary. */
    /* 22 */ le16 flags;           /* Bit array of MFT_RECORD_FLAGS. When a file
       is deleted, the MFT_RECORD_IN_USE flag is
       set to zero. */
    /* 24 */ le32 bytes_in_use;    /* Number of bytes used in this mft record.
       NOTE: Must be aligned to 8-byte boundary. */
    /* 28 */ le32 bytes_allocated; /* Number of bytes allocated for this mft
       record. This should be equal to the mft record size. */
    /* 32 */ le64 base_mft_record;
    /* 40 */ le16 next_attr_instance;
    /* The below fields are specific to NTFS 3.1+ (Windows XP and above): */
    /* 42 */ le16 reserved;          /* Reserved/alignment. */
    /* 44 */ le32 mft_record_number; /* Number of this mft record. */
                                     /* sizeof() = 48 bytes */
} __attribute__((__packed__)) ntfs_mft_record;

typedef struct {
    /* hex ofs */
    /*  0 */ MFT_REF parent_directory;             /* Directory this filename is
                                 referenced from. */
    /*  8 */ s64 creation_time;                    /* Time file was created. */
    /* 10 */ s64 last_data_change_time;            /* Time the data attribute was last
modified. */
    /* 18 */ s64 last_mft_change_time;             /* Time this mft record was last
modified. */
    /* 20 */ s64 last_access_time;                 /* Last time this mft record was
accessed. */
    /* 28 */ s64 allocated_size;                   /* Byte size of on-disk allocated space
for the data attribute.  So for
normal $DATA, this is the
allocated_size from the unnamed
$DATA attribute and for compressed
and/or sparse $DATA, this is the
compressed_size from the unnamed
$DATA attribute.  NOTE: This is a
multiple of the cluster size. */
    /* 30 */ s64 data_size;                        /* Byte size of actual data in data
attribute. */
    /* 38 */ ntfs_file_attr_flags file_attributes; /* Flags describing the file. */
    /* 3c */ union {
        /* 3c */ struct {
            /* 3c */ u16 packed_ea_size; /* Size of the buffer needed to
                         pack the extended attributes
                         (EAs), if such are present. */
            /* 3e */ u16 reserved;       /* Reserved for alignment. */
        } __attribute__((__packed__)) sname;
        /* 3c */ u32 reparse_point_tag; /* Type of reparse point,
                     present only in reparse
                     points and only if there are
                     no EAs. */
    } __attribute__((__packed__)) uname;
    /* 40 */ u8 file_name_length;                      /* Length of file name in
                                      (Unicode) characters. */
    /* 41 */ ntfs_file_name_type_flags file_name_type; /* Namespace of the file name. */
    /* 42 */ u16 file_name[0];                         /* File name in Unicode. */
} __attribute__((__packed__)) ntfs_file_name_attr;

/*
 * Attribute record header. Always aligned to 8-byte boundary.
 */
typedef struct {
    /* Ofs */
    /*  0 */ le32 type;             /* The (32-bit) type of the attribute. */
    /*  4 */ le32 length;           /* Byte size of the resident part of the
                                   attribute (aligned to 8-byte boundary).
                                   Used to get to the next attribute. */
    /*  8 */ u8 non_resident;       /* If 0, attribute is resident.
                                   If 1, attribute is non-resident. */
    /*  9 */ u8 name_length;        /* Unicode character size of name of attribute.
                                0 if unnamed. */
    /* 10 */ le16 name_offset;      /* If name_length != 0, the byte offset to the
                                  beginning of the name from the attribute
                                  record. Note that the name is stored as a
                                  Unicode string. When creating, place offset
                                  just at the end of the record header. Then,
                                  follow with attribute value or mapping pairs
                                  array, resident and non-resident attributes
                                  respectively, aligning to an 8-byte boundary. */
    /* 12 */ ntfs_attr_flags flags; /* Flags describing the attribute.  */
    /* 14 */ le16 instance;         /* The instance of this attribute record. This
  number is unique within this mft record (see
  MFT_RECORD/next_attribute_instance notes
  above for more details). */
    /* 16 */ union {
        /* Resident attributes. */
        struct {
            /* 16 */ le32 value_length;     /* Byte size of attribute value. */
            /* 20 */ le16 value_offset;     /* Byte offset of the attribute
                                               value from the start of the
                                               attribute record. When creating,
                                               align to 8-byte boundary if we
                                               have a name present as this might
                                               not have a length of a multiple
                                               of 8-bytes. */
            /* 22 */ u8 resident_flags;     /* See above. */
            /* 23 */ s8 reservedR;          /* Reserved/alignment to 8-byte boundary. */
            /* 24 */ void *resident_end[0]; /* Use offset of(ntfs_attr_record,
                                               resident_end) to get size of
                                               a resident attribute. */
        } __attribute__((__packed__)) resident;
        /* Non-resident attributes. */
        struct {
            /* 16 */ sle64 lowest_vcn;          /* Lowest valid virtual cluster number
                                               for this portion of the attribute value or
                                               0 if this is the only extent (usually the
                                               case). - Only when an attribute list is used
                                               does lowest_vcn != 0 ever occur. */
            /* 24 */ sle64 highest_vcn;         /* Highest valid vcn of this extent of
                                               the attribute value. - Usually there is only one
                                               portion, so this usually equals the attribute
                                               value size in clusters minus 1. Can be -1 for
                                               zero length files. Can be 0 for "single extent"
                                               attributes. */
            /* 32 */ le16 mapping_pairs_offset; /* Byte offset from the
                                               beginning of the structure to the mapping pairs
                                               array which contains the mappings between the
                                               VCNs and the logical cluster numbers (LCNs).
                                               When creating, place this at the end of this
                                               record header aligned to 8-byte boundary. */
            /* 34 */ u8 compression_unit;       /* The compression unit expressed
                                               as the log to the base 2 of the number of
                                               clusters in a compression unit. 0 means not
                                               compressed. (This effectively limits the
                                               compression unit size to be a power of two
                                               clusters.) WinNT4 only uses a value of 4. */
            /* 35 */ u8 reserved1[5];           /* Align to 8-byte boundary. */
            /* The sizes below are only used when lowest_vcn is zero, as otherwise it would
               be difficult to keep them up-to-date. */
            /* 40 */ sle64 allocated_size;      /* Byte size of disk space
                                               allocated to hold the attribute value. Always
                                               is a multiple of the cluster size. When a file
                                               is compressed, this field is a multiple of the
                                               compression block size (2^compression_unit) and
                                               it represents the logically allocated space
                                               rather than the actual on disk usage. For this
                                               use the compressed_size (see below). */
            /* 48 */ sle64 data_size;           /* Byte size of the attribute
                                               value. Can be larger than allocated_size if
                                               attribute value is compressed or sparse. */
            /* 56 */ sle64 initialized_size;    /* Byte size of initialized
                                            portion of the attribute value. Usually equals
                                            data_size. */
            /* 64 */ void *non_resident_end[0]; /* Use offsetof(ntfs_attr_record,
                                                   non_resident_end) to get
                                                   size of a non resident attribute. */
                                                /* sizeof(uncompressed attr) = 64 */
            /* 64 */ sle64 compressed_size;     /* Byte size of the attribute
                                             value after compression. Only present when
                                             compressed. Always is a multiple of the
                                             cluster size. Represents the actual amount of
                                             disk space being used on the disk. */
            /* 72 */ void *compressed_end[0];   /* Use offsetof(ntfs_attr_record, compressed_end) to
                                                   get size of a compressed attribute. */
                                                /* sizeof(compressed attr) = 72 */
        } __attribute__((__packed__)) non_resident;
    } __attribute__((__packed__)) data;
} __attribute__((__packed__)) ntfs_attr_record;

/**
 * struct ntfs_record -
 *
 * The Update Sequence Array (usa) is an array of the u16 values which belong
 * to the end of each sector protected by the update sequence record in which
 * this array is contained. Note that the first entry is the Update Sequence
 * Number (usn), a cyclic counter of how many times the protected record has
 * been written to disk. The values 0 and -1 (ie. 0xffff) are not used. All
 * last u16's of each sector have to be equal to the usn (during reading) or
 * are set to it (during writing). If they are not, an incomplete multi sector
 * transfer has occurred when the data was written.
 * The maximum size for the update sequence array is fixed to:
 * maximum size = usa_ofs + (usa_count * 2) = 510 bytes
 * The 510 bytes comes from the fact that the last u16 in the array has to
 * (obviously) finish before the last u16 of the first 512-byte sector.
 * This formula can be used as a consistency check in that usa_ofs +
 * (usa_count * 2) has to be less than or equal to 510.
 */
typedef struct {
    ntfs_record_types magic; /* A four-byte magic identifying the
                    record type and/or status. */
    u16 usa_ofs;             /* Offset to the Update Sequence Array (usa)
                        from the start of the ntfs record. */
    u16 usa_count;           /* Number of u16 sized entries in the usa
                      including the Update Sequence Number (usn),
                      thus the number of fixups is the usa_count
                      minus 1. */
} __attribute__((__packed__)) ntfs_record;

typedef struct {
    /*  0 */ u32 entries_offset;               /* Byte offset from the ntfs_index_header to first
                               INDEX_ENTRY, aligned to 8-byte boundary.  */
    /*  4 */ u32 index_length;                 /* Data size in byte of the INDEX_ENTRY's,
including the ntfs_index_header, aligned to 8. */
    /*  8 */ u32 allocated_size;               /* Allocated byte size of this index (block),
multiple of 8 bytes. See more below.      */
                                               /*
                                                  For the index root attribute, the above two numbers are always
                                                  equal, as the attribute is resident and it is resized as needed.

                                                  For the index allocation attribute, the attribute is not resident
                                                  and the allocated_size is equal to the index_block_size specified
                                                  by the corresponding INDEX_ROOT attribute minus the INDEX_BLOCK
                                                  size not counting the ntfs_index_header part (i.e. minus -24).
                                                */
    /* 12 */ ntfs_index_header_flags ih_flags; /* Bit field of ntfs_index_header_flags.  */
    /* 13 */ u8 reserved[3];                   /* Reserved/align to 8-byte boundary. */
    /* sizeof() == 16 */
} __attribute__((__packed__)) ntfs_index_header;

typedef struct {
    /*  0 */ ntfs_record_types type;              /* Type of the indexed attribute. Is
                              $FILE_NAME for directories, zero
                              for view indexes. No other values
                              allowed. */
    /*  4 */ ntfs_collation_rules collation_rule; /* Collation rule used to sort the
index entries. If type is $FILE_NAME,
this must be COLLATION_FILE_NAME. */
    /*  8 */ u32 index_block_size;                /* Size of index block in bytes (in
the index allocation attribute). */
    /* 12 */ s8 clusters_per_index_block;         /* Size of index block in clusters (in
the index allocation attribute), when
an index block is >= than a cluster,
otherwise sectors per index block. */
    /* 13 */ u8 reserved[3];                      /* Reserved/align to 8-byte boundary. */
    /* 16 */ ntfs_index_header index;             /* Index header describing the
following index entries. */
    /* sizeof()= 32 bytes */
} __attribute__((__packed__)) ntfs_index_root;

typedef struct {
    /*  0 */ union {
        MFT_REF indexed_file;
        struct {
            u16 data_offset;
            u16 data_length;
            u32 reservedV;
        } __attribute__((__packed__)) sname;
    } __attribute__((__packed__)) uname;
    /*  8 */ u16 length;
    /* 10 */ u16 key_length;
    /* 12 */ ntfs_index_entry_flags flags;
    /* 14 */ u16 reserved;
    /* sizeof() = 16 bytes */
} __attribute__((__packed__)) ntfs_index_entry_header;

typedef struct {
    /*  0    INDEX_ENTRY_HEADER; -- Unfolded here as gcc dislikes unnamed structs. */
    union {                  /* Only valid when INDEX_ENTRY_END is not set. */
        u64 indexed_file;    /* The mft reference of the file
                            described by this index
                            entry. Used for directory
                            indexes. */
        struct {             /* Used for views/indexes to find the entry's data. */
            u16 data_offset; /* Data byte offset from this
                             INDEX_ENTRY. Follows the
                             index key. */
            u16 data_length; /* Data length in bytes. */
            u32 reservedV;   /* Reserved (zero). */
        } __attribute__((__packed__)) sname;
    } __attribute__((__packed__)) uname;
    /*  8 */ u16 length;                      /* Byte size of this index entry, multiple of
                                                8-bytes. Size includes INDEX_ENTRY_HEADER
                                                and the optional subnode VCN. See below. */
    /* 10 */ u16 key_length;                  /* Byte size of the key value, which is in the
       index entry. It follows field reserved. Not
       multiple of 8-bytes. */
    /* 12 */ ntfs_index_entry_flags ie_flags; /* Bit field of ntfs_index_entry* flags. */
    /* 14 */ u16 reserved;                    /* Reserved/align to 8-byte boundary. */
                                              /*    End of INDEX_ENTRY_HEADER */
    /* 16 */ union {                          /* The key of the indexed attribute. NOTE: Only present
       if INDEX_ENTRY_END bit in flags is not set. NOTE: On
       NTFS versions before 3.0 the only valid key is the
       ntfs_file_name_attr. On NTFS 3.0+ the following
       additional index keys are defined: */
        ntfs_file_name_attr file_name;        /* $I30 index in directories. */
        SII_INDEX_KEY sii;                    /* $SII index in $Secure. */
        SDH_INDEX_KEY sdh;                    /* $SDH index in $Secure. */
        NTFS_GUID object_id;                  /* $O index in FILE_Extend/$ObjId: The
                                              object_id of the mft record found in
                                              the data part of the index. */
        NTFS_REPARSE_INDEX_KEY reparse;       /* $R index in
                                               FILE_Extend/$Reparse. */
        NTFS_SID sid;                         /* $O index in FILE_Extend/$Quota:
                                                 SID of the owner of the user_id. */
        u32 owner_id;                         /* $Q index in FILE_Extend/$Quota:
                                                 user_id of the owner of the quota
                                                 control entry in the data part of
                                                 the index. */
    } __attribute__((__packed__)) key;
    /* The (optional) index data is inserted here when creating. */
    // VCN vcn;       If INDEX_ENTRY_NODE bit in ie_flags is set, the last
    //           eight bytes of this index entry contain the virtual
    //           cluster number of the index block that holds the
    //           entries immediately preceding the current entry.
    //
    //           If the key_length is zero, then the vcn immediately
    //           follows the INDEX_ENTRY_HEADER.
    //
    //           The address of the vcn of "ie" ntfs_index_entry is given by
    //           (char*)ie + le16_to_cpu(ie->length) - sizeof(VCN)
} __attribute__((__packed__)) ntfs_index_entry;


typedef struct {
    /*  0    NTFS_RECORD; -- Unfolded here as gcc doesn't like unnamed structs. */
    ntfs_record_types magic; /* Magic is "INDX". */
    u16 usa_ofs;             /* See NTFS_RECORD definition. */
    u16 usa_count;           /* See NTFS_RECORD definition. */

    /*  8 */ s64 lsn;                 /* $LogFile sequence number of the last
                             modification of this index block. */
    /* 16 */ s64 index_block_vcn;     /* Virtual cluster number of the index block. */
    /* 24 */ ntfs_index_header index; /* Describes the following index entries. */
    /* sizeof()= 40 (0x28) bytes */
    /*
     * When creating the index block, we place the update sequence array at this
     * offset, i.e. before we start with the index entries. This also makes sense,
     * otherwise we could run into problems with the update sequence array
     * containing in itself the last two bytes of a sector which would mean that
     * multi sector transfer protection wouldn't work. As you can't protect data
     * by overwriting it since you then can't get it back...
     * When reading use the data from the ntfs record header.
     */
} __attribute__((__packed__)) ntfs_index_block;

typedef ntfs_index_block ntfs_index_allocation;

/**
 * struct ATTR_LIST_ENTRY - Attribute: Attribute list (0x20).
 *
 * - Can be either resident or non-resident.
 * - Value consists of a sequence of variable length, 8-byte aligned,
 * ATTR_LIST_ENTRY records.
 * - The attribute list attribute contains one entry for each attribute of
 * the file in which the list is located, except for the list attribute
 * itself. The list is sorted: first by attribute type, second by attribute
 * name (if present), third by instance number. The extents of one
 * non-resident attribute (if present) immediately follow after the initial
 * extent. They are ordered by lowest_vcn and have their instance set to zero.
 * It is not allowed to have two attributes with all sorting keys equal.
 * - Further restrictions:
 * - If not resident, the vcn to lcn mapping array has to fit inside the
 * base mft record.
 * - The attribute list attribute value has a maximum size of 256kb. This
 * is imposed by the Windows cache manager.
 * - Attribute lists are only used when the attributes of mft record do not
 * fit inside the mft record despite all attributes (that can be made
 * non-resident) having been made non-resident. This can happen e.g. when:
 * - File has a large number of hard links (lots of file name
 * attributes present).
 * - The mapping pairs array of some non-resident attribute becomes so
 * large due to fragmentation that it overflows the mft record.
 * - The security descriptor is very complex (not applicable to
 * NTFS 3.0 volumes).
 * - There are many named streams.
 */
typedef struct {
    /* Ofs */
    /*  0 */ ntfs_attr_types type;  /* Type of referenced attribute. */
    /*  4 */ u16 length;            /* Byte size of this entry. */
    /*  6 */ u8 name_length;        /* Size in Unicode chars of the name of the
attribute or 0 if unnamed. */
    /*  7 */ u8 name_offset;        /* Byte offset to beginning of attribute name
(always set this to where the name would
start even if unnamed). */
    /*  8 */ s64 lowest_vcn;        /* Lowest virtual cluster number of this portion
of the attribute value. This is usually 0. It
is non-zero for the case where one attribute
does not fit into one mft record and thus
several mft records are allocated to hold
this attribute. In the latter case, each mft
record holds one extent of the attribute and
there is one attribute list entry for each
extent. NOTE: This is DEFINITELY a signed
value! The windows driver uses cmp, followed
by jg when comparing this, thus it treats it
as signed. */
    /* 16 */ MFT_REF mft_reference; /* The reference of the mft record holding
the ATTR_RECORD for this portion of the
attribute value. */
    /* 24 */ u16 instance;          /* If lowest_vcn = 0, the instance of the
attribute being referenced; otherwise 0. */
    /* 26 */ ntfschar name[0];      /* Use when creating only. When reading use
name_offset to determine the location of the
name. */
    /* sizeof() = 26 + (attribute_name_length * 2) bytes */
} __attribute__((__packed__)) ntfs_attr_list_entry;

typedef struct {
    /*  0 */ ntfs_reparse_tags reparse_tag; /* Reparse point type (inc. flags). */
    /*  4 */ MFT_REF file_id;               /* Mft record of the file containing the
 reparse point attribute. */
    /* 12 */ u16 reserved;
    /* 14 */ u16 file_name_length; /* Unicode character length */
} __attribute__((__packed__)) ntfs_reparse;

/* 卷信息属性,取得文件系统的版本信息   */
typedef struct {
    u64 unused;   /* 总为0    */
    u8 major_vor; /* 主版本    */
    u8 minor_vor; /* 次版本    */
    u16 flag;     /* 标志     */
    u32 unused1;  /* 填充至8字节边界        */
} __attribute__((__packed__)) ntfs_attr_volinfo;

// 用于查找文件
typedef struct {
    ntfs_index_entry *index_entry; // 索引块数据起始位置
    ntfschar *search_name;         // 文件名
    int32_t search_len;            // 文件名长度
    uint16_t node_flag;            // 节点标识
    uint8_t *start_pos;            // 数据开始位置
    uint8_t *end_pos;              // 数据结束位置
} __attribute__((__packed__)) ntfs_search_condition;

// 用于遍历属性列表
struct ntfs_attr_look {
    uint32_t attr_num; // 属性类型值
    uint32_t attr_len; // 属性长度
};

// 定义分区基本信息
typedef struct {
    uint32_t sector_size;      // 扇区大小
    uint32_t cluster_size;     // 簇大小
    uint32_t mft_size;         // MFT项大小
    uint32_t index_block_size; // 索引块大小
    uint64_t sector_count;     // 分区扇区数(BOOT描述的扇区数-1)
    uint64_t mft_lcn;          // 第一个MFT项的起始簇号
} __attribute__((__packed__)) ntfs_part_info;

#pragma pack(pop)

/**
 * @brief NTFS文件系统分析内部处理类
 */
class ntfsUtility {
public:
    /* *
     * @brief 构造函数
     */
    ntfsUtility()
    {
        // 初始化
        m_reader = NULL;
        m_upcase_len = 0;
        m_upcase_data = NULL;

        m_attr_list_length = 0;
        m_atrr_list_data = NULL;

        m_mft_buffer = NULL;
        m_attr_list_mft_buffer = NULL;

        m_reparse_flg = 0;

        m_mft_zone_count = 0;
        m_mft_zone_runlist = NULL;
        memset_s(&m_ntfs_info, sizeof(ntfs_part_info), 0, sizeof(ntfs_part_info));
    }
    /* *
     * @brief 析构函数
     */
    ~ntfsUtility()
    {
        this->m_reader = NULL;
        this->m_upcase_data = NULL;
        this->m_atrr_list_data = NULL;
        this->m_mft_buffer = NULL;
        this->m_attr_list_mft_buffer = NULL;
        this->m_mft_zone_runlist = NULL;
    }

    /* *
     * @brief 初始化NTFS文件系统的基本信息
     *
     * @param &part_info  将分区基本数据返回
     * @param *img_reader 读取镜像的Reader指针
     *
     * @return int32_t 0  成功
     * 负数  失败
     *
     */
    int32_t ntfs_initFSInfo(ntfs_part_info &part_info, imgReader *img_reader);

    /* *
     * @brief 申请文件过滤时所需要的MFT数据空间
     * @return            0 成功
     * AFS_ERR_API 失败
     */
    int32_t ntfs_callocSearchSpace();
    /* *
     * @brief 分析MFT记录号所对应的簇流位置（一般情况下MFT记录号使用一个簇流存储）
     * @return int32_t AFS_SUCCESS 成功
     * AFS_ERR_IMAGE_READ 读失败
     * AFS_ERR_INNER      内部错误
     *
     */
    int32_t ntfs_getMFTZoneRunlist();

    /* *
     * @brief 根据MFT记录号，读取该MFT记录的数据(大小为1024字节)
     *
     * @param mft_no      MFT记录号
     * @param *mft_buffer 返回读取的MFT数据
     *
     * @return int32_t AFS_SUCCESS 成功
     * AFS_ERR_IMAGE_READ 读失败
     * AFS_ERR_INNER      内部错误
     *
     */
    int32_t ntfs_getMFTData(uint64_t mft_no, uint8_t *mft_buffer);
    /* *
     * @brief 获取MFT记录中指定的属性
     *
     * @param *mft_buffer     MFT数据
     * @param attr_num        属性ID
     * @param *attr_record    属性内容
     * @param *attr_start     返回属性起始位置
     *
     * @return int32_t 0 成功
     * -1  失败
     *
     */
    int32_t ntfs_getMFTAttr(uint8_t *mft_buffer, uint32_t attr_num, ntfs_attr_record *attr_record,
        uint32_t *attr_start);

    /* *
     * @brief 查找指定文件名的MFT记录号
     *
     * @param mft_no_search  当前目录的MFT记录号
     * @param *search_name   所要查找的文件名(Unicode字符)
     * @param search_len     文件名长度(Unicode字符)
     * @param &mft_result    查找结果的MFT记录号
     * @param &mft_attr      MFT记录的属性(文件属性)
     *
     * @return         AFS_SUCCESS： 成功
     * AFS_ERR_NOT_EXIST_PATH： 不能正常匹配文件名
     * 其他 ： 失败（错误ID）
     *
     */
    int32_t ntfs_searchMFTByName(MFT_REF mft_no_search, ntfschar *search_name, int32_t search_len, MFT_REF &mft_result,
        uint32_t &mft_attr);
    /* *
     * @brief 分析重解析目录
     *
     * @param mft_no         当前文件的MFT记录号
     * @param *reparse_path  重解析路径
     * @return 0 非重解析点
     * 1 重解析点，返回reparse_path
     * 负数  分析失败
     *
     */
    int32_t ntfs_analyzeReparsePath(MFT_REF mft_no, char *reparse_path);

    /* *
     * @brief 过滤当前MFT记录对应的子目录以及文件
     *
     * @param mft_no        当前文件MFT记录号
     * @param mft_attr      当前文件属性
     * @param &bitmap       返回过滤文件的bitmap
     *
     * @return 0 成功
     * AFS_ERR_FILE_TYPE 不支持的文件类型
     * 负值 失败
     */
    int32_t ntfs_getFileFilterBitmap(MFT_REF mft_no, uint32_t mft_attr, BitMap &bitmap);

    /* *
     * @brief 将多字符转换为Unicode字符
     *
     * @param *ins       多字符的Buffer
     * @param *outs      返回转换后的Unicode字符Buffer
     * @param outs_len   Unicode字符长度
     *
     * @return int32_t 0  成功
     * -1 失败
     *
     */
    int32_t ntfs_mbstoucs(const char *ins, ntfschar *outs, int32_t outs_len);

    /* *
     * @brief 读取NTFS文件系统的大小写表
     * @return  0 读取成功
     * 其他  失败
     */
    int32_t ntfs_getVolumeUpcaseData();

    /* *
     * @brief 检查NTFS文件系统版本，本项目支持的NTFS版本为(3.1)
     * @return            0 读取成功
     * AFS_ERR_INNER 内部错误
     * 其他负值   错误
     */
    int32_t ntfs_checkVersion();
    /* *
     * @brief 释放元文件$MFT的80H对应的数据
     * @return 无
     */
    void ntfs_freeMFTZoneSpace();

    /* *
     * @brief 释放文件过滤时所分配的数据空间
     * @return 无
     */
    void ntfs_freeSearchSpace();
    /* *
     * @brief 读取指定属性的实际数据
     *
     * @param *mft_buffer            MFT记录的数据
     * @param *attr_head             属性头
     * @param attr_start             属性开始位置
     * @param **data_buffer          返回读取到的数据
     * @param &data_size             返回读取到的数据长度
     *
     * @return  大于0 正常读取成功
     * 负数     失败
     *
     */
    int32_t ntfs_readAttributeData(uint8_t *mft_buffer, ntfs_attr_record *attr_head, uint32_t attr_start,
        uint8_t **data_buffer, uint64_t &data_size);

#ifdef CPPUNIT_MAIN
protected:
#else
private:
#endif
    /* *
     * @brief 复制拷贝构造函数
     *   */
    ntfsUtility(const ntfsUtility &ntfsUtil_Obj);
    /* *
     * @brief 赋值拷贝构造函数
     *   */
    ntfsUtility &operator = (const ntfsUtility &ntfsUtil_Obj);

    // 内部成员
    struct ntfs_runlist_element *m_mft_zone_runlist;
    uint32_t m_mft_zone_count;

    imgReader *m_reader;        // 从镜像读数据对象
    ntfs_part_info m_ntfs_info; // 分区信息

    // 大小写表
    uint64_t m_upcase_len;
    ntfschar *m_upcase_data;

    // 当MFT记录中存在属性列表(20H属性)时使用
    uint64_t m_attr_list_length;
    uint8_t *m_atrr_list_data;

    // MFT记录
    uint8_t *m_mft_buffer; // MFT记录数据
    // 数据列表
    uint8_t *m_attr_list_mft_buffer; // 属性列表数据

    uint8_t m_reparse_flg; // 重解析标识

    ntfsCommon m_ntfscomm;

    /* *
     * @brief  设置查找到的文件在Bitmap中的对应位
     *
     * @param   mft_no     文件对应的MFT记录号
     * @param   &bitmap    返回Bitmap数据
     * @return 0：成功  负数：失败
     *
     */
    int32_t ntfs_getFileBitmap(MFT_REF mft_no, BitMap &bitmap);
    int32_t ntfs_getFileBitmap_1(MFT_REF mft_no, uint8_t *mft_buffer_filter, int32_t attr_list_flag, BitMap &bitmap);
    int32_t ntfs_getFileBitmap_2(uint8_t *mft_buffer, ntfs_attr_record *attr_head, uint32_t attr_start, BitMap &bitmap);

    // 过滤整个目录
    int32_t ntfs_FilterDirectory(MFT_REF mft_no_filter, BitMap &bitmap);

    // 过滤根目录
    int32_t ntfs_filterRootDir(uint64_t mft_no_search, int32_t attr_list_flag, uint32_t &index_block_size,
        queue<uint64_t> &file_queue, BitMap &bitmap);
    // 过滤AO属性对应数据
    int32_t ntfs_filterIndexBlock(uint8_t *ia_data_buffer, uint64_t ia_size, uint8_t *ia_bitmap_buffer,
        uint64_t ia_bitmap_size, uint32_t index_block_size, queue<uint64_t> &file_queue, uint8_t *index_block_buffer,
        BitMap &bitmap);

    int32_t ntfs_getAllocationData(uint64_t mft_no_search, int32_t attr_list_flag, uint32_t attr_num,
        uint8_t **data_buffer, uint64_t &data_size);

    // 检查MFT记录中是否存在属性列表
    int32_t ntfs_checkMFTAttrList();
    uint8_t *ntfs_getMFTDataByAttrList(MFT_REF mft_no, uint8_t *mft_buffer_main, int32_t attr_list_flg,
        uint32_t attr_num);

    // 非常驻属性时读取数据流列表
    int32_t ntfs_getRunlist(uint8_t *mft_buffer, ntfs_attr_record *attr_head, uint32_t data_attr_start,
        struct ntfs_runlist_element **data_runlist, uint32_t &runlist_element_num);

    // 内部函数
    int32_t ntfs_readDataByRunlist(uint32_t runlist_element_num, const struct ntfs_runlist_element *data_runlist,
        uint8_t *process_data_cluster);
    // 根据指定的数据流列表读取相应的数据
    int32_t ntfs_getDataFromRunlist(uint8_t *mft_buffer, ntfs_attr_record *attr_head, uint32_t data_attr_start,
        uint8_t *data_buffer, uint64_t buffer_size);

    // 处理数据块的最后两个字节
    int32_t ntfs_updatePostReadFixup(ntfs_record *record, const u32 record_size);

    // 从索引块中查找指定文件名
    int32_t ntfs_findEntry(ntfs_search_condition *search_obj, uint64_t &vcnID, MFT_REF &mft_result, uint32_t &mft_attr);
    int32_t ntfs_findEntry_1(ntfs_index_entry *index_entry, ntfschar *search_name, int32_t search_len,
        MFT_REF &mft_result, uint32_t &mft_attr);
    void ntfs_printFileName(const ntfs_index_entry *index_entry);
    int32_t ntfs_isLeafNode(ntfs_index_entry *index_entry, uint16_t node_flag, uint64_t &vcnID);

    // 查找索引块中的所有文件/目录
    int32_t ntfs_findMFTByIndexBlock(ntfs_index_entry *index_entry, uint8_t *start_pos, uint8_t *end_pos,
        mft_map &mft_result);
    void ntfs_findMFTByIndexBlock_1(ntfs_index_entry *index_entry, mft_map &mft_result, uint64_t tmp_mft_no);
    // 将查找到的目录更新到队列中
    void ntfs_updateDirectoryQueue(mft_map &find_result_map, queue<uint64_t> &file_queue, BitMap &bitmap);

    int32_t ntfs_filterMFTDir(uint64_t mft_no_search, int32_t attr_list_flag, queue<uint64_t> &file_queue,
        uint8_t *index_block_buffer, BitMap &bitmap);

    int32_t ntfs_searchByRoot(MFT_REF mft_no_search, int32_t attr_list_flag, ntfs_search_condition *search_obj,
        uint32_t &index_block_size, MFT_REF &mft_result, uint32_t &mft_attr, uint64_t &vcn_id);

    int32_t ntfs_getSearchFileRunlist(MFT_REF mft_no_search, int32_t attr_list_flag,
        struct ntfs_runlist_element **data_runlist, uint32_t &runlist_element_num);

    int32_t ntfs_searchByAllocation(MFT_REF mft_no_search, int32_t attr_list_flag, ntfs_search_condition *search_obj,
        uint64_t vcn_id_root, uint32_t index_block_size, MFT_REF &mft_result, uint32_t &mft_attr);
    int32_t ntfs_searchByRunlist(struct ntfs_runlist_element *data_runlist, uint32_t runlist_element_num,
        uint32_t index_vcn_size, ntfs_search_condition *search_obj, uint64_t vcn_id_root, uint32_t index_block_size,
        MFT_REF &mft_result, uint32_t &mft_attr);

    int32_t ntfs_getReparsePath(uint8_t *reparse_data_buffer, char *reparse_path);

    // 检查索引块有效性
    int32_t ntfs_checkIndexBlock(uint8_t *index_block_buffer, uint32_t index_block_size, uint64_t vcn_id);

    // 计算VCN开始位置
    int64_t ntfs_getIndexBlockByVCNPosition(const struct ntfs_runlist_element *data_runlist,
        uint32_t runlist_element_num, uint32_t index_block_size, uint32_t index_vcn_size, uint64_t index_block_vcn_id,
        uint8_t *index_block_buffer);

    // Unicode字符串比较
    int32_t ntfs_collateNames(const ntfschar *name1, const uint32_t name1_len, const ntfschar *name2,
        const uint32_t name2_len, const IGNORE_CASE_BOOL ic, const ntfschar *upcase, const uint32_t upcase_len);
    int32_t ntfs_collateNames_1(const IGNORE_CASE_BOOL ic, const ntfschar *upcase, const uint32_t upcase_len,
        const ntfschar c1, const ntfschar c2);

    // 获取分区的大小写更新表
    int32_t ntfs_getVolumeUpcase(ntfschar *unicode, uint64_t upcase_len, uint8_t *upcase_mft,
        ntfs_attr_record upcase_attr_head, uint32_t upcase_data_attr_start);
    int32_t ntfs_getVolumeUpcase_1(ntfschar *unicode, uint64_t upcase_len, uint64_t clusterCount,
        struct ntfs_runlist_element *upcase_runlist, uint32_t runlist_element_num, ntfs_attr_record upcase_attr_head);

    // Unicode 字符转换为多字符
    int32_t ntfs_ucstombs(const ntfschar *ins, const int32_t ins_len, char **outs, int32_t outs_len);

    int32_t ntfs_doConvertMB2UC(const char *ins, ntfschar *ucs, int32_t ucs_len);
    int32_t ntfs_doConvertUC2MBs(const ntfschar *ins, const int32_t ins_len, int32_t mbs_len, char **mbs, char **outs);

    int32_t ntfs_getMFTZoneRunlist_1(uint8_t *mft_buffer);
    //    int32_t ntfs_doConvertUC2MBs_1(int32_t out_len, int32_t mbs_len,
    //                                char **mbs, char **outs);
};

#endif // NTFS_UTILITY_H_INCLUDED
