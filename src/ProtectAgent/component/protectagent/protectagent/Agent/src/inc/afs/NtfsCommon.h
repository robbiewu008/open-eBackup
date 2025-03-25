#ifndef FILESYSTEM_NTFSCOMMON_H_
#define FILESYSTEM_NTFSCOMMON_H_

#include "afs/FSCommon.h"

const uint8_t NTFS_ONE_CLUSTER_FLOW = 50;       // 用户定义一条簇流的大小
const uint8_t NTFS_BIT_PER_BYTE = 8;            // 一个字节的位数
const uint8_t NTFS_RUNLIST_REALLOC_LENGTH = 10; // runlist每次追加空间数

// LCN,VCN与簇流列表的映射关系(定义成链表)
struct ntfs_runlist_element {
    s64 vcn;    // vcn = 虚拟簇号
    s64 lcn;    // lcn = 逻辑簇号
    s64 length; // 簇流项的长度
};

class ntfsCommon {
public:
    ntfsCommon() {}

    // 相关处理函数
    int32_t analyzeRunList(const uint8_t *mft_pos, uint8_t mapping_first_byte, uint32_t runlist_count,
        uint8_t &runlist_one_length, struct ntfs_runlist_element **data_runlist, uint32_t &runlist_length);

    // 计算MFT记录占用磁盘空间大小
    uint32_t calculateMFTRecordSize(int8_t clusters_per_mft_record, uint32_t cluster_size);

    // 计算每个索引块节点的大小
    uint32_t calculateIndexBlockSize(int8_t clusters_per_index_block, uint32_t cluster_size);

#ifdef CPPUNIT_MAIN
protected:
#else
private:
#endif

    // 私有函数

    int32_t getOneRunlistItem(const uint8_t *mft_pos, uint8_t mapping_first_byte, uint32_t runlist_count,
        uint8_t &runlist_one_length, uint8_t *map_item, uint8_t &runlist_high, uint8_t &runlist_low);

    void getRunListItemInfo(uint8_t *map_item, uint8_t runlist_one_length, uint8_t runlist_high, uint8_t runlist_low,
        uint32_t runlist_count, int64_t &start_lcn, uint64_t &item_len);

    // 检查Runlist空间是否需要再扩充
    int32_t checkRunListSpace(uint32_t runlist_count, struct ntfs_runlist_element **data_runlist,
        uint32_t &runlist_length);
};

#endif /* FILESYSTEM_NTFSCOMMON_H_ */
