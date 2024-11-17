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
#ifndef LVM_STRIPESEGMENT_H_
#define LVM_STRIPESEGMENT_H_

#include "afs/AfsLVM.h"
#include "afs/Segment.h"

typedef struct seg_strip_internal {
    // input
    vector<imgReader *> &reader_vect;
    char *buf;
    uint64_t count_sector;
    // data
    uint32_t start_extentno;
    int64_t start_mapped;
} seg_strip_internal_t;

typedef struct seg_strip2_internal {
    // input
    vector<imgReader *> &reader_vect;
    char *buf;
    uint64_t count_sector;
    // data
    uint32_t start_extentno;
    uint32_t start_stripeno; // /stripe深度下标
    int64_t start_mapped;
} seg_strip2_internal_t;


/**
 * @brief stripe类
 */
class stripeSegment : public segment {
public:
    stripeSegment();

    virtual ~stripeSegment();
    virtual int64_t findBlock(imgReader *reader, uint64_t start_sectno, char *buf, uint64_t count_sector,
        int32_t is_annotated);
    virtual int64_t mapVaddrToPaddr(int64_t vaddr, int32_t &disk_id);
    virtual int64_t getPartitionFirstAddr();
    virtual int32_t initSegment(segment_init_info *info);
    virtual int32_t setSegProp(afsLVM *plvm);

    // _pmspare:需要将整个lvm设置bitmap（一般是stripe模式，stripe实现即可）
    int32_t getBitMap(vector<BitMap *> &bitmap_vect);

private:
    int64_t findStripeBlock(vector<imgReader *> &reader_vect, uint64_t start_sectno, char *buf, uint64_t count_sector,
        int32_t is_annotated);
    int32_t findStripeBlockDoLoop(seg_strip2_internal_t &data, int32_t is_annotated);
    int64_t stripeCalcReadCnt(uint64_t &count_sector, uint32_t stripe_mod, int64_t &cnt_delta);
    int32_t stripeMapPaddr(seg_strip2_internal_t &data, uint32_t index, uint32_t index_stripe);
    int64_t mapStripeVaddrToPaddr(int64_t vaddr, int32_t &disk_id);

    int64_t calcReadCnt(uint64_t &count_sector, int64_t &cnt_delta);
    int32_t findBlockDoLoop(seg_strip_internal_t &data);
    int32_t parseSegStripedPropCalc(afsLVM *plvm);
    int32_t parseSegStripedPropStripeCount(afsLVM *plvm);

    stripeSegment(stripeSegment &obj);
    stripeSegment &operator = (const stripeSegment &obj);

public:
    // stripe_count计数
    uint32_t m_stripe_count;
    uint32_t m_stripe_size;
    // 两个向量一一对应(stripde的stripe_pv域作为map的key)
    vector<stripe> m_stripe_vector;
    //    vector<physicalVolume *> m_map_pvolumes_vector;
    map<int32_t, physicalVolume *> m_map_pvolumes_vector;
};

#endif /* LVM_STRIPESEGMENT_H_ */
