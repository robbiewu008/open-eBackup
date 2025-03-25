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
 * VMFS bitmaps.
 */

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include "dataprocess/ioscheduler/libvmfs/Vmfs.h"


namespace Vmfs5IO {
VmfsBitmap *VmfsBitmap::m_instance = nullptr;
std::mutex VmfsBitmap::m_mutex;

VmfsBitmap *VmfsBitmap::Instance()
{
    if (m_instance == nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_instance == nullptr) {
            m_instance = new (std::nothrow) VmfsBitmap();
        }
    }
    return m_instance;
}

/* Read a bitmap header */
int VmfsBitmap::VmfsBMHRead(VmfsBitmapHeaderT *bmh, const u_char *buf)
{
    bmh->itemsPerBitmapEntry = ReadLE32(buf, 0x0);
    bmh->bmpEntriesPerArea = ReadLE32(buf, 0x4);
    bmh->hdrSize = ReadLE32(buf, 0x8);
    bmh->dataSize = ReadLE32(buf, 0xc);
    bmh->areaSize = ReadLE32(buf, 0x10);
    bmh->totalItems = ReadLE32(buf, 0x14);
    bmh->areaCount = ReadLE32(buf, 0x18);
    return (0);
}

/* Read a bitmap entry */
int VmfsBitmap::VmfsBMERead(VmfsBitmapEntryT *bme, const u_char *buf, int copy_bitmap)
{
    VmfsMetadata::Instance()->HdrRead(&bme->mdh, buf);
    bme->id = ReadLE32(buf, VMFS5_BME_OFFSET_ID);
    bme->total = ReadLE32(buf, VMFS5_BME_OFFSET_TOTAL);
    bme->free = ReadLE32(buf, VMFS5_BME_OFFSET_FREE);
    bme->ffree = ReadLE32(buf, VMFS5_BME_OFFSET_FFREE);

    if (copy_bitmap) {
        int ret = memcpy_s(bme->bitmap, (bme->total + NUM_SEVEN) / NUM_EIGHT, &buf[VMFS5_BME_OFFSET_BITMAP],
            (bme->total + NUM_SEVEN) / NUM_EIGHT);
        if (ret != 0) {
            return (-1);
        }
    }

    return (0);
}

/* Get number of items per area */
u_int VmfsBitmap::VmfsBitmapGetIteamsPerArea(const VmfsBitmapHeaderT *bmh)
{
    return (bmh->bmpEntriesPerArea * bmh->itemsPerBitmapEntry);
}

/* Get address of a given area (pointing to bitmap array) */
off_t VmfsBitmap::VmfsBitmapGetAreaAddr(const VmfsBitmapHeaderT *bmh, u_int area)
{
    return (bmh->hdrSize + (area * bmh->areaSize));
}

/* Get position of an item */
off_t VmfsBitmap::VmfsBitmapGetItemPos(VmfsBitmapT *b, uint32_t entry, uint32_t item)
{
    off_t pos;
    uint32_t addr;
    uint32_t items_per_area;
    u_int area;

    addr = (entry * b->bmh.itemsPerBitmapEntry) + item;

    items_per_area = VmfsBitmapGetIteamsPerArea(&b->bmh);
    area = addr / items_per_area;

    pos = b->bmh.hdrSize + (area * b->bmh.areaSize);
    pos += b->bmh.bmpEntriesPerArea * VMFS5_BITMAP_ENTRY_SIZE;
    pos += (addr % items_per_area) * b->bmh.dataSize;

    return (pos);
}

/* Read a bitmap item from its entry and item numbers */
bool VmfsBitmap::VmfsBitmapGetItem(VmfsBitmapT *b, uint32_t entry, uint32_t item, u_char *buf)
{
    off_t pos = VmfsBitmapGetItemPos(b, entry, item);
    return (VmfsFile::Instance()->Read(b->f, buf, b->bmh.dataSize, pos) == b->bmh.dataSize);
}

/* Get offset of an item in a bitmap entry */
void VmfsBitmap::VmfsBitmapGetIteamOffset(const VmfsBitmapHeaderT *bmh, u_int addr, u_int *array_idx, u_int *bit_idx)
{
    u_int idx;

    idx = addr % bmh->itemsPerBitmapEntry;
    *array_idx = idx >> NUM_THREE;
    *bit_idx = idx & 0x07;
}

/* Update the first free item field */
void VmfsBitmap::VmfsBitmapUpdateFFree(VmfsBitmapEntryT *entry)
{
    u_int array_idx;
    u_int bit_idx;
    int i;

    entry->ffree = 0;

    for (i = 0; i < entry->total; i++) {
        array_idx = i >> NUM_THREE;
        bit_idx = i & 0x07;

        if (entry->bitmap[array_idx] & (1 << bit_idx)) {
            entry->ffree = i;
            break;
        }
    }
}

/* Count the total number of allocated items in a bitmap area */
uint32_t VmfsBitmap::VmfsBitmapAreaAllocatedItems(VmfsBitmapT *b, u_int area)
{
    u_char buf[VMFS5_BITMAP_ENTRY_SIZE];
    VmfsBitmapEntryT entry;
    uint32_t count;
    off_t pos;
    int i;

    pos = VmfsBitmapGetAreaAddr(&b->bmh, area);

    for (i = 0, count = 0; i < b->bmh.bmpEntriesPerArea; i++) {
        if (VmfsFile::Instance()->Read(b->f, buf, sizeof(buf), pos) != sizeof(buf)) {
            break;
        }

        VmfsBMERead(&entry, buf, 0);
        count += entry.total - entry.free;
        pos += sizeof(buf);
    }

    return count;
}

/* Call a user function for each allocated item in a bitmap */
void VmfsBitmap::VmfsBitmapAreaForeach(VmfsBitmapT *b, u_int area, VmfsBitmapForeachCbkT cbk, void *optArg)
{
    DECL_ALIGNED_BUFFER(buf, VMFS5_BITMAP_ENTRY_SIZE);
    VmfsBitmapEntryT entry;
    off_t pos;
    uint32_t addr;
    u_int array_idx;
    u_int bit_idx;
    u_int i;
    u_int j;

    pos = VmfsBitmapGetAreaAddr(&b->bmh, area);

    for (i = 0; i < b->bmh.bmpEntriesPerArea; i++) {
        if (VmfsFile::Instance()->Read(b->f, buf, buf_len, pos) != buf_len) {
            break;
        }

        VmfsBMERead(&entry, buf, 1);

        for (j = 0; j < entry.total; j++) {
            array_idx = j >> NUM_THREE;
            bit_idx = j & 0x07;

            addr = area * VmfsBitmapGetIteamsPerArea(&b->bmh);
            addr += i * b->bmh.itemsPerBitmapEntry;
            addr += j;

            if (!(entry.bitmap[array_idx] & (1 << bit_idx))) {
                cbk(b, addr, optArg);
            }
        }

        pos += buf_len;
    }
}

/* Open a bitmap file */
inline VmfsBitmapT *VmfsBitmap::VmfsBitmapOpenFromFile(VmfsFileT *f)
{
    DECL_ALIGNED_BUFFER(buf, NUM_512);
    VmfsBitmapT *b;

    if (!f) {
        return NULL;
    }

    if (VmfsFile::Instance()->Read(f, buf, buf_len, 0) != buf_len) {
        VmfsFile::Instance()->Close(f);
        return NULL;
    }

    if (!(b = (VmfsBitmapT *)calloc(1, sizeof(VmfsBitmapT)))) {
        VmfsFile::Instance()->Close(f);
        return NULL;
    }

    VmfsBMHRead(&b->bmh, buf);
    b->f = f;
    return b;
}

VmfsBitmapT *VmfsBitmap::VmfsBitmapOpenAt(VmfsDirT *d, const char *name)
{
    return VmfsBitmapOpenFromFile(VmfsFile::Instance()->OpenAt(d, name));
}

VmfsBitmapT *VmfsBitmap::VmfsBitmapOpenFromInode(const VmfsInodeT *inode)
{
    return VmfsBitmapOpenFromFile(VmfsFile::Instance()->OpenFromInode(inode));
}

/* Close a bitmap file */
void VmfsBitmap::VmfsBitmapClose(VmfsBitmapT *b)
{
    if (b != NULL) {
        VmfsFile::Instance()->Close(b->f);
        free(b);
    }
}
}