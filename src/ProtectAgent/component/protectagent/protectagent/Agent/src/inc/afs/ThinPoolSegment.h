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
#ifndef LVM_THINPOOLSEGMENT_H_
#define LVM_THINPOOLSEGMENT_H_

#include "afs/AfsLVM.h"
#include "afs/Segment.h"
#include "afs/ThinPoolBridge.h"

/**
 * @brief pool 类
 */
class thinpoolSegment : public segment {
public:
    thinpoolSegment();
    virtual ~thinpoolSegment();

    logicalVolume *findMetaLv();
    logicalVolume *findDataLv();

    //    int setThinkPoolSegmentInfo(uint32_t start_extent, uint32_t extent_count,
    //            struct segment_type segtype, string metadata, string pool);

    virtual int64_t findBlock(imgReader *reader, uint64_t start_sectno, char *buf, uint64_t count_sector,
        int32_t is_annotated);
    virtual int32_t initSegment(segment_init_info *info);
    virtual int64_t mapVaddrToPaddr(int64_t vaddr, int32_t &disk_id);
    virtual int32_t setSegProp(afsLVM *plvm);

    string m_sthin_metadata;
    string m_sthin_data;
    // device id
    int32_t m_thinlv_device_id;

    thinPoolBridge m_pb;

private:
    logicalVolume *lookupLv(string lv_name);

    thinpoolSegment(thinpoolSegment &obj);
    thinpoolSegment &operator = (const thinpoolSegment &obj);
};

#endif /* LVM_THINPOOLSEGMENT_H_ */
