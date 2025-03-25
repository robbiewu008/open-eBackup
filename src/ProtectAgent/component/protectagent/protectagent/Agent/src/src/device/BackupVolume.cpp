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