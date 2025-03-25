/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file udev.h
 * @brief  Contains function declarations basic prepare backup fs media
 * @version 1.0.0
 * @date 2020-03-28
 * @author wangguitao 00510599
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
