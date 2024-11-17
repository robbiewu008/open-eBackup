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
#ifndef __VMFS5_BLOCK_H__
#define __VMFS5_BLOCK_H__

#include <mutex>


namespace Vmfs5IO {
/* Block types */
enum VmfsBlockType {
    VMFS5_BLK_TYPE_NONE = 0,
    VMFS5_BLK_TYPE_FB, /* file block */
    VMFS5_BLK_TYPE_SB, /* sub block */
    VMFS5_BLK_TYPE_PB, /* pointer block */
    VMFS5_BLK_TYPE_FD,
    VMFS5_BLK_TYPE_MAX,
};

#define VMFS5_BLK_SHIFT(mask) __builtin_ctz(mask)
#define VMFS5_BLK_VALUE(blkId, mask) (((blkId) & (mask)) >> VMFS5_BLK_SHIFT(mask))
#define VMFS5_BLK_MAX_VALUE(mask) (((mask) >> VMFS5_BLK_SHIFT(mask)) + 1)
#define VMFS5_BLK_FILL(value, mask) (((value) << VMFS5_BLK_SHIFT(mask)) & (mask))

#define VMFS5_BLK_TYPE_MASK 0x00000007

/* Extract block type from a block ID */
#define VMFS5_BLK_TYPE(blkId) VMFS5_BLK_VALUE(blkId, VMFS5_BLK_TYPE_MASK)

#define VMFS5_BLK_FB_ITEM_MASK 0xffffffc0
#define VMFS5_BLK_FB_FLAGS_MASK 0x00000038

/* TBZ flag specifies if the block must be zeroed. */
#define VMFS5_BLK_FB_TBZ_FLAG 4

#define VMFS5_BLK_FB_ITEM(blkId) VMFS5_BLK_VALUE(blkId, VMFS5_BLK_FB_ITEM_MASK)
#define VMFS5_BLK_FB_FLAGS(blkId) VMFS5_BLK_VALUE(blkId, VMFS5_BLK_FB_FLAGS_MASK)

#define VMFS5_BLK_FB_MAX_ITEM VMFS5_BLK_MAX_VALUE(VMFS5_BLK_FB_ITEM_MASK)

#define VMFS5_BLK_FB_TBZ(blkId) (VMFS5_BLK_FB_FLAGS(blkId) & VMFS5_BLK_FB_TBZ_FLAG)

#define VMFS5_BLK_FB_TBZ_CLEAR(blkId) ((blkId) & ~(VMFS5_BLK_FILL(VMFS5_BLK_FB_TBZ_FLAG, VMFS5_BLK_FB_FLAGS_MASK)))

#define VMFS5_BLK_FB_BUILD(item, flags) \
    (VMFS5_BLK_FILL(item, VMFS5_BLK_FB_ITEM_MASK) | VMFS5_BLK_FILL(flags, VMFS5_BLK_FB_FLAGS_MASK) | VMFS5_BLK_TYPE_FB)

#define VMFS5_BLK_SB_ITEM_LSB_MASK 0xf0000000
#define VMFS5_BLK_SB_ENTRY_MASK 0x0fffffc0
#define VMFS5_BLK_SB_FLAGS_MASK 0x00000020
#define VMFS5_BLK_SB_ITEM_MSB_MASK 0x00000018

#define VMFS5_BLK_SB_ITEM_VALUE_LSB_MASK 0x0000000f
#define VMFS5_BLK_SB_ITEM_VALUE_MSB_MASK 0x00000030

#define VMFS5_BLK_SB_ITEM(blkId)                                                                         \
    (VMFS5_BLK_FILL(VMFS5_BLK_VALUE(blkId, VMFS5_BLK_SB_ITEM_LSB_MASK), VMFS5_BLK_SB_ITEM_VALUE_LSB_MASK) | \
        VMFS5_BLK_FILL(VMFS5_BLK_VALUE(blkId, VMFS5_BLK_SB_ITEM_MSB_MASK), VMFS5_BLK_SB_ITEM_VALUE_MSB_MASK))
#define VMFS5_BLK_SB_ENTRY(blkId) VMFS5_BLK_VALUE(blkId, VMFS5_BLK_SB_ENTRY_MASK)
#define VMFS5_BLK_SB_FLAGS(blkId) VMFS5_BLK_VALUE(blkId, VMFS5_BLK_SB_FLAGS_MASK)

#define VMFS5_BLK_SB_MAX_ITEM VMFS5_BLK_MAX_VALUE(VMFS5_BLK_SB_ITEM_VALUE_LSB_MASK | VMFS5_BLK_SB_ITEM_VALUE_MSB_MASK)
#define VMFS5_BLK_SB_MAX_ENTRY VMFS5_BLK_MAX_VALUE(VMFS5_BLK_SB_ENTRY_MASK)

#define VMFS5_BLK_SB_BUILD(entry, item, flags)                                                             \
    (VMFS5_BLK_FILL(entry, VMFS5_BLK_SB_ENTRY_MASK) |                                                       \
        VMFS5_BLK_FILL(VMFS5_BLK_VALUE(item, VMFS5_BLK_SB_ITEM_VALUE_LSB_MASK), VMFS5_BLK_SB_ITEM_LSB_MASK) | \
        VMFS5_BLK_FILL(VMFS5_BLK_VALUE(item, VMFS5_BLK_SB_ITEM_VALUE_MSB_MASK), VMFS5_BLK_SB_ITEM_MSB_MASK) | \
        VMFS5_BLK_FILL(flags, VMFS5_BLK_SB_FLAGS_MASK) | VMFS5_BLK_TYPE_SB)

#define VMFS5_BLK_PB_ITEM_MASK 0xf0000000
#define VMFS5_BLK_PB_ENTRY_MASK 0x0fffffc0
#define VMFS5_BLK_PB_FLAGS_MASK 0x00000038

#define VMFS5_BLK_PB_ITEM(blkId) VMFS5_BLK_VALUE(blkId, VMFS5_BLK_PB_ITEM_MASK)
#define VMFS5_BLK_PB_ENTRY(blkId) VMFS5_BLK_VALUE(blkId, VMFS5_BLK_PB_ENTRY_MASK)
#define VMFS5_BLK_PB_FLAGS(blkId) VMFS5_BLK_VALUE(blkId, VMFS5_BLK_PB_FLAGS_MASK)

#define VMFS5_BLK_PB_MAX_ITEM VMFS5_BLK_MAX_VALUE(VMFS5_BLK_PB_ITEM_MASK)
#define VMFS5_BLK_PB_MAX_ENTRY VMFS5_BLK_MAX_VALUE(VMFS5_BLK_PB_ENTRY_MASK)

#define VMFS5_BLK_PB_BUILD(entry, item, flags)                                                    \
    (VMFS5_BLK_FILL(entry, VMFS5_BLK_PB_ENTRY_MASK) | VMFS5_BLK_FILL(item, VMFS5_BLK_PB_ITEM_MASK) | \
        VMFS5_BLK_FILL(flags, VMFS5_BLK_PB_FLAGS_MASK) | VMFS5_BLK_TYPE_PB)

#define VMFS5_BLK_FD_ITEM_MASK 0xffc00000
#define VMFS5_BLK_FD_ENTRY_MASK 0x003fffc0
#define VMFS5_BLK_FD_FLAGS_MASK 0x00000038

#define VMFS5_BLK_FD_ITEM(blkId) VMFS5_BLK_VALUE(blkId, VMFS5_BLK_FD_ITEM_MASK)
#define VMFS5_BLK_FD_ENTRY(blkId) VMFS5_BLK_VALUE(blkId, VMFS5_BLK_FD_ENTRY_MASK)
#define VMFS5_BLK_FD_FLAGS(blkId) VMFS5_BLK_VALUE(blkId, VMFS5_BLK_FD_FLAGS_MASK)

#define VMFS5_BLK_FD_MAX_ITEM VMFS5_BLK_MAX_VALUE(VMFS5_BLK_FD_ITEM_MASK)
#define VMFS5_BLK_FD_MAX_ENTRY VMFS5_BLK_MAX_VALUE(VMFS5_BLK_FD_ENTRY_MASK)

#define VMFS5_BLK_FD_BUILD(entry, item, flags)                                                    \
    (VMFS5_BLK_FILL(entry, VMFS5_BLK_FD_ENTRY_MASK) | VMFS5_BLK_FILL(item, VMFS5_BLK_FD_ITEM_MASK) | \
        VMFS5_BLK_FILL(flags, VMFS5_BLK_FD_FLAGS_MASK) | VMFS5_BLK_TYPE_FD)

struct VmfsBlockInfoS {
    uint32_t entry, item, flags;
    enum VmfsBlockType type;
};

class VmfsBlock {
public:
    static VmfsBlock *Instance();
    virtual ~VmfsBlock() = default;

    /* Read a piece of a sub-block */
    ssize_t VmfsBlockReadSb(const VmfsFsT *fs, uint32_t blkId, off_t pos, u_char *buf, size_t len);

    /* Read a piece of a file block */
    ssize_t VmfsBlockReadFb(const VmfsFsT *fs, uint32_t blkId, off_t pos, u_char *buf, size_t len);

private:
    VmfsBlock() = default;
    static VmfsBlock *m_instance;
    static std::mutex m_mutex;
};
}

#endif
