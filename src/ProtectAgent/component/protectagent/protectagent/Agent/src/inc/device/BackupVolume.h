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
#ifndef _AGENT_DEVICE_BACKUP_VOLUME_H_
#define _AGENT_DEVICE_BACKUP_VOLUME_H_
#include "common/Types.h"

class BackupVolume {
public:
    BackupVolume(const mp_string &volWwn, const mp_string &isExtend, const mp_string &isMetaData,
                 const mp_string &volumeType);
    ~BackupVolume();

    mp_string GetVolWwn() const;
    mp_string GetIsExtend() const;
    mp_string GetIsMetaData() const;
    mp_string GetVolumeType() const;

private:
    mp_string volWwn;
    // extend flag, when the volume is extended, this flag will be set 1
    mp_string isExtend;
    // save backup metadata, when backup asm oracle database, can't save backup metadata into ASM diskgroup,
    // so we need to create a new 1GB filesystem to save backup metadata, and app will be set the flag,
    // which volume will be created metadata, it's valid only asm oracle storage
    mp_string isMetaData;
    // volumeType 0 is Logvolume 1 is Datavolume
    mp_string volumeType;
};

#endif
