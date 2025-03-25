/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file BackupVolume.cpp
 * @brief  Contains function declarations for BackupVolume
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "device/BackupVolume.h"

#include "common/Log.h"

BackupVolume::BackupVolume(const mp_string &volWwn, const mp_string &isExtend, const mp_string &isMetaData,
                           const mp_string &volumeType)
{
    this->volWwn = volWwn;
    this->isExtend = isExtend;
    this->isMetaData = isMetaData;
    this->volumeType = volumeType;
}

BackupVolume::~BackupVolume()
{}

mp_string BackupVolume::GetVolWwn() const
{
    return volWwn;
}

mp_string BackupVolume::GetIsExtend() const
{
    return isExtend;
}

mp_string BackupVolume::GetIsMetaData() const
{
    return isMetaData;
}

mp_string BackupVolume::GetVolumeType() const
{
    return volumeType;
}