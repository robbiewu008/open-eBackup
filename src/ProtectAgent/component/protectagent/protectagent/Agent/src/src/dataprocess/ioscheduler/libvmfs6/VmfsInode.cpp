#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <sys/stat.h>
#include "dataprocess/ioscheduler/libvmfs6/Vmfs.h"


namespace Vmfs6IO {
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
    V6_DECL_ALIGNED_BUFFER(buf, VMFS6_INODE_SIZE);

    int ret = memset_s(buf, VMFS6_INODE_SIZE, 0, VMFS6_INODE_SIZE);
    if (ret != 0) {
        return (-1);
    }
    InodeWrite(inode, buf);

    if (update_blk_list) {
        WriteBlkList(inode, buf);
    } else {
        buf_len -= VMFS6_INODE_BLK_COUNT * sizeof(uint32_t);
    }

    if (VmfsDeviceWrite(inode->fs->dev, inode->mdh.pos, buf, buf_len) != buf_len) {
        return (-1);
    }

    return (0);
}

/* Get inode corresponding to a block id */
int VmfsInode::GetInodeId(const VmfsFsT *fs, uint64_t blkId, VmfsInodeT *inode)
{
    V6_DECL_ALIGNED_BUFF_WOL(buf, VMFS6_INODE_SIZE);

    if (VMFS6_BLK_TYPE(blkId) != VMFS6_BLK_TYPE_FD) {
        return (-1);
    }

    if (!VmfsBitmap::Instance()->VmfsBitmapGetItem(fs->fdc, VMFS6_BLK_FD_ENTRY(blkId), VMFS6_BLK_FD_ITEM(blkId), buf)) {
        return (-1);
    }

    return (InodeRead(inode, buf));
}

/* Acquire an inode */
VmfsInodeT *VmfsInode::Acquire(const VmfsFsT *fs, uint64_t blkId)
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
            Update(inode, inode->update_flags & VMFS6_INODE_SYNC_BLK);
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
int VmfsInode::GetBlock(const VmfsInodeT *inode, off_t pos, uint64_t *blkId)
{
    const VmfsFsT *fs = inode->fs;
    u_int blk_index;
    uint32_t zla;
    int vmfs6Ext;

    *blkId = 0;

    if (!inode->blkSize) {
        return (-EIO);
    }

    zla = inode->zla;
    if (zla >= VMFS6_ZLA_BASE) {
        vmfs6Ext = 1;
        zla -= VMFS6_ZLA_BASE;
    } else {
        vmfs6Ext = 0;
    }

    switch (zla) {
        case VMFS6_BLK_TYPE_FB:
        case VMFS6_BLK_TYPE_SB:
            blk_index = pos / inode->blkSize;

            if (blk_index >= VMFS6_INODE_BLK_COUNT) {
                return (-EINVAL);
            }

            *blkId = inode->blocks[blk_index];
            break;

        case VMFS6_BLK_TYPE_PB2: {
            V6_DECL_ALIGNED_BUFF_WOL(buf, fs->pb2->bmh.dataSize);
            uint64_t pb_blk_id;
            uint32_t blkPerPb;
            u_int pbIndex;
            u_int subIndex;

            blkPerPb = fs->pb2->bmh.dataSize / sizeof(uint64_t);
            blk_index = pos / inode->blkSize;

            pbIndex = blk_index / blkPerPb;
            subIndex = blk_index % blkPerPb;

            if (pbIndex >= VMFS6_INODE_BLK_COUNT) {
                return (-EINVAL);
            }

            pb_blk_id = inode->blocks[pbIndex];

            if (!pb_blk_id) {
                break;
            }
            if (!VmfsBitmap::Instance()->VmfsBitmapGetItem(fs->pb2, VMFS6_BLK_PB2_ENTRY(pb_blk_id),
                VMFS6_BLK_PB2_ITEM(pb_blk_id), buf)) {
                return (-EIO);
            }

            *blkId = ReadLE64(buf, subIndex * sizeof(uint64_t));
            break;
        }

        case VMFS6_BLK_TYPE_PB: {
            if (vmfs6Ext) {
                int err;
                uint64_t blkIdTmp;

                // Double Indirect Addressing
                if ((err = DoubleIndirectAddr(inode, pos, &blkIdTmp)) < 0) {
                    COMMLOG(OS_LOG_ERROR, "vmfsio - Get block failed 0x%lx", blkIdTmp);
                    return (err);
                }
                *blkId = blkIdTmp;
                COMMLOG(OS_LOG_DEBUG, "vmfsio - PB blkId 0x%lx", *blkId);
            } else {
                V6_DECL_ALIGNED_BUFF_WOL(buf, fs->pbc->bmh.dataSize);
                uint64_t pbBlkId;
                uint32_t blkPerPb;
                u_int pbIndex;
                u_int subIndex;

                blkPerPb = fs->pbc->bmh.dataSize / sizeof(uint64_t);
                blk_index = pos / inode->blkSize;

                pbIndex = blk_index / blkPerPb;
                subIndex = blk_index % blkPerPb;

                if (pbIndex >= VMFS6_INODE_BLK_COUNT) {
                    return (-EINVAL);
                }

                pbBlkId = inode->blocks[pbIndex];

                if (!pbBlkId) {
                    break;
                }

                if (!VmfsBitmap::Instance()->VmfsBitmapGetItem(fs->sbc, VMFS6_BLK_SB_ENTRY(pbBlkId),
                    VMFS6_BLK_SB_ITEM(pbBlkId), buf)) {
                    return (-EIO);
                }

                *blkId = ReadLE64(buf, subIndex * sizeof(uint64_t));
            }
            break;
        }

        case VMFS6_BLK_TYPE_FD:
            if (vmfs6Ext) {
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
    buf->st_blksize = VMFS6_M_BLK_SIZE;
    buf->st_blocks = inode->blkCount * (inode->blkSize / S_BLKSIZE);
    return (0);
}

/* Get inode status */
int VmfsInode::StatFromBlkid(const VmfsFsT *fs, uint64_t blkId, struct stat *buf)
{
    VmfsInodeT *inode;

    if (!(inode = Acquire(fs, blkId))) {
        return (-EIO);
    }

    InodeStat(inode, buf);
    Release(inode);
    return (0);
}

inline uint64_t VmfsInode::ReadBlkid(const u_char *buf, u_int index)
{
    return (ReadLE64(buf, VMFS6_INODE_OFFSET_BLK_ARRAY + (index * sizeof(uint64_t))));
}

inline void VmfsInode::WriteBlkid(u_char *buf, u_int index, uint64_t blkId)
{
    WriteLE64(buf, VMFS6_INODE_OFFSET_BLK_ARRAY + (index * sizeof(uint64_t)), blkId);
}

/* Read inode */
int VmfsInode::InodeRead(VmfsInodeT *inode, const u_char *buf)
{
    int i;
    int res;

    VmfsMetadata::Instance()->HdrRead(&inode->mdh, buf);

    if (inode->mdh.magic != VMFS6_INODE_MAGIC) {
        return (-1);
    }

    inode->id = ReadLE32(buf, VMFS6_INODE_OFFSET_ID);
    inode->blkFdItem = ReadLE32(buf, VMFS6_INODE_OFFSET_ID2);
    inode->nLink = ReadLE32(buf, VMFS6_INODE_OFFSET_NLINK);
    inode->type = ReadLE32(buf, VMFS6_INODE_OFFSET_TYPE);
    inode->flags = ReadLE32(buf, VMFS6_INODE_OFFSET_FLAGS);
    inode->size = ReadLE64(buf, VMFS6_INODE_OFFSET_SIZE);
    inode->blkSize = ReadLE64(buf, VMFS6_INODE_OFFSET_BLK_SIZE);
    inode->blkCount = ReadLE64(buf, VMFS6_INODE_OFFSET_BLK_COUNT);
    inode->mtime = ReadLE32(buf, VMFS6_INODE_OFFSET_MTIME);
    inode->ctime = ReadLE32(buf, VMFS6_INODE_OFFSET_CTIME);
    inode->atimev = ReadLE32(buf, VMFS6_INODE_OFFSET_ATIME);
    inode->uid = ReadLE32(buf, VMFS6_INODE_OFFSET_UID);
    inode->gid = ReadLE32(buf, VMFS6_INODE_OFFSET_GID);
    inode->mode = ReadLE32(buf, VMFS6_INODE_OFFSET_MODE);
    inode->zla = ReadLE32(buf, VMFS6_INODE_OFFSET_ZLA);
    inode->tbz = ReadLE32(buf, VMFS6_INODE_OFFSET_TBZ);
    inode->cow = ReadLE32(buf, VMFS6_INODE_OFFSET_COW);

    /* "corrected" mode */
    res = (inode->mode) & S_IFMT;
    if (res == S_IFDIR) {
        inode->cmode = inode->mode;
    } else {
        inode->cmode = inode->mode | VmfsFile::Instance()->VmfsFileType2Mode(inode->type);
    }

    if (inode->type == VMFS6_FTYPE_RDM) {
        inode->rdmId = ReadLE32(buf, VMFS6_INODE_OFFSET_RDM_ID);
    } else if (inode->zla == VMFS6_ZLA_BASE + VMFS6_BLK_TYPE_FD) {
        int ret = memcpy_s(inode->content, inode->size, buf + VMFS6_INODE_OFFSET_CONTENT, inode->size);
        if (ret != 0) {
            return (-1);
        }
    } else {
        COMMLOG(OS_LOG_DEBUG, "vmfsio - inode id: %d, off: %ld, inode blocks:",
            inode->id, VMFS6_INODE_OFFSET_BLK_ARRAY);
        for (i = 0; i < VMFS6_INODE_BLK_COUNT; i++) {
            inode->blocks[i] = ReadBlkid(buf, i);
            if (inode->blocks[i] == 0) {
                break;
            }
            COMMLOG(OS_LOG_DEBUG, "  blocks - %d:%016lx", i, inode->blocks[i]);
        }
    }

    return (0);
}

/* Write an inode */
int VmfsInode::InodeWrite(const VmfsInodeT *inode, u_char *buf)
{
    VmfsMetadata::Instance()->HdrWrite(&inode->mdh, buf);
    WriteLE32(buf, VMFS6_INODE_OFFSET_ID, inode->id);
    WriteLE32(buf, VMFS6_INODE_OFFSET_ID2, inode->blkFdItem);
    WriteLE32(buf, VMFS6_INODE_OFFSET_NLINK, inode->nLink);
    WriteLE32(buf, VMFS6_INODE_OFFSET_TYPE, inode->type);
    WriteLE32(buf, VMFS6_INODE_OFFSET_FLAGS, inode->flags);
    WriteLE64(buf, VMFS6_INODE_OFFSET_SIZE, inode->size);
    WriteLE64(buf, VMFS6_INODE_OFFSET_BLK_SIZE, inode->blkSize);
    WriteLE64(buf, VMFS6_INODE_OFFSET_BLK_COUNT, inode->blkCount);
    WriteLE32(buf, VMFS6_INODE_OFFSET_MTIME, inode->mtime);
    WriteLE32(buf, VMFS6_INODE_OFFSET_CTIME, inode->ctime);
    WriteLE32(buf, VMFS6_INODE_OFFSET_ATIME, inode->atimev);
    WriteLE32(buf, VMFS6_INODE_OFFSET_UID, inode->uid);
    WriteLE32(buf, VMFS6_INODE_OFFSET_GID, inode->gid);
    WriteLE32(buf, VMFS6_INODE_OFFSET_MODE, inode->mode);
    WriteLE32(buf, VMFS6_INODE_OFFSET_ZLA, inode->zla);
    WriteLE32(buf, VMFS6_INODE_OFFSET_TBZ, inode->tbz);
    WriteLE32(buf, VMFS6_INODE_OFFSET_COW, inode->cow);
    return (0);
}

/* Update block list */
void VmfsInode::WriteBlkList(const VmfsInodeT *inode, u_char *buf)
{
    int i;

    for (i = 0; i < VMFS6_INODE_BLK_COUNT; i++) {
        WriteBlkid(buf, i, inode->blocks[i]);
    }
}

/* Hash function to retrieve an in-core inode */
inline u_int VmfsInode::InodeHash(const VmfsFsT *fs, uint64_t blkId)
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

/* Get block ID corresponding the specified position. Double Indirecting Addressing, Pointer block */
int VmfsInode::DoubleIndirectAddr(const VmfsInodeT *inode, off_t pos, uint64_t *blkId)
{
    const VmfsFsT *fs = inode->fs;
    V6_DECL_ALIGNED_BUFF_WOL(buf, fs->sbc->bmh.dataSize);

    u_int blkIndex;
    uint32_t blkPerExtendedPb;
    uint32_t blkperprimpb;
    uint32_t blkPerSecondaryPb;

    uint64_t primPbBlkId;
    uint64_t secndPbPlkId;

    u_int primPbIndex;
    u_int primSubIndex;
    u_int secndPbIndex;
    u_int secndSubIndex;

    blkIndex = pos / inode->blkSize;
    COMMLOG(OS_LOG_DEBUG, "vmfsio - blkIndex: %d = pos/inode->blkSize: (%ld/%ld)", blkIndex, pos, inode->blkSize);

    blkperprimpb = fs->sbc->bmh.dataSize / sizeof(uint64_t);   // 8192
    blkPerSecondaryPb = fs->sbc->bmh.dataSize / sizeof(uint64_t); // 8192
    blkPerExtendedPb = blkperprimpb * blkPerSecondaryPb;        // 64m
    COMMLOG(OS_LOG_DEBUG, "vmfsio - blkPerExtendedPb: %d\n", blkPerExtendedPb);

    primPbIndex = blkIndex / blkPerExtendedPb;
    primSubIndex = blkIndex % blkPerExtendedPb;
    COMMLOG(OS_LOG_DEBUG, "vmfsio - primPbIndex: %d, primSubIndex: %d", primPbIndex, primSubIndex);

    secndPbIndex = primSubIndex / blkPerSecondaryPb;
    secndSubIndex = primSubIndex % blkPerSecondaryPb;
    COMMLOG(OS_LOG_DEBUG, "vmfsio - secndPbIndex: %d, secndSubIndex: %d", secndPbIndex, secndSubIndex);

    if (primPbIndex >= VMFS6_INODE_BLK_COUNT) {
        return (-EINVAL);
    }

    primPbBlkId = inode->blocks[primPbIndex];
    COMMLOG(OS_LOG_DEBUG, "vmfsio - primPbBlkId: 0x%lx", primPbBlkId);

    if (!primPbBlkId) {
        return (-EINVAL);
    }

    if (!VmfsBitmap::Instance()->VmfsBitmapGetItem(fs->sbc, VMFS6_BLK_SB_ENTRY(primPbBlkId),
        VMFS6_BLK_SB_ITEM(primPbBlkId), buf)) {
        return (-EIO);
    }
    COMMLOG(OS_LOG_DEBUG, "vmfsio - primPbBlkId: 0x%lx", primPbBlkId);

    secndPbPlkId = ReadLE64(buf, secndPbIndex * sizeof(uint64_t));
    COMMLOG(OS_LOG_DEBUG, "vmfsio - secndPbPlkId: 0x%lx, secndPbIndex: %d", secndPbPlkId, secndPbIndex);

    if (!VmfsBitmap::Instance()->VmfsBitmapGetItem(fs->sbc, VMFS6_BLK_SB_ENTRY(secndPbPlkId),
        VMFS6_BLK_SB_ITEM(secndPbPlkId), buf)) {
        return (-EIO);
    }

    *blkId = ReadLE64(buf, secndSubIndex * sizeof(uint64_t));
    COMMLOG(OS_LOG_DEBUG, "vmfsio - blkId: 0x%lx, secndSubIndex: %d", *blkId, secndSubIndex);

    return (0);
}
}
