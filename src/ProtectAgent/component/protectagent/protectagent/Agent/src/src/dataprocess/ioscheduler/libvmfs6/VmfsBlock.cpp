#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include "dataprocess/ioscheduler/libvmfs6/Vmfs.h"


namespace Vmfs6IO {
VmfsBlock *VmfsBlock::m_instance = nullptr;
std::mutex VmfsBlock::m_mutex;

VmfsBlock *VmfsBlock::Instance()
{
    if (m_instance == nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_instance == nullptr) {
            m_instance = new (std::nothrow) VmfsBlock();
        }
    }
    return m_instance;
}

/* Read a piece of a sub-block */
ssize_t VmfsBlock::VmfsBlockReadSb(const VmfsFsT *fs, uint64_t blkId, off_t pos, u_char *buf, size_t len)
{
    V6_DECL_ALIGNED_BUFF_WOL(tmpbuf, fs->sbc->bmh.dataSize);
    uint32_t offset;
    uint32_t sbc_entry;
    uint32_t sbc_item;
    size_t clen;

    offset = pos % fs->sbc->bmh.dataSize;
    clen = M_MIN(fs->sbc->bmh.dataSize - offset, len);

    sbc_entry = VMFS6_BLK_SB_ENTRY(blkId);
    sbc_item = VMFS6_BLK_SB_ITEM(blkId);
    if (!VmfsBitmap::Instance()->VmfsBitmapGetItem(fs->sbc, sbc_entry, sbc_item, tmpbuf)) {
        return (-EIO);
    }

    int ret = memcpy_s(buf, clen, tmpbuf + offset, clen);
    if (ret != 0) {
        return (-1);
    }
    return (clen);
}

/* Read a piece of a file block */
ssize_t VmfsBlock::VmfsBlockReadFb(const VmfsFsT *fs, uint64_t blkId, off_t pos, u_char *buf, size_t len)
{
    uint64_t offset;
    uint64_t n_offset;
    uint64_t blkSize;
    size_t clen;
    size_t n_clen;
    uint32_t fb_item;
    u_char *tmpbuf;

    blkSize = VmfsFs::Instance()->GetBlockSize(fs);

    offset = pos % blkSize;
    clen = M_MIN(blkSize - offset, len);

    /* Use "normalized" offset / length to access data (for direct I/O) */
    n_offset = offset & ~(V6_M_DIO_BLK_SIZE - 1);
    n_clen = V6_ALIGN_NUMBER(clen + (offset - n_offset), V6_M_DIO_BLK_SIZE);

    fb_item = VMFS6_BLK_FB_ITEM(blkId);

    /* If everything is aligned for direct I/O, store directly in user buffer */
    if ((n_offset == offset) && (n_clen == clen) && V6_ALIGN_CHECK((uintptr_t)buf, V6_M_DIO_BLK_SIZE)) {
        if (VmfsFs::Instance()->ReadBlock(fs, fb_item, n_offset, buf, n_clen) != n_clen) {
            return (-EIO);
        }
        return (n_clen);
    }

    /* Allocate a temporary buffer and copy result to user buffer */
    if (!(tmpbuf = IOBufferAlloc(n_clen))) {
        return (-1);
    }

    if (VmfsFs::Instance()->ReadBlock(fs, fb_item, n_offset, tmpbuf, n_clen) != n_clen) {
        IOBufferFree(tmpbuf);
        return (-EIO);
    }

    int ret = memcpy_s(buf, clen, tmpbuf + (offset - n_offset), clen);
    if (ret != 0) {
        IOBufferFree(tmpbuf);
        return (-1);
    }

    IOBufferFree(tmpbuf);
    return (clen);
}

ssize_t VmfsBlock::VmfsBlockReadLfb(const VmfsFsT *fs, uint64_t blkId, off_t pos, u_char *buf, size_t len)
{
    uint64_t offset;
    uint64_t n_offset;
    uint64_t largeBlkSize;
    size_t clen;
    size_t n_clen;
    uint32_t fb_item;
    u_char *tmpbuf;

    largeBlkSize = LARGE_BLOCK_SIZE;
    offset = pos % largeBlkSize;
    clen = M_MIN(largeBlkSize - offset, len);

    /* Use "normalized" offset / length to access data (for direct I/O) */
    n_offset = offset & ~(V6_M_DIO_BLK_SIZE - 1);
    n_clen = V6_ALIGN_NUMBER(clen + (offset - n_offset), V6_M_DIO_BLK_SIZE);
    fb_item = VMFS6_BLK_FB_ITEM(blkId);

    /* If everything is aligned for direct I/O, store directly in user buffer */
    if ((n_offset == offset) && (n_clen == clen) && V6_ALIGN_CHECK((uintptr_t)buf, V6_M_DIO_BLK_SIZE)) {
        if (VmfsFs::Instance()->ReadBlock(fs, fb_item, n_offset, buf, n_clen) != n_clen) {
            return (-EIO);
        }
        return (n_clen);
    }

    /* Allocate a temporary buffer and copy result to user buffer */
    if (!(tmpbuf = IOBufferAlloc(n_clen))) {
        return (-1);
    }
    if (VmfsFs::Instance()->ReadBlock(fs, fb_item, n_offset, tmpbuf, n_clen) != n_clen) {
        IOBufferFree(tmpbuf);
        return (-EIO);
    }

    memcpy_s(buf, clen, tmpbuf + (offset - n_offset), clen);
    IOBufferFree(tmpbuf);
    return (clen);
}
}