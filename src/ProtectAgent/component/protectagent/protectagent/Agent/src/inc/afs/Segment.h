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
#ifndef LV_LVSEGMENT_H_
#define LV_LVSEGMENT_H_

#include "afs/AfsError.h"
#include "afs/ImgReader.h"
#include "afs/Afslibrary.h"
#include "afs/VolumeGroup.h"

/* Supported segment types */
typedef enum SEG_TYPE_ENU {
    SEG_NULL = -1,
    SEG_CRYPT,
    SEG_ERROR,
    SEG_LINEAR,
    SEG_MIRRORED,
    SEG_REPLICATOR,
    SEG_REPLICATOR_DEV,
    SEG_SNAPSHOT,
    SEG_SNAPSHOT_ORIGIN,
    SEG_SNAPSHOT_MERGE,
    SEG_STRIPED,
    SEG_ZERO,
    SEG_THIN_POOL,
    SEG_THIN,
    SEG_RAID0,
    SEG_RAID1,
    SEG_RAID10,
    SEG_RAID4,
    SEG_RAID5,
    SEG_RAID5_LA,
    SEG_RAID5_RA,
    SEG_RAID5_LS,
    SEG_RAID5_RS,
    SEG_RAID6_ZR,
    SEG_RAID6_NR,
    SEG_RAID6_NC,
    SEG_LAST,
} SEG_TYPE_ENU;

struct segment_type {
    string name;
    SEG_TYPE_ENU type;
};

struct segment_init_info {
    imgReader *reader;
};

class afsLVM;
class logicalVolume;

/**
 * @brief lv_segment:segment类
 */
class segment {
public:
    segment()
    {
        m_start_extent = 0;
        m_extent_count = 0;

        m_this_vg = NULL;
        m_this_lv = NULL;

        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"crypt\"", SEG_CRYPT));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"error\"", SEG_ERROR));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"linear\"", SEG_LINEAR));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"mirror\"", SEG_MIRRORED));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"replicator\"", SEG_REPLICATOR));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"replicator-dev\"", SEG_REPLICATOR_DEV));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"snapshot\"", SEG_SNAPSHOT));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"snapshot-origin\"", SEG_SNAPSHOT_ORIGIN));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"snapshot-merge\"", SEG_SNAPSHOT_MERGE));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"striped\"", SEG_STRIPED));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"zero\"", SEG_ZERO));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"thin-pool\"", SEG_THIN_POOL));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"thin\"", SEG_THIN));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"raid0\"", SEG_RAID0));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"raid1\"", SEG_RAID1));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"raid10\"", SEG_RAID10));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"raid4\"", SEG_RAID4));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"raid5\"", SEG_RAID5));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"raid5_la\"", SEG_RAID5_LA));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"raid5_ra\"", SEG_RAID5_RA));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"raid5_ls\"", SEG_RAID5_LS));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"raid5_rs\"", SEG_RAID5_RS));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"raid6_zr\"", SEG_RAID6_ZR));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"raid6_nr\"", SEG_RAID6_NR));
        m_segtypes_map.insert(pair<string, SEG_TYPE_ENU>("\"raid6_nc\"", SEG_RAID6_NC));
    }

    virtual ~segment()
    {
        m_this_vg = NULL;
        m_this_lv = NULL;
    }

    void setType(string key)
    {
        m_segtype.type = SEG_NULL;

        std::map<string, SEG_TYPE_ENU>::iterator segtype_it;
        segtype_it = m_segtypes_map.find(key);
        if (segtype_it != m_segtypes_map.end()) {
            m_segtype.type = segtype_it->second;
        }

        m_segtype.name = key;
    }

    SEG_TYPE_ENU getType()
    {
        return m_segtype.type;
    }

    bool isThinPoolMode()
    {
        if (SEG_THIN_POOL != getType()) {
            return false;
        }

        return true;
    }

    bool isThinLvMode()
    {
        if (SEG_THIN != getType()) {
            return false;
        }

        return true;
    }

    virtual int32_t initSegment(segment_init_info *info)
    {
        return AFS_SUCCESS;
    }

    virtual int64_t findBlock(imgReader *reader, uint64_t start_sectno, char *buf, uint64_t count_sector,
        int32_t is_annotated)
    {
        return -1;
    }

    virtual int64_t mapVaddrToPaddr(int64_t vaddr, int32_t &disk_id)
    {
        // 地址没找到返回-1,自己内部使用，不向外提供
        return -1;
    }

    virtual int64_t getPartitionFirstAddr()
    {
        return -1;
    }

    virtual int32_t setSegProp(afsLVM *plvm)
    {
        return AFS_SUCCESS;
    }

public:
    // 公共三个成员
    uint32_t m_start_extent;
    uint32_t m_extent_count;

    // 根据type生成不同segme
    // 根据type生成不同segments
    struct segment_type m_segtype;

    volumeGroup *m_this_vg;
    logicalVolume *m_this_lv;

private:
    segment(segment &obj);
    segment &operator = (const segment &obj);

    map<string, SEG_TYPE_ENU> m_segtypes_map;
};

#endif /* LV_LVSEGMENT_H_ */
