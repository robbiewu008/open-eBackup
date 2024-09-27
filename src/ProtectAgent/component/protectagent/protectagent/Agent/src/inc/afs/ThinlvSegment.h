/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file ThinlvSegment.h
 *
 * @brief AFS - LVM Thin Pool logical volume segment class
 *
 */

#ifndef LVM_THINLVSEGMENT_H_
#define LVM_THINLVSEGMENT_H_

#include "afs/AfsLVM.h"
#include "afs/Segment.h"
#include "afs/ThinPoolBridge.h"

/**
 * @brief thin-lv类
 */
class thinlvSegment : public segment {
public:
    thinlvSegment();
    virtual ~thinlvSegment();

    // thin lv
    string sthin_pool;
    // device id
    int32_t device_id;
    int32_t m_thin_lv_disk_id;

    logicalVolume *pool_lv;

    virtual int64_t findBlock(imgReader *reader, uint64_t start_sectno, char *buf, uint64_t count_sector,
        int32_t is_annotated);
    virtual int32_t initSegment(segment_init_info *info);
    virtual int64_t mapVaddrToPaddr(int64_t vaddr, int32_t &disk_id);
    virtual int32_t setSegProp(afsLVM *plvm);

    // 获得thin-lv的首地址
    virtual int64_t getPartitionFirstAddr();

private:
    logicalVolume *findPoolLv();
    thinlvSegment(thinlvSegment &obj);
    thinlvSegment &operator = (const thinlvSegment &obj);
};

#endif /* LVM_THINLVSEGMENT_H_ */
