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
