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
#ifndef __AGENT_VMWARENATIVE_FILESTORAGEDEVICE_H__
#define __AGENT_VMWARENATIVE_FILESTORAGEDEVICE_H__

#include "common/Types.h"
#include "AbstractStorageDevice.h"

class FileStorageDevice : public AbstractStorageDevice {
public:
    FileStorageDevice();
    virtual ~FileStorageDevice();
    mp_void* read(AGENT_VMWARENATIVE_DOUBLEQUEUE::DoubleQueue& dQueue, const vmware_volume_info& volumeInfo,
        mp_int32& condition, AGENT_VMWARENATIVE_COUNTDOWN::CountDown& countdown,
        AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock& mutex);
    mp_void* write(AGENT_VMWARENATIVE_DOUBLEQUEUE::DoubleQueue& dQueue, const vmware_volume_info& volumeInfo,
        mp_int32& condition, AGENT_VMWARENATIVE_COUNTDOWN::CountDown& countdown,
        AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock& mutex, mp_uint64 backupLevel = STORAGEDEVICE_BACKUPLEVEL::FULL);

private:
    mp_void GenerateDescFileContent(const vmware_volume_info& volumeInfo, mp_string& strContent);
    mp_bool PrepareForDiskCopy(const vmware_volume_info &volumeInfo, mp_string &folder, FILE **file,
        mp_uint64 backLevel);
    mp_bool GenerateDiskDescFile(mp_uint64 backupLevel, const vmware_volume_info& volumeInfo, const mp_string& folder);
    mp_void ReadDataBlockToQueue(AGENT_VMWARENATIVE_DOUBLEQUEUE::DoubleQueue& dQueue, FILE* file,
        std::unique_ptr<char[]>& buffer, const vmware_volume_info& volumeInfo, mp_uint64& block);

private:
    static mp_string m_strPathSeparatorIdentifier;
    static mp_string m_strNasFilesystemMnt;
    static mp_string m_strDiskFileExtension;
    static mp_string m_strDiskDescFileExtension;
    static mp_int32 m_iSectorSize;
};

#endif