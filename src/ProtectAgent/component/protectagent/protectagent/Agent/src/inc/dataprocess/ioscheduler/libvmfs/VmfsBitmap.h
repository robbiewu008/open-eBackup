/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#ifndef __VMFS5_BITMAP_H__
#define __VMFS5_BITMAP_H__

#include <stdbool.h>
#include <mutex>


namespace Vmfs5IO {
struct VmfsBitmapHeaderS {
    uint32_t itemsPerBitmapEntry;
    uint32_t bmpEntriesPerArea;
    uint32_t hdrSize;
    uint32_t dataSize;
    uint32_t areaSize;
    uint32_t totalItems;
    uint32_t areaCount;
};
using VmfsBitmapHeaderT = struct VmfsBitmapHeaderS;

/* === Bitmap entry === */
#define VMFS5_BITMAP_ENTRY_SIZE 0x400

#define VMFS5_BITMAP_BMP_MAX_SIZE 0x1f0

struct VmfsBitmapEntryPlain {
    struct VmfsMetadataHdrRaw mdh; /* Metadata header */
    uint32_t id;                   /* Bitmap ID */
    uint32_t total;                /* Total number of items in this entry */
    uint32_t free;                 /* Free items */
    uint32_t ffree;                /* First free item */
    uint8_t bitmap[VMFS5_BITMAP_BMP_MAX_SIZE];
} __attribute__((packed));

#define VMFS5_BME_OFFSET_ID offsetof(struct VmfsBitmapEntryPlain, id)
#define VMFS5_BME_OFFSET_TOTAL offsetof(struct VmfsBitmapEntryPlain, total)
#define VMFS5_BME_OFFSET_FREE offsetof(struct VmfsBitmapEntryPlain, free)
#define VMFS5_BME_OFFSET_FFREE offsetof(struct VmfsBitmapEntryPlain, ffree)
#define VMFS5_BME_OFFSET_BITMAP offsetof(struct VmfsBitmapEntryPlain, bitmap)

struct VmfsBitmapEntryS {
    VmfsMetadataHdrT mdh;
    uint32_t id;
    uint32_t total;
    uint32_t free;
    uint32_t ffree;
    uint8_t bitmap[VMFS5_BITMAP_BMP_MAX_SIZE];
};
using VmfsBitmapEntryT = struct VmfsBitmapEntryS;

/* A bitmap file instance */
struct VmfsBitmapS {
    VmfsFileT *f;
    VmfsBitmapHeaderT bmh;
};
using VmfsBitmapT = struct VmfsBitmapS;

/* Callback prototype for vmfs_bitmap_foreach() */
typedef void (*VmfsBitmapForeachCbkT)(VmfsBitmapT *b, uint32_t addr, void *optArg);

class VmfsBitmap {
public:
    static VmfsBitmap *Instance();
    virtual ~VmfsBitmap() = default;

public:
    /* Read a bitmap item from its entry and item numbers */
    bool VmfsBitmapGetItem(VmfsBitmapT *b, uint32_t entry, uint32_t item, u_char *buf);

    /* Open a bitmap file */
    VmfsBitmapT *VmfsBitmapOpenAt(VmfsDirT *d, const char *name);

    VmfsBitmapT *VmfsBitmapOpenFromInode(const VmfsInodeT *inode);

    /* Close a bitmap file */
    void VmfsBitmapClose(VmfsBitmapT *b);

private:
    int VmfsBMHRead(VmfsBitmapHeaderT *bmh, const u_char *buf);

    /* Read a bitmap entry */
    int VmfsBMERead(VmfsBitmapEntryT *bme, const u_char *buf, int copy_bitmap);

    /* Get position of an item */
    off_t VmfsBitmapGetItemPos(VmfsBitmapT *b, uint32_t entry, uint32_t item);

    /* Count the total number of allocated items in a bitmap area */
    uint32_t VmfsBitmapAreaAllocatedItems(VmfsBitmapT *b, u_int area);

    /* Call a user function for each allocated item in a bitmap */
    void VmfsBitmapAreaForeach(VmfsBitmapT *b, u_int area, VmfsBitmapForeachCbkT cbk, void *optArg);

    u_int VmfsBitmapGetIteamsPerArea(const VmfsBitmapHeaderT *bmh);

    off_t VmfsBitmapGetAreaAddr(const VmfsBitmapHeaderT *bmh, u_int area);

    void VmfsBitmapGetIteamOffset(const VmfsBitmapHeaderT *bmh, u_int addr, u_int *array_idx, u_int *bit_idx);

    void VmfsBitmapUpdateFFree(VmfsBitmapEntryT *entry);

    inline VmfsBitmapT *VmfsBitmapOpenFromFile(VmfsFileT *f);

private:
    VmfsBitmap() = default;
    static VmfsBitmap *m_instance;
    static std::mutex m_mutex;
};
}


#endif
