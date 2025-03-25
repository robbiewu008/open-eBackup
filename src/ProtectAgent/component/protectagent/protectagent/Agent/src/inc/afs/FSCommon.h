/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file FSCommon.h
 * @brief AFS - Analyze file system common class.
 *
 */

#ifndef __AFS_COMMON_H__
#define __AFS_COMMON_H__

#include "afs/AfsType.h"

typedef uint8_t u8; /* Unsigned types of an exact size */
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8; /* Signed types of an exact size */
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif
#ifdef __CHECK_ENDIAN__
#define __bitwise __bitwise__
#else
#define __bitwise
#endif
typedef u16 __bitwise le16;
typedef u32 __bitwise le32;
typedef u64 __bitwise le64;
typedef u16 __bitwise sle16;
typedef u32 __bitwise sle32;
typedef u64 __bitwise sle64;

typedef u16 __be16;
typedef u32 __le32;
typedef u32 __be32;
typedef u64 __beu64;

#define SECTOR_SIZE 512

// 分区识别幻数
#define EXT_MAGIC 0xEF53
#define XFS_MAGIC 0x58465342
#define NTFS_MAGIC 0x202020205346544eULL

enum {
    /* These three have identical behaviour; use the second one if DOS FDISK gets
       confused about extended/logical partitions starting past cylinder 1023. */
    DOS_EXTENDED_PARTITION = 5,
    LINUX_EXTENDED_PARTITION = 0x85,
    WIN98_EXTENDED_PARTITION = 0x0f,

    SUN_WHOLE_DISK = DOS_EXTENDED_PARTITION,

    LINUX_SWAP_PARTITION = 0x82,
    LINUX_DATA_PARTITION = 0x83,
    LINUX_LVM_PARTITION = 0x8e,
    LINUX_RAID_PARTITION = 0xfd, /* autodetect RAID partition */

    SOLARIS_X86_PARTITION = LINUX_SWAP_PARTITION,
    NEW_SOLARIS_X86_PARTITION = 0xbf,

    DM6_AUX1PARTITION = 0x51, /* no DDO:  use xlated geom */
    DM6_AUX3PARTITION = 0x53, /* no DDO:  use xlated geom */
    DM6_PARTITION = 0x54,     /* has DDO: use xlated geom & offset */
    EZD_PARTITION = 0x55,     /* EZ-DRIVE */

    FREEBSD_PARTITION = 0xa5,  /* FreeBSD Partition ID */
    OPENBSD_PARTITION = 0xa6,  /* OpenBSD Partition ID */
    NETBSD_PARTITION = 0xa9,   /* NetBSD Partition ID */
    BSDI_PARTITION = 0xb7,     /* BSDI Partition ID */
    MINIX_PARTITION = 0x81,    /* Minix Partition ID */
    UNIXWARE_PARTITION = 0x63, /* Same as GNU_HURD and SCO Unix */
};

struct swsusp_header {
    char reserved[4060]; // (4096 - 20 - sizeof(uint64_t) - sizeof(int) - sizeof(uint32_t))
    uint32_t crc32;
    uint64_t image;
    uint32_t flags; /* Flags to pass to the "boot" kernel */
    char orig_sig[10];
    char sig[10];
} __attribute__((packed));

#define AFS_SWAP_PART 0
#define AFS_NORMAL_PART 1

/**
 * @brief 输入参数信息
 */
typedef struct input_para_space {
    AFS_HANDLE handle;
    AFS_READ_CALLBACK_FUNC_t read_callback_func;

    const char *file_path;

    union {
        int32 part_index;
        int32 input_part_num;
    };

    union {
        char *bitmap_buffer;
        char *part_info;
    };

    int64 buffer_size;

    int32 bytes_per_bit;

    // 过滤文件列表使用
    const char **file_list;
    int32 file_num;
} input_para_space_t;

typedef struct input_para_space_2 {
    AFS_HANDLE *handle;
    AFS_READ_CALLBACK_FUNC_t read_callback_func;
    char *fs_uuid;
    int32 disk_num;
    char **bitmap_buffer;
    int64 *buffer_size;
    int32 bytes_per_bit;
    char **file_list; // 过滤文件列表使用
    int32 file_num;
} input_para_space_2_t;

/**
 * @brief 分区操作函数
 */
struct partitionOpt : public partition {
public:
    partitionOpt()
    {
        part_type = (unsigned char)0; // 分区类型
        part_id = 0;                  // 分区ID(第一个分区ID为0)

        is_lvm = false; // 是否是LVM管理

        // 分区信息
        offset = 0;                   // 分区偏移
        length = 0;                   // 分区长度(512倍数)
        fstype = AFS_FILESYSTEM_NULL; // 分区文件系统类型
        disk_id = 0;
        is_pv_part = false; // 默认是单独的物理分区

        lvm_info.lv_length = 0;
        lvm_info.lv_name[0] = '0';
        lvm_info.lv_offset = 0;
        lvm_info.lv_update = 1;
        lvm_info.lv_pvUUID[AFS_PV_UUID_LEN] = '\0';
    }

    void setPartInfo(unsigned char ptype, int32_t pid, bool pis_lvm, uint64_t poffset, uint64_t plength,
        AFS_FSTYPE_t pfstype)
    {
        part_type = ptype; // 分区类型
        part_id = pid;     // 分区ID(第一个分区ID为0)

        is_lvm = pis_lvm; // 是否是LVM管理

        // 分区信息
        offset = poffset; // 分区偏移
        length = plength; // 分区长度(512倍数)
        fstype = pfstype; // 分区文件系统类型
    }
};

#endif /* INCLUDE_LVM_TYPE_H_ */
