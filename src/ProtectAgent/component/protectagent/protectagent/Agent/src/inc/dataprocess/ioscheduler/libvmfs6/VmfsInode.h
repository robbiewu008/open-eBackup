/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * @file
 * @brief
 * @version 1.0.0.0
 * @date 2023-11-22
 * @author
 */
#ifndef __VMFS6_INODE_H__
#define __VMFS6_INODE_H__

#include <stddef.h>
#include <sys/stat.h>
#include <mutex>


#define VMFS6_INODE_SIZE 0x2000
#define VMFS6_INODE_BLK_COUNT 0x140

#define VMFS6_INODE_MAGIC 0x10c00001


namespace Vmfs6IO {
struct VmfsInodeRaw {
    struct VmfsMetadataHdrRaw mdh;
    uint32_t id;
    uint32_t blkFdItem;
    uint32_t nLink;
    uint32_t type;
    uint32_t flags;
    uint64_t size;
    uint64_t blkSize;
    uint64_t blkCount;
    uint32_t mtime;
    uint32_t ctime;
    uint32_t atimev;
    uint32_t uid;
    uint32_t gid;
    unsigned int mode;
    uint32_t zla;
    uint32_t tbz;
    uint32_t cow;
    u_char unknown2[432];
    union {
        struct {
            u_char _unknown3[0x400];
            uint64_t blocks[VMFS6_INODE_BLK_COUNT];
        };
        uint32_t rdmId;
        char content[(VMFS6_INODE_BLK_COUNT * sizeof(uint64_t)) + 0x400];
    };
} __attribute__((packed));

#define VMFS6_INODE_OFFSET_ID offsetof(struct VmfsInodeRaw, id)
#define VMFS6_INODE_OFFSET_ID2 offsetof(struct VmfsInodeRaw, blkFdItem)
#define VMFS6_INODE_OFFSET_NLINK offsetof(struct VmfsInodeRaw, nLink)
#define VMFS6_INODE_OFFSET_TYPE offsetof(struct VmfsInodeRaw, type)
#define VMFS6_INODE_OFFSET_FLAGS offsetof(struct VmfsInodeRaw, flags)
#define VMFS6_INODE_OFFSET_SIZE offsetof(struct VmfsInodeRaw, size)
#define VMFS6_INODE_OFFSET_BLK_SIZE offsetof(struct VmfsInodeRaw, blkSize)
#define VMFS6_INODE_OFFSET_BLK_COUNT offsetof(struct VmfsInodeRaw, blkCount)
#define VMFS6_INODE_OFFSET_MTIME offsetof(struct VmfsInodeRaw, mtime)
#define VMFS6_INODE_OFFSET_CTIME offsetof(struct VmfsInodeRaw, ctime)
#define VMFS6_INODE_OFFSET_ATIME offsetof(struct VmfsInodeRaw, atimev)
#define VMFS6_INODE_OFFSET_UID offsetof(struct VmfsInodeRaw, uid)
#define VMFS6_INODE_OFFSET_GID offsetof(struct VmfsInodeRaw, gid)
#define VMFS6_INODE_OFFSET_MODE offsetof(struct VmfsInodeRaw, mode)
#define VMFS6_INODE_OFFSET_ZLA offsetof(struct VmfsInodeRaw, zla)
#define VMFS6_INODE_OFFSET_TBZ offsetof(struct VmfsInodeRaw, tbz)
#define VMFS6_INODE_OFFSET_COW offsetof(struct VmfsInodeRaw, cow)

#define VMFS6_INODE_OFFSET_BLK_ARRAY offsetof(struct VmfsInodeRaw, blocks)
#define VMFS6_INODE_OFFSET_RDM_ID offsetof(struct VmfsInodeRaw, rdmId)
#define VMFS6_INODE_OFFSET_CONTENT offsetof(struct VmfsInodeRaw, content)

#define VMFS6_INODE_SYNC_META 0x01
#define VMFS6_INODE_SYNC_BLK 0x02
#define VMFS6_INODE_SYNC_ALL (VMFS6_INODE_SYNC_META | VMFS6_INODE_SYNC_BLK)

#define VMFS6_ZLA_BASE 4301

struct VmfsInodeS {
    VmfsMetadataHdrT mdh;
    uint32_t id, blkFdItem;
    uint32_t nLink;
    uint32_t type;
    uint32_t flags;
    uint64_t size;
    uint64_t blkSize;
    uint64_t blkCount;
    time_t mtime, ctime, atimev;
    uint32_t uid, gid;
    uint32_t mode, cmode;
    uint32_t zla, tbz, cow;
    uint32_t rdmId;
    union {
        uint64_t blocks[VMFS6_INODE_BLK_COUNT];
        char content[VMFS6_INODE_BLK_COUNT * sizeof(uint64_t) + 0x400];
    };

    /* In-core inode information */
    const VmfsFsT *fs;
    VmfsInodeT **pprev, *next;
    u_int refCount;
    u_int update_flags;
};
using VmfsInodeT = struct VmfsInodeS;

/* Callback function for vmfs_inode_foreach_block() */
typedef void (*VmfsInodeForeachBlockCbkT)(const VmfsInodeT *inode, uint32_t pbBlk, uint64_t blkId, void *optArg);

class VmfsInode {
public:
    static VmfsInode *Instance();
    virtual ~VmfsInode() = default;

    /* Update an inode on disk */
    int Update(const VmfsInodeT *inode, int update_blk_list);

    /* Acquire an inode */
    VmfsInodeT *Acquire(const VmfsFsT *fs, uint64_t blkId);

    /* Release an inode */
    void Release(VmfsInodeT *inode);

    /*
     * Get block ID corresponding the specified position. Pointer block
     * resolution is transparently done here.
     */
    int GetBlock(const VmfsInodeT *inode, off_t pos, uint64_t *blkId);

    /* Get inode status */
    int StatFromBlkid(const VmfsFsT *fs, uint64_t blkId, struct stat *buf);

private:
    /* Get inode corresponding to a block id */
    int GetInodeId(const VmfsFsT *fs, uint64_t blkId, VmfsInodeT *inode);
    /* Get inode status */
    int InodeStat(const VmfsInodeT *inode, struct stat *buf);
    uint64_t ReadBlkid(const u_char *buf, u_int index);
    void WriteBlkid(u_char *buf, u_int index, uint64_t blkId);
    /* Read an inode */
    int InodeRead(VmfsInodeT *inode, const u_char *buf);
    /* Write an inode */
    int InodeWrite(const VmfsInodeT *inode, u_char *buf);
    /* Update block list */
    void WriteBlkList(const VmfsInodeT *inode, u_char *buf);
    /* Hash function to retrieve an in-core inode */
    u_int InodeHash(const VmfsFsT *fs, uint64_t blkId);
    /* Register an inode in the in-core inode hash table */
    void Register(const VmfsFsT *fs, VmfsInodeT *inode);
    /* Get block ID corresponding the specified position. Double Indirecting Addressing, Pointer block */
    int DoubleIndirectAddr(const VmfsInodeT *inode, off_t pos, uint64_t *blkId);

private:
    VmfsInode() = default;
    static VmfsInode *m_instance;
    static std::mutex m_mutex;
};
}

#endif
