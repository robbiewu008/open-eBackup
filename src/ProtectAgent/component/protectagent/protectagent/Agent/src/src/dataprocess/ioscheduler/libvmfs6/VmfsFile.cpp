#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "dataprocess/ioscheduler/libvmfs6/Vmfs.h"


namespace Vmfs6IO {
VmfsFile *VmfsFile::m_instance = nullptr;
std::mutex VmfsFile::m_mutex;

VmfsFile *VmfsFile::Instance()
{
    if (m_instance == nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_instance == nullptr) {
            m_instance = new (std::nothrow) VmfsFile();
        }
    }
    return m_instance;
}

/* Open a file based on an inode buffer */
VmfsFileT *VmfsFile::OpenFromInode(const VmfsInodeT *inode)
{
    VmfsFileT *f = nullptr;
    if (!(f = (VmfsFileT *)calloc(1, sizeof(*f)))) {
        return NULL;
    }
    f->inode = (VmfsInodeT *)inode;
    return f;
}

/* Open a file based on a directory entry */
VmfsFileT *VmfsFile::OpenFromBlkid(const VmfsFsT *fs, uint64_t blkId)
{
    VmfsInodeT *inode = nullptr;
    if (!(inode = VmfsInode::Instance()->Acquire(fs, blkId))) {
        return NULL;
    }
    return (OpenFromInode(inode));
}

/* Open a file */
VmfsFileT *VmfsFile::OpenAt(VmfsDirT *dir, const char *path)
{
    uint64_t blkId = 0;
    if (!(blkId = VmfsDirent::Instance()->VmfsDirResolvePath(dir, path, 1))) {
        return (NULL);
    }
    return (VmfsFile::Instance()->OpenFromBlkid(VmfsDirent::Instance()->VmfsDirGetFs(dir), blkId));
}

/* Close a file */
int VmfsFile::Close(VmfsFileT *f)
{
    if (f == NULL) {
        return (-1);
    }

    if (f->flags & VMFS6_FFLAG_FD) {
        close(f->fd);
    } else {
        VmfsInode::Instance()->Release(f->inode);
    }

    free(f);
    return (0);
}

/* Read data from a file at the specified position */
ssize_t VmfsFile::Read(VmfsFileT *f, u_char *buf, size_t len, off_t pos)
{
    COMMLOG(OS_LOG_DEBUG, "");
    const VmfsFsT *fs = VmfsFileGetFs(f);
    uint64_t blkId;
    uint32_t blk_type;
    uint64_t blkSize;
    uint64_t blk_len;
    uint64_t file_size;
    uint64_t offset;
    ssize_t res = 0;
    ssize_t rlen = 0;
    size_t exp_len;
    int err;

    if (f->flags & VMFS6_FFLAG_FD) {
        return pread(f->fd, buf, len, pos);
    }

    /* We don't handle RDM files */
    if (f->inode->type == VMFS6_FTYPE_RDM) {
        return (-EIO);
    }

    blkSize = VmfsFs::Instance()->GetBlockSize(fs);
    file_size = VmfsFileGetSize(f);

    while (len > 0) {
        if (pos >= file_size) {
            break;
        }

        if ((err = VmfsInode::Instance()->GetBlock(f->inode, pos, &blkId)) < 0) {
            return (err);
        }

        blk_type = VMFS6_BLK_FB_TBZ(blkId) ? VMFS6_BLK_TYPE_NONE : VMFS6_BLK_TYPE(blkId);
        switch (blk_type) {
            /* Unallocated block */
            case VMFS6_BLK_TYPE_NONE: {
                offset = pos % blkSize;
                blk_len = blkSize - offset;
                exp_len = M_MIN(blk_len, len);
                res = M_MIN(exp_len, file_size - pos);
                int ret = memset_s(buf, res, 0, res);
                if (ret != 0) {
                    return (-EIO);
                }
                break;
            }
            /* File-Block */
            case VMFS6_BLK_TYPE_FB:
            case VMFS6_BLK_TYPE_PB2: {
                exp_len = M_MIN(len, file_size - pos);
                res = VmfsBlock::Instance()->VmfsBlockReadFb(fs, blkId, pos, buf, exp_len);
                break;
            }
            /* Sub-Block */
            case VMFS6_BLK_TYPE_SB: {
                exp_len = M_MIN(len, file_size - pos);
                res = VmfsBlock::Instance()->VmfsBlockReadSb(fs, blkId, pos, buf, exp_len);
                break;
            }
            /* Large File-Block */
            case VMFS6_BLK_TYPE_LFB: {
                exp_len = M_MIN(len, file_size - pos);
                res = VmfsBlock::Instance()->VmfsBlockReadLfb(fs, blkId, pos, buf, exp_len);
                break;
            }
            /* Inline in the inode */
            case VMFS6_BLK_TYPE_FD: {
                if (blkId == f->inode->id) {
                    exp_len = M_MIN(len, file_size - pos);
                    int ret = memcpy_s(buf, exp_len, f->inode->content + pos, exp_len);
                    if (ret != 0) {
                        return (-EIO);
                    }
                    res = exp_len;
                    break;
                }
            }
            default: {
                COMMLOG(OS_LOG_ERROR, "vmfsio - unknown block type 0x%2.2x\n", blk_type);
                return (-EIO);
            }
        }

        if (res < 0) {
            COMMLOG(OS_LOG_ERROR, "vmfsio - read failed. res=%ld", res);
            return (res);
        }

        pos += res;
        rlen += res;
        buf += res;
        len -= res;
    }

    return (rlen);
}
}
