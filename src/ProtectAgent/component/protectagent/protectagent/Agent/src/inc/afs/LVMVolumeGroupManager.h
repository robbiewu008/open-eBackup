/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file LVMVolumeGroupManager.h
 *
 * @brief Afs - A C/C++ Library for analyze format of a disk image.
 *
 */

#ifndef PARTITION_ILVMVOLUMEGROUP_H_
#define PARTITION_ILVMVOLUMEGROUP_H_

#include "afs/AfsObject.h"
#include "afs/Bitmap.h"
#include "afs/VolumeGroup.h"

/**
 * @brief 卷组管理
 */
class LVMVolumeGroupManager {
public:
    LVMVolumeGroupManager() {}
    ~LVMVolumeGroupManager()
    {
        list<volumeGroup *>::iterator iter;
        for (iter = groupList.begin(); iter != groupList.end(); ++iter) {
            if (NULL != (*iter)) {
                delete (*iter);
                (*iter) = NULL;
            }
        }
        groupList.clear();
    }

    int32_t getBitmap(vector<BitMap *> &bitmap_vect);

    // vg:卷组
    volumeGroup *addVolgroup(string uuid, string name, int32_t seq, int32_t size);
    volumeGroup *findVolgroup(string &uuid);

    // 查找分区id对应的lv信息
    logicalVolume *findPartIdMapLv(int32_t index);

    list<volumeGroup *> &getGroupListAddr()
    {
        return groupList;
    }
    uint32_t getVGSize()
    {
        return groupList.size();
    }

private:
    list<volumeGroup *> groupList;
};

#endif /* PARTITION_ILVMVOLUMEGROUP_H_ */
