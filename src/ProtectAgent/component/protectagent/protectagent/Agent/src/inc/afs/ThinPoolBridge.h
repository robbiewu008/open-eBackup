#ifndef LVM_THINPOOLBRIDGE_H_
#define LVM_THINPOOLBRIDGE_H_

#include "afs/LogMsg.h"
#include "afs/AfsError.h"
#include "afs/FSCommon.h"
#include "afs/LogicalVolume.h"

/* This should be plenty */
#define SPACE_MAP_ROOT_SIZE 128

/*
 * Compat feature flags.  Any incompat flags beyond the ones
 * specified below will prevent use of the thin metadata.
 */
#define THIN_FEATURE_COMPAT_SUPP 0UL
#define THIN_FEATURE_COMPAT_RO_SUPP 0UL
#define THIN_FEATURE_INCOMPAT_SUPP 0UL

#define THIN_SUPERBLOCK_MAGIC 27022010
#define THIN_SUPERBLOCK_LOCATION 0
#define THIN_VERSION 2
#define SECTOR_TO_BLOCK_SHIFT 3

#define SUPERBLOCK_CSUM_XOR 160774

/*
 * Little endian on-disk superblock and device details.
 */
struct thin_disk_superblock {
    le32 csum; /* Checksum of superblock except for this field. */
    le32 flags;
    le64 blocknr; /* This block number, dm_block_t. */

    u8 uuid[16];
    le64 magic;
    le32 version;
    le32 time;

    le64 trans_id; // 多线程操作

    /*
     * Root held by userspace transactions.
     */
    le64 held_root; // 多线程隔离资源标志
    // 64
    u8 data_space_map_root[SPACE_MAP_ROOT_SIZE];     // /
    u8 metadata_space_map_root[SPACE_MAP_ROOT_SIZE]; // 元数据描述
    // 320(0x140)
    /*
     * 2-level btree mapping (dev_id, (dev block, time)) -> data block
     */
    le64 data_mapping_root; // 数据根

    /*
     * Device detail root mapping dev_id -> device_details
     */
    le64 device_details_root;

    le32 data_block_size; /* In 512-byte sectors. */

    le32 metadata_block_size; /* In 512-byte sectors. */
    le64 metadata_nr_blocks;

    le32 compat_flags;
    le32 compat_ro_flags;
    le32 incompat_flags;
} __attribute__((__packed__));

struct disk_device_details {
    le64 mapped_blocks;
    le64 transaction_id; /* When created. */
    le32 creation_time;
    le32 snapshotted_time;
} __attribute__((__packed__));

struct disk_sm_root {
    le64 nr_blocks;
    le64 nr_allocated;
    le64 bitmap_root;
    le64 ref_count_root;
} __attribute__((__packed__));

struct disk_bitmap_header {
    le32 csum;
    le32 not_used;
    le64 blocknr;
} __attribute__((__packed__));

// btree
/*
 * Information about the values stored within the btree.
 */
struct dm_btree_value_type {
    void *context;

    /*
     * The size in bytes of each value.
     */
    uint32_t size;
};

/*
 * The shape and contents of a btree.
 */
struct dm_btree_info {
    //    struct dm_transaction_manager *tm;

    /*
     * Number of nested btrees. (Not the depth of a single tree.)
     */
    unsigned levels;
    struct dm_btree_value_type value_type;
};

/*
 * We'll need 2 accessor functions for n->csum and n->blocknr
 * to support dm-btree-spine.c in that case.
 */

enum node_flags {
    LVM_INTERNAL_NODE = 1,
    LVM_LEAF_NODE = 1 << 1
};

/*
 * Every btree node begins with this structure.  Make sure it's a multiple
 * of 8-bytes in size, otherwise the 64bit keys will be mis-aligned.
 */
struct node_header {
    le32 csum;
    le32 flags;
    le64 blocknr; /* Block this node is supposed to live in. */

    le32 nr_entries;
    le32 max_entries;
    le32 value_size;
    le32 padding;
} __attribute__((__packed__));

struct btree_node {
    struct node_header header;
    le64 keys[0];
} __attribute__((__packed__));

/**
 * @brief 连接thin-pool和hin-lv类
 */
class thinPoolBridge {
public:
    thinPoolBridge();
    thinPoolBridge(imgReader *reader, uint64_t device_id);
    virtual ~thinPoolBridge();

    // 解析超级块
    int32_t parseSb();

    // 虚拟块号  ***^_^***（mapping）***>>> 物理块号
    // (vir_blk_addr, size) -得到-> (value)
    int64_t mappingDatablk(uint64_t vir_blk_addr, char *buf, uint64_t size);
    int64_t mappingVaddToPaddr(int64_t vir_blk_addr, int32_t &disk_id);

    // 元数据和data的bitmap（512字节）
    int32_t getBitMap(vector<BitMap *> &bitmap_vect);

    void setReader(imgReader *reader)
    {
        m_reader = reader;
    }

    void setDeviceId(uint64_t device_id)
    {
        m_device_id = device_id;
    }

    void setMetaLv(logicalVolume *lv)
    {
        m_meta_lv = lv;
    }

    logicalVolume *getMetaLv()
    {
        return m_meta_lv;
    }

    void setDataLv(logicalVolume *lv)
    {
        m_data_lv = lv;
    }

    logicalVolume *getDataLv()
    {
        return m_data_lv;
    }

    uint32_t m_data_block_size; /* In 512-byte sectors. */

private:
    thinPoolBridge(thinPoolBridge &obj);
    thinPoolBridge &operator = (const thinPoolBridge &obj);

    int64_t btreeLookupDevidRootNode(btree_node *node);
    int64_t btreeLookupNode(btree_node *node, uint64_t root);

    // 多层btree查找
    int32_t btreeLookup(uint64_t key, int64_t *value_blk_num);

    // btree查找
    int32_t btreeLookupRaw(uint64_t key, int64_t *data);
    int32_t btreeLookupRawDoLoop(btree_node *pnode, uint64_t key);

    // 二分查找
    int32_t bsearch(btree_node *cur_node, uint64_t key, int32_t want_hi);

    /*
     * Some inlines.
     */
    le64 *keyPtr(struct btree_node *n, uint32_t index)
    {
        return n->keys + index;
    }

    void *valueBase(struct btree_node *n)
    {
        return &n->keys[n->header.max_entries];
    }

    void *valuePtr(struct btree_node *n, uint32_t index)
    {
        return (le64 *)valueBase(n) + index;
    }

    /*
     * Assumes the values are suitably-aligned and converts to core format.
     */
    uint64_t value64(struct btree_node *n, uint32_t index)
    {
        le64 *values_le = (le64 *)valueBase(n);

        return values_le[index];
    }

    // 1.获取元数据的node节点
    int32_t findBtreeNode(btree_node *cur_node, uint64_t node_nr);

    // 2.获取数据块
    int64_t findDataBlock(uint64_t blk_num, char *buf);

    // 读函数,操作
    int64_t rawReadOp(logicalVolume *lv, uint32_t unit, uint64_t block_nr, char *buf);

    // 获取bitmap
    int32_t rawGetBitmapOp(vector<BitMap *> &bitmap_vect);

    int32_t collectDeviceidMap();
    int32_t thinPoolBridgeDoSeg(vector<BitMap *> &bitmap_vect, uint64_t index);

    imgReader *m_reader;

    // 需要维护4个btree
    // 1.数据位图btree
    uint64_t m_data_bitmap_root;
    // 2.元数据位图btree
    uint64_t m_metal_data_bitmap_root;
    // 3.数据块btree
    uint64_t m_data_blk_root;
    // 3.设备 details btree
    uint64_t m_device_detail_root;

    uint64_t m_device_id;

    uint32_t m_metadata_block_size; /* In 512-byte sectors. */

    // 1.meta_lv
    logicalVolume *m_meta_lv;
    // 2.data_lv
    logicalVolume *m_data_lv;
    // sb_meta_root信息
    struct disk_sm_root m_sb_meta_root;

    // 缓存所有设备的节点key-> device-id, value-> node
    map<uint64_t, char *> m_map_devid;
};

#endif /* LVM_THINPOOLBRIDGE_H_ */
