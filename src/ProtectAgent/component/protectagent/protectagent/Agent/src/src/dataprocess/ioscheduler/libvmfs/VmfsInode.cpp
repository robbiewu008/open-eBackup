/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * @file
 * @brief
 * @version 1.0.0.0
 * @date 2023-11-22
 * @author
 */
/*
 * VMFS inodes.
 */

#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <sys/stat.h>
#include "dataprocess/ioscheduler/libvmfs/Vmfs.h"


namespace Vmfs5IO {
VmfsInode *VmfsInode::m_instance = nullptr;
std::mutex VmfsInode::m_mutex;

VmfsInode *VmfsInode::Instance()
{
    if (m_instance == nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_instance == nullptr) {
            m_instance = new (std::nothrow) VmfsInode();
        }
    }
    return m_instance;
}

/* Update an inode on disk */
int VmfsInode::Update(const VmfsInodeT *inode, int update_blk_list)
{
    DECL_ALIGNED_BUFFER(buf, VMFS5_INODE_SIZE);

    int ret = memset_s(buf, VMFS5_INODE_SIZE, 0, VMFS5_INODE_SIZE);
    if (ret != 0) {
        return (-1);
    }
    InodeWrite(inode, buf);

    if (update_blk_list) {
        WriteBlkList(inode, buf);
    } else {
        buf_len -= VMFS5_INODE_BLK_COUNT * sizeof(uint32_t);
    }

    if (VmfsDeviceWrite(inode->fs->dev, inode->mdh.pos, buf, buf_len) != buf_len) {
        return (-1);
    }

    return (0);
}

/* Get inode corresponding to a block id */
int VmfsInode::GetInodeId(const VmfsFsT *fs, uint32_t blkId, VmfsInodeT *inode)
{
    DECL_ALIGNED_BUFF_WOL(buf, VMFS5_INODE_SIZE);

    if (VMFS5_BLK_TYPE(blkId) != VMFS5_BLK_TYPE_FD) {
        return (-1);
    }

    if (!VmfsBitmap::Instance()->VmfsBitmapGetItem(fs->fdc, VMFS5_BLK_FD_ENTRY(blkId), VMFS5_BLK_FD_ITEM(blkId), buf)) {
        return (-1);
    }

    return (InodeRead(inode, buf));
}

/* Acquire an inode */
VmfsInodeT *VmfsInode::Acquire(const VmfsFsT *fs, uint32_t blkId)
{
    VmfsInodeT *inode;
    u_int hb;

    hb = InodeHash(fs, blkId);
    for (inode = fs->inodes[hb]; inode; inode = inode->next)
        if (inode->id == blkId) {
            inode->refCount++;
            return inode;
        }

    /* Inode not yet used, allocate room for it */
    if (!(inode = (VmfsInodeT *)calloc(1, sizeof(*inode)))) {
        return NULL;
    }

    if (GetInodeId(fs, blkId, inode) == -1) {
        free(inode);
        return NULL;
    }

    Register(fs, inode);
    return inode;
}

/* Release an inode */
void VmfsInode::Release(VmfsInodeT *inode)
{
    if (inode->refCount == 0) {
        return;
    }

    if (--inode->refCount == 0) {
        if (inode->update_flags) {
            Update(inode, inode->update_flags & VMFS5_INODE_SYNC_BLK);
        }

        if (inode->pprev != NULL) {
            if (inode->next != NULL) {
                inode->next->pprev = inode->pprev;
            }
            *(inode->pprev) = inode->next;
            free(inode);
        }
    }
}

/*
 * Get block ID corresponding the specified position. Pointer block
 * resolution is transparently done here.
 */
int VmfsInode::GetBlock(const VmfsInodeT *inode, off_t pos, uint32_t *blkId)
{
    const VmfsFsT *fs = inode->fs;
    u_int blk_index;
    uint32_t zla;
    int vmfs5Ext;

    *blkId = 0;

    if (!inode->blkSize) {
        return (-EIO);
    }

    zla = inode->zla;
    if (zla >= VMFS5_ZLA_BASE) {
        vmfs5Ext = 1;
        zla -= VMFS5_ZLA_BASE;
    } else {
        vmfs5Ext = 0;
    }

    switch (zla) {
        case VMFS5_BLK_TYPE_FB:
        case VMFS5_BLK_TYPE_SB:
            blk_index = pos / inode->blkSize;

            if (blk_index >= VMFS5_INODE_BLK_COUNT) {
                return (-EINVAL);
            }

            *blkId = inode->blocks[blk_index];
            break;

        case VMFS5_BLK_TYPE_PB: {
            DECL_ALIGNED_BUFF_WOL(buf, fs->pbc->bmh.dataSize);
            uint32_t pbBlk_id;
            uint32_t blk_per_pb;
            u_int pb_index;
            u_int sub_index;

            blk_per_pb = fs->pbc->bmh.dataSize / sizeof(uint32_t);
            blk_index = pos / inode->blkSize;

            pb_index = blk_index / blk_per_pb;
            sub_index = blk_index % blk_per_pb;

            if (pb_index >= VMFS5_INODE_BLK_COUNT) {
                return (-EINVAL);
            }

            pbBlk_id = inode->blocks[pb_index];

            if (!pbBlk_id) {
                break;
            }

            if (!VmfsBitmap::Instance()->VmfsBitmapGetItem(fs->pbc, VMFS5_BLK_PB_ENTRY(pbBlk_id),
                VMFS5_BLK_PB_ITEM(pbBlk_id), buf)) {
                return (-EIO);
            }

            *blkId = ReadLE32(buf, sub_index * sizeof(uint32_t));
            break;
        }

        case VMFS5_BLK_TYPE_FD:
            if (vmfs5Ext) {
                *blkId = inode->id;
                break;
            }
        default:
            /* Unexpected ZLA type */
            return (-EIO);
    }

    return (0);
}

/* Get inode status */
int VmfsInode::InodeStat(const VmfsInodeT *inode, struct stat *buf)
{
    int ret = memset_s(buf, sizeof(*buf), 0, sizeof(*buf));
    if (ret != 0) {
        return (-1);
    }
    buf->st_mode = inode->cmode;
    buf->st_nlink = inode->nLink;
    buf->st_uid = inode->uid;
    buf->st_gid = inode->gid;
    buf->st_size = inode->size;
    buf->st_atime = inode->atimev;
    buf->st_mtime = inode->mtime;
    buf->st_ctime = inode->ctime;
    buf->st_blksize = M_BLK_SIZE;
    buf->st_blocks = inode->blkCount * (inode->blkSize / S_BLKSIZE);
    return (0);
}

/* Get inode status */
int VmfsInode::StatFromBlkid(const VmfsFsT *fs, uint32_t blkId, struct stat *buf)
{
    VmfsInodeT *inode;

    if (!(inode = Acquire(fs, blkId))) {
        return (-EIO);
    }

    InodeStat(inode, buf);
    Release(inode);
    return (0);
}

inline uint32_t VmfsInode::ReadBlkid(const u_char *buf, u_int index)
{
    return (ReadLE32(buf, VMFS5_INODE_OFFSET_BLK_ARRAY + (index * sizeof(uint32_t))));
}

inline void VmfsInode::WriteBlkid(u_char *buf, u_int index, uint32_t blkId)
{
    WriteLE32(buf, VMFS5_INODE_OFFSET_BLK_ARRAY + (index * sizeof(uint32_t)), blkId);
}

/* Read an inode */
int VmfsInode::InodeRead(VmfsInodeT *inode, const u_char *buf)
{
    int i;

    VmfsMetadata::Instance()->HdrRead(&inode->mdh, buf);

    if (inode->mdh.magic != VMFS5_INODE_MAGIC) {
        return (-1);
    }

    inode->id = ReadLE32(buf, VMFS5_INODE_OFFSET_ID);
    inode->blkFdItem = ReadLE32(buf, VMFS5_INODE_OFFSET_ID2);
    inode->nLink = ReadLE32(buf, VMFS5_INODE_OFFSET_NLINK);
    inode->type = ReadLE32(buf, VMFS5_INODE_OFFSET_TYPE);
    inode->flags = ReadLE32(buf, VMFS5_INODE_OFFSET_FLAGS);
    inode->size = ReadLE64(buf, VMFS5_INODE_OFFSET_SIZE);
    inode->blkSize = ReadLE64(buf, VMFS5_INODE_OFFSET_BLK_SIZE);
    inode->blkCount = ReadLE64(buf, VMFS5_INODE_OFFSET_BLK_COUNT);
    inode->mtime = ReadLE32(buf, VMFS5_INODE_OFFSET_MTIME);
    inode->ctime = ReadLE32(buf, VMFS5_INODE_OFFSET_CTIME);
    inode->atimev = ReadLE32(buf, VMFS5_INODE_OFFSET_ATIME);
    inode->uid = ReadLE32(buf, VMFS5_INODE_OFFSET_UID);
    inode->gid = ReadLE32(buf, VMFS5_INODE_OFFSET_GID);
    inode->mode = ReadLE32(buf, VMFS5_INODE_OFFSET_MODE);
    inode->zla = ReadLE32(buf, VMFS5_INODE_OFFSET_ZLA);
    inode->tbz = ReadLE32(buf, VMFS5_INODE_OFFSET_TBZ);
    inode->cow = ReadLE32(buf, VMFS5_INODE_OFFSET_COW);

    /* "corrected" mode */
    inode->cmode = inode->mode | VmfsFile::Instance()->VmfsFileType2Mode(inode->type);

    if (inode->type == VMFS5_FTYPE_RDM) {
        inode->rdmId = ReadLE32(buf, VMFS5_INODE_OFFSET_RDM_ID);
    } else if (inode->zla == VMFS5_ZLA_BASE + VMFS5_BLK_TYPE_FD) {
        int ret = memcpy_s(inode->content, inode->size, buf + VMFS5_INODE_OFFSET_CONTENT, inode->size);
        if (ret != 0) {
            return (-1);
        }
    } else {
        for (i = 0; i < VMFS5_INODE_BLK_COUNT; i++) {
            inode->blocks[i] = ReadBlkid(buf, i);
        }
    }

    return (0);
}

/* Write an inode */
int VmfsInode::InodeWrite(const VmfsInodeT *inode, u_char *buf)
{
    VmfsMetadata::Instance()->HdrWrite(&inode->mdh, buf);
    WriteLE32(buf, VMFS5_INODE_OFFSET_ID, inode->id);
    WriteLE32(buf, VMFS5_INODE_OFFSET_ID2, inode->blkFdItem);
    WriteLE32(buf, VMFS5_INODE_OFFSET_NLINK, inode->nLink);
    WriteLE32(buf, VMFS5_INODE_OFFSET_TYPE, inode->type);
    WriteLE32(buf, VMFS5_INODE_OFFSET_FLAGS, inode->flags);
    WriteLE64(buf, VMFS5_INODE_OFFSET_SIZE, inode->size);
    WriteLE64(buf, VMFS5_INODE_OFFSET_BLK_SIZE, inode->blkSize);
    WriteLE64(buf, VMFS5_INODE_OFFSET_BLK_COUNT, inode->blkCount);
    WriteLE32(buf, VMFS5_INODE_OFFSET_MTIME, inode->mtime);
    WriteLE32(buf, VMFS5_INODE_OFFSET_CTIME, inode->ctime);
    WriteLE32(buf, VMFS5_INODE_OFFSET_ATIME, inode->atimev);
    WriteLE32(buf, VMFS5_INODE_OFFSET_UID, inode->uid);
    WriteLE32(buf, VMFS5_INODE_OFFSET_GID, inode->gid);
    WriteLE32(buf, VMFS5_INODE_OFFSET_MODE, inode->mode);
    WriteLE32(buf, VMFS5_INODE_OFFSET_ZLA, inode->zla);
    WriteLE32(buf, VMFS5_INODE_OFFSET_TBZ, inode->tbz);
    WriteLE32(buf, VMFS5_INODE_OFFSET_COW, inode->cow);
    return (0);
}

/* Update block list */
void VmfsInode::WriteBlkList(const VmfsInodeT *inode, u_char *buf)
{
    int i;

    for (i = 0; i < VMFS5_INODE_BLK_COUNT; i++) {
        WriteBlkid(buf, i, inode->blocks[i]);
    }
}

/* Hash function to retrieve an in-core inode */
inline u_int VmfsInode::InodeHash(const VmfsFsT *fs, uint32_t blkId)
{
    return ((blkId ^ (blkId >> NUM_NINE)) & (fs->inodeHashBuckets - 1));
}

/* Register an inode in the in-core inode hash table */
void VmfsInode::Register(const VmfsFsT *fs, VmfsInodeT *inode)
{
    u_int ihs;

    ihs = InodeHash(fs, inode->id);

    inode->fs = fs;
    inode->refCount = 1;

    inode->next = fs->inodes[ihs];
    inode->pprev = &fs->inodes[ihs];

    if (inode->next != NULL) {
        inode->next->pprev = &inode->next;
    }

    fs->inodes[ihs] = inode;
}
}