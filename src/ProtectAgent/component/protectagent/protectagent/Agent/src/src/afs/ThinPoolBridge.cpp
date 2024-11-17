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
#include "afs/ThinPoolBridge.h"
#include <exception>
#include "afs/StripeSegment.h"
#include "afs/ThinPoolSegment.h"
#include "afs/Afslibrary.h"

/**
 * @brief 构造函数
 */
thinPoolBridge::thinPoolBridge()
{
    m_reader = NULL;
    m_device_id = 0;

    // 1.数据位图btree
    m_data_bitmap_root = 0;
    // 2.元数据位图btree
    m_metal_data_bitmap_root = 0;
    // 3.数据块btree
    m_data_blk_root = 0;
    // 3.设备 details btree
    m_device_detail_root = 0;

    m_meta_lv = NULL;
    m_data_lv = NULL;
    m_reader = NULL;

    m_data_block_size = 0;     /* In 512-byte sectors. */
    m_metadata_block_size = 0; /* In 512-byte sectors. */

    memset_s(&m_sb_meta_root, sizeof(struct disk_sm_root), 0, sizeof(struct disk_sm_root));

    m_map_devid.clear();
}

/**
 * @brief 构造函数
 *
 * @param reader 句柄
 *
 * @param device_id 设备id
 */
thinPoolBridge::thinPoolBridge(imgReader *reader, uint64_t device_id) : m_reader(reader), m_device_id(device_id)
{
    // 1.数据位图btree
    m_data_bitmap_root = 0;
    // 2.元数据位图btree
    m_metal_data_bitmap_root = 0;
    // 3.数据块btree
    m_data_blk_root = 0;
    // 3.设备 details btree
    m_device_detail_root = 0;

    m_meta_lv = NULL;
    m_data_lv = NULL;
    m_reader = NULL;

    m_data_block_size = 0;     /* In 512-byte sectors. */
    m_metadata_block_size = 0; /* In 512-byte sectors. */

    memset_s(&m_sb_meta_root, sizeof(struct disk_sm_root), 0, sizeof(struct disk_sm_root));

    m_map_devid.clear();
}

/**
 * @brief 析构函数
 */
thinPoolBridge::~thinPoolBridge()
{
    map<uint64_t, char *>::iterator map_iter = m_map_devid.begin();

    while (map_iter != m_map_devid.end()) {
        if (NULL != map_iter->second) {
            delete[] map_iter->second;
            map_iter->second = NULL;
        }

        ++map_iter;
    }

    m_map_devid.clear();

    // 1.meta_lv
    m_meta_lv = NULL;
    // 2.data_lv
    m_data_lv = NULL;
}

/**
 * @brief 解析超级块
 *
 * @return
 * AFS_ERR_API
 * AFS_SUCCESS
 */
int32_t thinPoolBridge::parseSb()
{
    int64_t read_size = 0;
    uint32_t features = 0;
    thin_disk_superblock sb;
    int32_t ret = 0;
    int32_t disk_id = -1;
    int32_t pv_key = -1;
    CHECK_MEMSET_S_OK(&sb, sizeof(sb), 0, sizeof(sb));

    if (NULL == m_meta_lv) {
        AFS_TRACE_OUT_ERROR("Don't have meta volume");
        return AFS_ERR_API;
    }

    char *buf = new char[4096 * static_cast<uint64_t>(SECTOR_SIZE)]();
    if (NULL == buf) {
        AFS_TRACE_OUT_ERROR("Can't malloc space");
        return AFS_ERR_API;
    }

    list<segment *>::iterator segIter = (m_meta_lv->m_segments).begin();
    stripeSegment *pStripeSeg = dynamic_cast<stripeSegment *>(*segIter);
    pv_key = (pStripeSeg->m_stripe_vector[0]).stripe_pv;
    disk_id = (pStripeSeg->m_map_pvolumes_vector[pv_key])->disk_id;

    AFS_TRACE_OUT_DBG("thinPoolBridge::parseSb() disk_id = %d", disk_id);
    setReader(m_meta_lv->m_disk_readers_vect[disk_id]);

    // 读4K
    read_size = rawReadOp(m_meta_lv, 4096 / SECTOR_SIZE, 0, buf);
    if ((4096 / SECTOR_SIZE) != read_size) {
        AFS_TRACE_OUT_ERROR("Cann't Read Meta Volume");
        ret = AFS_ERR_API;
        goto fail_tail;
    }

    ret = memcpy_s(&sb, sizeof(sb), buf, sizeof(sb));
    if (ret != EOK) {
        AFS_TRACE_OUT_ERROR("call memcpy_s() failed, ret = %d", ret);
        goto fail_tail;
    }

    AFS_TRACE_OUT_INFO("Thin-Provision magic is (%llu, hex:0x19C52BA), ", sb.magic);
    AFS_TRACE_OUT_DBG("Thin-Provision incompat is (%ld), ", sb.incompat_flags);
    AFS_TRACE_OUT_DBG("Thin-Provision compat_ro_flags is (%ld), ", sb.compat_ro_flags);

    // 幻数判断
    if (sb.magic != THIN_SUPERBLOCK_MAGIC) {
        AFS_TRACE_OUT_ERROR("Can't support this LVM, right-magic(%ld)", THIN_SUPERBLOCK_MAGIC);
        ret = AFS_ERR_LVM_PART;
        goto fail_tail;
    }

    // 标志位解析
    features = sb.incompat_flags & ~THIN_FEATURE_INCOMPAT_SUPP;
    if (features) {
        AFS_TRACE_OUT_ERROR("Can't support this incompat");
        ret = AFS_ERR_LVM_PART;
        goto fail_tail;
    }

    features = sb.compat_ro_flags & ~THIN_FEATURE_COMPAT_RO_SUPP;
    if (features) {
        AFS_TRACE_OUT_ERROR("Can't support this ro incompat");
        ret = AFS_ERR_LVM_PART;
        goto fail_tail;
    }

    // 1.元数据位图btree
    // meta 信息
    ret = memcpy_s(&m_sb_meta_root, sizeof(m_sb_meta_root), sb.metadata_space_map_root, sizeof(m_sb_meta_root));
    if (ret != EOK) {
        AFS_TRACE_OUT_ERROR("call memcpy_s() failed, ret = %d", ret);
        goto fail_tail;
    }
    // 2.数据块btree
    m_data_blk_root = sb.data_mapping_root;
    // 3.设备 details btree
    m_device_detail_root = sb.device_details_root;

    // 块大小
    // m_data_block_size = sb.data_block_size;            /* In 512-byte sectors. */  //通过解析字符串获取
    m_metadata_block_size = sb.metadata_block_size; /* In 512-byte sectors. */

    // 缓存所有设备id的节点信息
    ret = collectDeviceidMap();
    AFS_TRACE_OUT_INFO("thin pool data blocks = %u(sectors), metadata blocks = %u(sectors)", m_data_block_size,
        m_metadata_block_size);

fail_tail:
    delete[] buf;
    return ret;
}

/**
 * @brief 虚地址到物理地址映射
 *
 * @param vir_blk_addr 虚地址
 *
 * @return 物理地址（扇区）
 * -1
 */
int64_t thinPoolBridge::mappingVaddToPaddr(int64_t vir_blk_addr, int32_t &disk_id)
{
    if ((0 == m_metadata_block_size) || (0 == m_data_block_size)) {
        AFS_TRACE_OUT_ERROR("The space is zero");
        return -1;
    }

    // 将扇区地址进行对齐操作
    uint64_t align_addr = vir_blk_addr / m_data_block_size;
    uint64_t remain_offset = vir_blk_addr % m_data_block_size;
    int64_t blk_num = 0;
    int32_t ret = 0;

    ret = btreeLookup(align_addr, &blk_num);
    // 地址返回错误
    if (ret == LVMNODATA) {
        return LVMNODATA;
    } else if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Can't new space");
        return -1;
    }

    // 消去低24位的时间
    blk_num >>= 24;

    int64_t map_addr = 0;
    list<segment *>::iterator seg_iter;
    stripeSegment *p_seg = NULL;
    // data卷查找地址
    for (seg_iter = m_data_lv->m_segments.begin(); seg_iter != m_data_lv->m_segments.end(); ++seg_iter) {
        p_seg = (stripeSegment *)(*seg_iter);
        if (NULL == p_seg) {
            AFS_TRACE_OUT_ERROR("The segment is NULL");
            return -1;
        }
        // 卷目前支持为这两种模式
        // 单位转换,dm和lvm，都转换为(扇区)
        map_addr = p_seg->mapVaddrToPaddr(static_cast<int64_t>(blk_num * m_data_block_size) + remain_offset, disk_id);
        if (-1 == map_addr) {
            // 不在此segment中，轮询下一个segment
            continue;
        } else if (map_addr < 0 || disk_id == -1) {
            AFS_TRACE_OUT_ERROR("Failed to convert virtual to disk. ret = %lld, disk_id = %d", (long long)map_addr,
                disk_id);
            return -1;
        }
        break;
    }

    return map_addr;

    // 单位扇区
    return (map_addr + remain_offset);
}

/**
 * @brief 映射数据块
 *
 * @param vir_blk_addr 虚拟块号
 * @param buf 缓存
 * @param size 大小(单位:扇区)
 *
 * @return 读到的数据（扇区）
 * AFS_ERR_LVM_PART
 * AFS_ERR_API
 */
int64_t thinPoolBridge::mappingDatablk(uint64_t vir_blk_addr, char *buf, uint64_t size)
{
    if (0 == m_metadata_block_size || 0 == m_data_block_size) {
        AFS_TRACE_OUT_ERROR("The space is zero");
        return AFS_ERR_LVM_PART;
    }

    // 1.利用data-根节点信息（2-level）btree，由虚拟块号映射到物理块号
    // 单位转换：源（扇区） >>>>>> 目的（超级块）
    // 构造两层key（device-id & virt_blk）和info结构体
    int64_t blk_num = 0;
    int32_t ret = 0;
    int64_t read_size = 0;
    uint64_t align_addr = vir_blk_addr / m_data_block_size;
    uint64_t remain_offset = vir_blk_addr % m_data_block_size;

    uint64_t align_cnt = (size + m_data_block_size - 1) / m_data_block_size;

    // 设置缓存偏移
    char *align_buf = new char[align_cnt * m_data_block_size * static_cast<uint64_t>(SECTOR_SIZE)]();
    if (NULL == align_buf) {
        AFS_TRACE_OUT_ERROR("Can't new space");
        return AFS_ERR_API;
    }

    // 64K-block
    for (uint64_t i = 0; i < align_cnt; i++) {
        ret = btreeLookup(align_addr + i, &blk_num);
        if (ret == LVMNODATA) {
            AFS_TRACE_OUT_DBG("the virtual block addr [%llu] can not be found", vir_blk_addr);
            continue;
        } else if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("Can't new space");
            delete[] align_buf;
            align_buf = NULL;
            return ret;
        }

        // 消去低24位的时间
        blk_num >>= 24;

        // // data-lv,find
        read_size = findDataBlock(blk_num, align_buf + i * m_data_block_size * SECTOR_SIZE);
        if (read_size != m_data_block_size) {
            AFS_TRACE_OUT_ERROR("Can't read buf");
            delete[] align_buf;
            align_buf = NULL;
            return read_size;
        }
    }

    if (ret != LVMNODATA) {
        ret = memcpy_s(buf, size * SECTOR_SIZE, align_buf + remain_offset * SECTOR_SIZE, size * SECTOR_SIZE);
        if (ret != EOK) {
            AFS_TRACE_OUT_ERROR("call memcpy_s() failed, ret = %d", ret);
            delete[] align_buf;
            align_buf = NULL;
            return AFS_ERR_API;
        }
    }

    delete[] align_buf;
    align_buf = NULL;

    return size;
}

/**
 * @brief 获得bitmap
 * @param bitmap 需要设置的bitmap
 */
int32_t thinPoolBridge::getBitMap(vector<BitMap *> &bitmap_vect)
{
    // 获取元数据的BitMap，数据的BitMap由文件系统进行设置
    return rawGetBitmapOp(bitmap_vect);
}

/**
 * @brief 收集devid-map信息
 *
 * @return
 * AFS_ERR_API API错误
 * AFS_SUCCESS 成功
 */
int32_t thinPoolBridge::collectDeviceidMap()
{
    uint64_t dev_root = 0;
    int32_t ret = AFS_SUCCESS;

    // 新建节点
    // 以下两个成员将作为除数
    if (0 == m_metadata_block_size || 0 == m_data_block_size) {
        AFS_TRACE_OUT_ERROR("The space is zero");
        return AFS_ERR_LVM_PART;
    }

    char *devid_node_buf = new char[static_cast<uint32_t>(m_metadata_block_size * SECTOR_SIZE)]();
    if (NULL == devid_node_buf) {
        AFS_TRACE_OUT_ERROR("Can't new space");
        return AFS_ERR_API;
    }

    btree_node *devid_node = reinterpret_cast<btree_node *>(devid_node_buf);

    ret = btreeLookupDevidRootNode(reinterpret_cast<btree_node *>(devid_node));
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Can't find devid node");
        delete[] devid_node_buf;
        return ret;
    }

    // 构建map
    uint32_t cnt = devid_node->header.nr_entries;
    for (uint32_t index = 0;
        index < cnt && index < ((uint64_t)m_metadata_block_size * SECTOR_SIZE - sizeof(node_header)); index++) {
        dev_root = value64(devid_node, index);
        char *root_node = new char[static_cast<uint32_t>(m_metadata_block_size * SECTOR_SIZE)]();
        if (NULL == root_node) {
            AFS_TRACE_OUT_ERROR("Can't new space");
            delete[] devid_node_buf;
            return AFS_ERR_API;
        }

        ret = btreeLookupNode(reinterpret_cast<btree_node *>(root_node), dev_root);
        if (AFS_SUCCESS != ret) {
            delete[] root_node;
            AFS_TRACE_OUT_ERROR("Can't find devid node");
            delete[] devid_node_buf;
            return ret;
        }

        m_map_devid.insert(pair<uint64_t, char *>((devid_node->keys)[index], root_node));
    }

    delete[] devid_node_buf;

    return AFS_SUCCESS;
}

/**
 * @brief 获取根节点
 *
 * @param node : 数据根节点
 *
 * @return 返回值
 */
int64_t thinPoolBridge::btreeLookupDevidRootNode(btree_node *node)
{
    return btreeLookupNode(node, m_data_blk_root);
}

/**
 * @brief 查找根节点
 *
 * @param node 节点
 * @param root 根号
 *
 * @return
 * AFS_ERR_LVM_PART 错误分区
 * AFS_SUCCESS      成功
 */
int64_t thinPoolBridge::btreeLookupNode(btree_node *node, uint64_t root)
{
    int32_t ret = findBtreeNode(node, root);
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Can't find btree node!");
        return AFS_ERR_LVM_PART;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 查找btree
 *
 * @param key            查找关键字
 * @param *value_blk_num block number
 *
 * @return
 * AFS_ERR_LVM_PART 错误分区
 * AFS_SUCCESS      成功
 */
int32_t thinPoolBridge::btreeLookup(uint64_t key, int64_t *value_blk_num)
{
    int32_t ret = 0;

    ret = btreeLookupRaw(key, value_blk_num);
    if (ret == LVMNODATA) {
        return LVMNODATA;
    } else if (ret != AFS_SUCCESS) {
        AFS_TRACE_OUT_ERROR("Fail to lookup btree.");
        return AFS_ERR_LVM_PART;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 循环查找数据
 *
 * @param pnode 节点信息
 * @param key 关键字
 *
 * @return 返回索引号
 * AFS_ERR_API
 */
int32_t thinPoolBridge::btreeLookupRawDoLoop(btree_node *pnode, uint64_t key)
{
    int32_t index = 0;
    int32_t ret = 0;
    uint64_t i_count = 0;
    uint32_t flags = 0;
    uint32_t nr_entries = 0;
    uint64_t root = 0;

    do {
        // 第一次的根已经缓存
        if (0 != i_count) {
            ret = findBtreeNode(pnode, root);
            if (AFS_SUCCESS != ret) {
                AFS_TRACE_OUT_ERROR("Can't find btree node!");
                return AFS_ERR_LVM_PART;
            }
        }

        index = bsearch(pnode, key, 0);

        nr_entries = pnode->header.nr_entries;
        if (index < 0 || index >= (int32_t)nr_entries) {
            AFS_TRACE_OUT_ERROR("Can't find btree node! nr_entries(%d) index(%d)", (int32_t)nr_entries, index);
            return AFS_ERR_LVM_PART;
        }

        flags = pnode->header.flags;
        if (flags & (uint32_t)LVM_INTERNAL_NODE) {
            root = value64(pnode, (uint32_t)index);
        }

        i_count++;
    } while (!(flags & (uint32_t)LVM_LEAF_NODE));

    return index;
}

/**
 * @brief btree原始查找
 *
 * @param key 关键字
 * @param data 数据
 *
 * @return
 * LVMNODATA 没有数据
 * AFS_ERR_LVM_PART 错误分区
 * AFS_SUCCESS      成功
 */
int32_t thinPoolBridge::btreeLookupRaw(uint64_t key, int64_t *data)
{
    int32_t index = 0;
    int32_t ret = 0;
    uint64_t rkey = 0;

    // 新建节点
    char *p_buf = new char[m_metadata_block_size * static_cast<uint64_t>(SECTOR_SIZE)]();
    if (NULL == p_buf) {
        AFS_TRACE_OUT_ERROR("Can't new space");
        return AFS_ERR_API;
    }

    btree_node *pnode = (btree_node *)p_buf;
    ret = memcpy_s(pnode, m_metadata_block_size * static_cast<uint64_t>(SECTOR_SIZE), m_map_devid[m_device_id],
        (m_metadata_block_size * static_cast<uint64_t>(SECTOR_SIZE)));
    if (ret != EOK) {
        AFS_TRACE_OUT_ERROR("call memcpy_s() failed, ret = %d", ret);
        goto lookup_failed;
    }

    index = btreeLookupRawDoLoop(pnode, key);
    if (index < 0) {
        AFS_TRACE_OUT_ERROR("Can't find node.");
        ret = AFS_ERR_API;
        goto lookup_failed;
    }

    rkey = pnode->keys[index];
    // 没找到key
    if (rkey != key) {
        AFS_TRACE_OUT_ERROR("Find node's key is invalid.right-key(%d) find-key(%llu)", key, rkey);
        ret = LVMNODATA;
        goto lookup_failed;
    }

    ret = memcpy_s(data, sizeof(int64_t), valuePtr(pnode, (uint32_t)index), (size_t)(pnode->header.value_size));
    if (ret != EOK) {
        AFS_TRACE_OUT_ERROR("call memcpy_s() failed, ret = %d", ret);
        goto lookup_failed;
    }

    ret = AFS_SUCCESS;

lookup_failed:
    delete[] p_buf;
    p_buf = NULL;
    return ret;
}

/**
 * @brief 二分查找
 *
 * @param cur_node 节点信息
 * @param key 关键字
 * @param want_hi 高标志位
 *
 * @return 正确的key
 */
int32_t thinPoolBridge::bsearch(btree_node *cur_node, uint64_t key, int32_t want_hi)
{
    btree_node *pnode = cur_node;
    int32_t lo = -1;
    int32_t hi = pnode->header.nr_entries; // n ---> node

    while (hi - lo > 1) {
        int32_t mid = lo + ((hi - lo) / 2);
        uint64_t mid_key = pnode->keys[mid];
        if (mid_key == key)
            return mid;
        if (mid_key < key)
            lo = mid;
        else
            hi = mid;
    }

    return want_hi ? hi : lo;
}

/**
 * @brief 读数据
 *
 * @param lv 需要读的lv
 * @param unit 单元大小
 * @param block_nr 块号
 * @param buf 缓存
 *
 * @return 读到的大小（扇区）
 */
int64_t thinPoolBridge::rawReadOp(logicalVolume *lv, uint32_t unit, uint64_t block_nr, char *buf)
{
    int64_t read_size = 0;
    int64_t read_ret = 0;
    for (list<segment *>::iterator seg_iter = lv->m_segments.begin(); seg_iter != lv->m_segments.end(); ++seg_iter) {
        if (NULL == (*seg_iter)) {
            AFS_TRACE_OUT_ERROR("The segment is NULL");
            return AFS_ERR_LVM_PART;
        }
        // 卷目前支持为这两种模式扇区
        // 单位转换,dm和lvm，都转换为
        read_ret = (*seg_iter)->findBlock(m_reader, (unit * block_nr), buf, unit, 0);
        if (read_ret < 0) {
            AFS_TRACE_OUT_ERROR("Failed to read disk. ret = %lld", (long long)read_ret);
            return 0; // 返回read size 0
        }

        read_size += read_ret;
        // 不在当前segment 或者 未完全读完
        if ((read_ret == 0) || (static_cast<uint32_t>(read_size) < unit)) {
            continue;
        } else {
            // 读完结束
            break;
        }
    }
    return read_size;
}

/**
 * @brief find空闲节点
 *
 * @param cur_node 节点信息
 * @param node_nr 节点号
 *
 * @return
 * AFS_ERR_LVM_PART 错误lvm分区
 * AFS_SUCCESS      成功
 */
int32_t thinPoolBridge::findBtreeNode(btree_node *cur_node, uint64_t node_nr)
{
    int32_t ret = rawReadOp(m_meta_lv, m_metadata_block_size, node_nr, (char *)cur_node);
    if (ret <= 0) {
        AFS_TRACE_OUT_ERROR("Can't find btree node!");
        return AFS_ERR_LVM_PART;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 寻找数据块
 *
 * @param blk_num 块号
 * @param buf 缓存
 *
 * @return 查找到的数据（扇区）
 * AFS_ERR_LVM_PART 错误的lvm分区
 */
int64_t thinPoolBridge::findDataBlock(uint64_t blk_num, char *buf)
{
    int64_t read_size = rawReadOp(m_data_lv, m_data_block_size, blk_num, buf);
    if (read_size <= 0) {
        AFS_TRACE_OUT_ERROR("Can't find read data lv area!");
        return AFS_ERR_LVM_PART;
    }

    return read_size;
}

/**
 * @brief 设置segment的bitmap
 *
 * @param bitmap
 * @param index 索引
 *
 * @return
 * AFS_ERR_LVM_PART
 * AFS_SUCCESS
 */
int32_t thinPoolBridge::thinPoolBridgeDoSeg(vector<BitMap *> &bitmap_vect, uint64_t index)
{
    if (NULL == m_meta_lv) {
        AFS_TRACE_OUT_ERROR("The space is NULL");
        return AFS_ERR_LVM_PART;
    }

    list<segment *>::iterator seg_iter;
    int64 offset = 0;
    int32_t ret = 0;
    int32_t disk_id = -1;
    BitMap *pBitmap = NULL;
    uint32_t stripe_size = m_meta_lv->m_stripe_size;

    if (stripe_size != 0 && m_metadata_block_size > stripe_size) {
        AFS_TRACE_OUT_ERROR("m_metadata_block_size [%u] > stripe_size [%u], inner error!", m_metadata_block_size,
            stripe_size);
        return AFS_ERR_INNER;
    }

    for (seg_iter = m_meta_lv->m_segments.begin(); seg_iter != m_meta_lv->m_segments.end(); seg_iter++) {
        if (NULL == (*seg_iter)) {
            AFS_TRACE_OUT_ERROR("The segment is NULL");
            return AFS_ERR_LVM_PART;
        }

        // 卷目前支持为这两种模式
        // 单位转换,dm和lvm，都转换为扇区
        disk_id = -1;
        offset = (*seg_iter)->mapVaddrToPaddr(static_cast<int64_t>(index * m_metadata_block_size), disk_id);
        if (-1 == offset) {
            // 没找到，找下一个segment
            continue;
        } else if (offset < 0 || -1 == disk_id) {
            AFS_TRACE_OUT_ERROR("Failed to convert virtual to disk. ret = %lld, disk_id =%d", (long long)offset,
                disk_id);
            return AFS_ERR_LVM_PART;
        }

        pBitmap = bitmap_vect[disk_id];
        ret = pBitmap->bitmapSetRange(offset, m_metadata_block_size, 1);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("thinPoolBridgeDoSeg Can't set bitmap");
            return ret;
        }

        break;
    }

    return AFS_SUCCESS;
}

/**
 * @brief 获取bitmap
 *
 * @param bitmap 需要设置的bitmap
 *
 * @return 设置bitmap是否成功
 * AFS_SUCCESS
 */
int32_t thinPoolBridge::rawGetBitmapOp(vector<BitMap *> &bitmap_vect)
{
    int32_t ret = 0;

    // /处理thin pool下所有的
    AFS_TRACE_OUT_DBG("the thin pool has total[%lld] metadata block[%u sectors]", m_sb_meta_root.nr_blocks,
        m_metadata_block_size);
    for (uint64_t index = 0; index < m_sb_meta_root.nr_blocks; index++) {
        ret = thinPoolBridgeDoSeg(bitmap_vect, index);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("Can't set bitmap");
            return ret;
        }
    }

    return AFS_SUCCESS;
}
