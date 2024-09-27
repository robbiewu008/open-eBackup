#ifndef __VMFS6_BLOCK_H__
#define __VMFS6_BLOCK_H__

#include <mutex>


namespace Vmfs6IO {
/* Block types */
enum VmfsBlockType {
    VMFS6_BLK_TYPE_NONE = 0,
    VMFS6_BLK_TYPE_FB, /* file block */
    VMFS6_BLK_TYPE_SB, /* sub block */
    VMFS6_BLK_TYPE_PB, /* pointer block */
    VMFS6_BLK_TYPE_FD, /* file descriptor */
    VMFS6_BLK_TYPE_PB2,
    VMFS6_BLK_TYPE_UNKNOWN, /* unknown */
    VMFS6_BLK_TYPE_LFB,     /* large file block */
    VMFS6_BLK_TYPE_MAX,
};

#define LARGE_BLOCK_SIZE 0X20000000

#define VMFS6_BLK_SHIFT(mask) __builtin_ctzl(mask)
#define VMFS6_BLK_VALUE(blkId, mask) (((blkId) & (mask)) >> VMFS6_BLK_SHIFT(mask))
#define VMFS6_BLK_MAX_VALUE(mask) (((mask) >> VMFS6_BLK_SHIFT(mask)) + 1)
#define VMFS6_BLK_FILL(value, mask) (((value) << VMFS6_BLK_SHIFT(mask)) & (mask))

#define VMFS6_BLK_TYPE_MASK 0x0000000000000007ULL

/* Extract block type from a block ID */
#define VMFS6_BLK_TYPE(blkId) VMFS6_BLK_VALUE(blkId, VMFS6_BLK_TYPE_MASK)

#define VMFS6_BLK_FB_ITEM_LSB_MASK 0x0ff8000000000000UL
#define VMFS6_BLK_FB_ITEM_MSB_MASK 0x00000000ffff8000UL
#define VMFS6_BLK_FB_ITEM_VALUE_LSB_MASK 0x01ffUL
#define VMFS6_BLK_FB_ITEM_VALUE_MSB_MASK 0x03fffe00UL

#define VMFS6_BLK_FB_FLAGS_MASK 0x0000000000000038UL
#define VMFS6_BLK_FB_ZERO_MASK 0x0000000000000080UL

/* TBZ flag specifies if the block must be zeroed. */
#define VMFS6_BLK_FB_TBZ_FLAG 4

#define VMFS6_BLK_FB_ITEM(blk_id)                                                                            \
    (VMFS6_BLK_FILL(VMFS6_BLK_VALUE(blk_id, VMFS6_BLK_FB_ITEM_LSB_MASK), VMFS6_BLK_FB_ITEM_VALUE_LSB_MASK) | \
        VMFS6_BLK_FILL(VMFS6_BLK_VALUE(blk_id, VMFS6_BLK_FB_ITEM_MSB_MASK), VMFS6_BLK_FB_ITEM_VALUE_MSB_MASK))

#define VMFS6_BLK_FB_FLAGS(blk_id) VMFS6_BLK_VALUE(blk_id, VMFS6_BLK_FB_FLAGS_MASK)
#define VMFS6_BLK_FB_ZERO(blk_id) VMFS6_BLK_VALUE(blk_id, VMFS6_BLK_FB_ZERO_MASK)

#define VMFS6_BLK_FB_MAX_ITEM VMFS6_BLK_MAX_VALUE(VMFS6_BLK_FB_ITEM_VALUE_LSB_MASK | VMFS6_BLK_FB_ITEM_VALUE_MSB_MASK)

#define VMFS6_BLK_FB_TBZ(blk_id) (VMFS6_BLK_FB_FLAGS(blk_id) & VMFS6_BLK_FB_TBZ_FLAG)

#define VMFS6_BLK_FB_TBZ_CLEAR(blk_id) ((blk_id) & ~(VMFS6_BLK_FILL(VMFS6_BLK_FB_TBZ_FLAG, VMFS6_BLK_FB_FLAGS_MASK)))

#define VMFS6_BLK_FB_BUILD(item, flags)                                                                       \
    (VMFS6_BLK_FILL(VMFS6_BLK_VALUE(item, VMFS6_BLK_FB_ITEM_VALUE_LSB_MASK), VMFS6_BLK_FB_ITEM_LSB_MASK) |    \
        VMFS6_BLK_FILL(VMFS6_BLK_VALUE(item, VMFS6_BLK_FB_ITEM_VALUE_MSB_MASK), VMFS6_BLK_FB_ITEM_MSB_MASK) | \
        VMFS6_BLK_FILL(flags, VMFS6_BLK_FB_FLAGS_MASK) | VMFS6_BLK_TYPE_FB)

#define VMFS6_BLK_SB_ITEM_LSB_MASK 0xff00000000000000UL
#define VMFS6_BLK_SB_ENTRY_MASK 0x0000000000003FC0UL
#define VMFS6_BLK_SB_FLAGS_MASK 0x0000000000000038UL
#define VMFS6_BLK_SB_ITEM_MSB_MASK 0x000000000003C000UL

#define VMFS6_BLK_SB_ITEM_VALUE_LSB_MASK 0x0000FFUL
#define VMFS6_BLK_SB_ITEM_VALUE_MSB_MASK 0x000F00UL

#define VMFS6_BLK_SB_ITEM(blkId)                                                                            \
    (VMFS6_BLK_FILL(VMFS6_BLK_VALUE(blkId, VMFS6_BLK_SB_ITEM_LSB_MASK), VMFS6_BLK_SB_ITEM_VALUE_LSB_MASK) | \
        VMFS6_BLK_FILL(VMFS6_BLK_VALUE(blkId, VMFS6_BLK_SB_ITEM_MSB_MASK), VMFS6_BLK_SB_ITEM_VALUE_MSB_MASK))
#define VMFS6_BLK_SB_ENTRY(blkId) VMFS6_BLK_VALUE(blkId, VMFS6_BLK_SB_ENTRY_MASK)
#define VMFS6_BLK_SB_FLAGS(blkId) VMFS6_BLK_VALUE(blkId, VMFS6_BLK_SB_FLAGS_MASK)

#define VMFS6_BLK_SB_MAX_ITEM VMFS6_BLK_MAX_VALUE(VMFS6_BLK_SB_ITEM_VALUE_LSB_MASK | VMFS6_BLK_SB_ITEM_VALUE_MSB_MASK)
#define VMFS6_BLK_SB_MAX_ENTRY VMFS6_BLK_MAX_VALUE(VMFS6_BLK_SB_ENTRY_MASK)

#define VMFS6_BLK_SB_BUILD(entry, item, flags)                                                                \
    (VMFS6_BLK_FILL(entry, VMFS6_BLK_SB_ENTRY_MASK) |                                                         \
        VMFS6_BLK_FILL(VMFS6_BLK_VALUE(item, VMFS6_BLK_SB_ITEM_VALUE_LSB_MASK), VMFS6_BLK_SB_ITEM_LSB_MASK) | \
        VMFS6_BLK_FILL(VMFS6_BLK_VALUE(item, VMFS6_BLK_SB_ITEM_VALUE_MSB_MASK), VMFS6_BLK_SB_ITEM_MSB_MASK) | \
        VMFS6_BLK_FILL(flags, VMFS6_BLK_SB_FLAGS_MASK) | VMFS6_BLK_TYPE_SB)

#define VMFS6_BLK_PB_ITEM_MASK 0xf0000000
#define VMFS6_BLK_PB_ENTRY_MASK 0x0fffffc0
#define VMFS6_BLK_PB_FLAGS_MASK 0x00000038

#define VMFS6_BLK_PB_ITEM(blkId) VMFS6_BLK_VALUE(blkId, VMFS6_BLK_PB_ITEM_MASK)
#define VMFS6_BLK_PB_ENTRY(blkId) VMFS6_BLK_VALUE(blkId, VMFS6_BLK_PB_ENTRY_MASK)
#define VMFS6_BLK_PB_FLAGS(blkId) VMFS6_BLK_VALUE(blkId, VMFS6_BLK_PB_FLAGS_MASK)

#define VMFS6_BLK_PB_MAX_ITEM VMFS6_BLK_MAX_VALUE(VMFS6_BLK_PB_ITEM_MASK)
#define VMFS6_BLK_PB_MAX_ENTRY VMFS6_BLK_MAX_VALUE(VMFS6_BLK_PB_ENTRY_MASK)

#define VMFS6_BLK_PB_BUILD(entry, item, flags)                                                       \
    (VMFS6_BLK_FILL(entry, VMFS6_BLK_PB_ENTRY_MASK) | VMFS6_BLK_FILL(item, VMFS6_BLK_PB_ITEM_MASK) | \
        VMFS6_BLK_FILL(flags, VMFS6_BLK_PB_FLAGS_MASK) | VMFS6_BLK_TYPE_PB)

#define VMFS6_BLK_PB2_ITEM_MASK 0xf8000000
#define VMFS6_BLK_PB2_ENTRY_MASK 0x07ffffc0
#define VMFS6_BLK_PB2_FLAGS_MASK 0x00000038

#define VMFS6_BLK_PB2_ITEM(blk_id) VMFS6_BLK_VALUE(blk_id, VMFS6_BLK_PB2_ITEM_MASK)
#define VMFS6_BLK_PB2_ENTRY(blk_id) VMFS6_BLK_VALUE(blk_id, VMFS6_BLK_PB2_ENTRY_MASK)
#define VMFS6_BLK_PB2_FLAGS(blk_id) VMFS6_BLK_VALUE(blk_id, VMFS6_BLK_PB2_FLAGS_MASK)

#define VMFS6_BLK_PB2_MAX_ITEM VMFS6_BLK_MAX_VALUE(VMFS6_BLK_PB2_ITEM_MASK)
#define VMFS6_BLK_PB2_MAX_ENTRY VMFS6_BLK_MAX_VALUE(VMFS6_BLK_PB2_ENTRY_MASK)

#define VMFS6_BLK_PB2_BUILD(entry, item, flags)                                                        \
    (VMFS6_BLK_FILL(entry, VMFS6_BLK_PB2_ENTRY_MASK) | VMFS6_BLK_FILL(item, VMFS6_BLK_PB2_ITEM_MASK) | \
        VMFS6_BLK_FILL(flags, VMFS6_BLK_PB2_FLAGS_MASK) | VMFS6_BLK_TYPE_PB2)

#define VMFS6_BLK_FD_ITEM_MASK 0xffc00000
#define VMFS6_BLK_FD_ENTRY_MASK 0x003fffc0
#define VMFS6_BLK_FD_FLAGS_MASK 0x00000038

#define VMFS6_BLK_FD_ITEM(blkId) VMFS6_BLK_VALUE(blkId, VMFS6_BLK_FD_ITEM_MASK)
#define VMFS6_BLK_FD_ENTRY(blkId) VMFS6_BLK_VALUE(blkId, VMFS6_BLK_FD_ENTRY_MASK)
#define VMFS6_BLK_FD_FLAGS(blkId) VMFS6_BLK_VALUE(blkId, VMFS6_BLK_FD_FLAGS_MASK)

#define VMFS6_BLK_FD_MAX_ITEM VMFS6_BLK_MAX_VALUE(VMFS6_BLK_FD_ITEM_MASK)
#define VMFS6_BLK_FD_MAX_ENTRY VMFS6_BLK_MAX_VALUE(VMFS6_BLK_FD_ENTRY_MASK)

#define VMFS6_BLK_FD_BUILD(entry, item, flags)                                                       \
    (VMFS6_BLK_FILL(entry, VMFS6_BLK_FD_ENTRY_MASK) | VMFS6_BLK_FILL(item, VMFS6_BLK_FD_ITEM_MASK) | \
        VMFS6_BLK_FILL(flags, VMFS6_BLK_FD_FLAGS_MASK) | VMFS6_BLK_TYPE_FD)

struct VmfsBlockInfoS {
    uint32_t entry, item, flags;
    enum VmfsBlockType type;
};

class VmfsBlock {
public:
    static VmfsBlock *Instance();
    virtual ~VmfsBlock() = default;

    /* Read a piece of a sub-block */
    ssize_t VmfsBlockReadSb(const VmfsFsT *fs, uint64_t blkId, off_t pos, u_char *buf, size_t len);

    /* Read a piece of a file block */
    ssize_t VmfsBlockReadFb(const VmfsFsT *fs, uint64_t blkId, off_t pos, u_char *buf, size_t len);

    /* Read a piece of a large file block */
    ssize_t VmfsBlockReadLfb(const VmfsFsT *fs, uint64_t blkId, off_t pos, u_char *buf, size_t len);

private:
    VmfsBlock() = default;
    static VmfsBlock *m_instance;
    static std::mutex m_mutex;
};
}

#endif
