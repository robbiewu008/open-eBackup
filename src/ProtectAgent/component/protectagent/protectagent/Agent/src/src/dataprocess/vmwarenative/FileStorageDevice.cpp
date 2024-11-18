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
#include "dataprocess/vmwarenative/FileStorageDevice.h"
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include "common/Defines.h"
#include "dataprocess/vmwarenative/CountDown.h"
#include "dataprocess/vmwarenative/Define.h"
#include "dataprocess/vmwarenative/MutexLock.h"
#include "dataprocess/vmwarenative/MutexLockGuard.h"
#include "apps/vmwarenative/VMwareDef.h"

using namespace AGENT_VMWARENATIVE_MUTEXLOCK;
using namespace AGENT_VMWARENATIVE_COUNTDOWN;
using namespace AGENT_VMWARENATIVE_MUTEXLOCKGUARD;
using namespace AGENT_VMWARENATIVE_DOUBLEQUEUE;

mp_string FileStorageDevice::m_strPathSeparatorIdentifier = "/";
mp_string FileStorageDevice::m_strNasFilesystemMnt = "/opt/advbackup/vmware/data/";
mp_string FileStorageDevice::m_strDiskFileExtension = "-flat.vmdk";
mp_string FileStorageDevice::m_strDiskDescFileExtension = ".vmdk";
mp_int32 FileStorageDevice::m_iSectorSize = 512;
FileStorageDevice::FileStorageDevice()
{}

FileStorageDevice::~FileStorageDevice()
{}

mp_void FileStorageDevice::ReadDataBlockToQueue(DoubleQueue& dQueue, FILE* file, std::unique_ptr<char[]>& buffer,
    const vmware_volume_info& volumeInfo, mp_uint64& block)
{
    for (std::vector<dirty_range>::const_iterator iter = volumeInfo.vecDirtyRange.begin();
         iter != volumeInfo.vecDirtyRange.end();
         ++iter) {
        if (memset_s(buffer.get(), VMWARE_DATABLOCK_SIZE, 0, VMWARE_DATABLOCK_SIZE) != EOK) {
            COMMLOG(OS_LOG_ERROR, "memset_s exec failed!");
            break;
        }
        // check dirty range scope
        if (volumeInfo.ulDiskSize <= iter->start) {
            COMMLOG(OS_LOG_ERROR, "Invalid offset value, no data will be written to queue.");
            break;
        }
        if (volumeInfo.ulDiskSize < (iter->start + VMWARE_DATABLOCK_SIZE)) {
            m_sizeToProtect = volumeInfo.ulDiskSize - iter->start;
            COMMLOG(OS_LOG_WARN, "The allocated memory block is small than 4M!");
        } else {
            m_sizeToProtect = VMWARE_DATABLOCK_SIZE;
        }
        // read data from FusionStorage mounted file
        if (-1 == fseek(file, iter->start, SEEK_SET)) {
            COMMLOG(OS_LOG_ERROR,
                "fseek exec failed, task '%s', parent task '%s'.",
                volumeInfo.strTaskID.c_str(),
                volumeInfo.strParentTaskID.c_str());
            break;
        }
        (void)fread(buffer.get(), m_sizeToProtect, 1, file);

        // push data buffer to queue
        mp_string strTmpBuff(buffer.get(), m_sizeToProtect);
        st_disk_datablock st_dataBlock(iter->start, m_sizeToProtect, strTmpBuff);
        // push data to queue
        dQueue.EnQueue(st_dataBlock);
        block++;
    }
}

mp_void* FileStorageDevice::read(DoubleQueue& dQueue, const vmware_volume_info& volumeInfo, mp_int32& condition,
    AGENT_VMWARENATIVE_COUNTDOWN::CountDown& countdown, AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock& mutex)
{
    auto buffer = std::make_unique<char[]>(VMWARE_DATABLOCK_SIZE);
    mp_uint64 numOfBlockRead = 0;

    // open backend storage lun mounted for reading, pls note the open mode 'rb'
    mp_string strFilePath = m_strNasFilesystemMnt + volumeInfo.strParentTaskID +
                            FileStorageDevice::m_strPathSeparatorIdentifier + volumeInfo.strBackupedDiskID +
                            FileStorageDevice::m_strPathSeparatorIdentifier + volumeInfo.strBackupedDiskID +
                            m_strDiskFileExtension;
    FILE* pDiskFile = NULL;

    if (buffer.get() == NULL) {
        COMMLOG(OS_LOG_ERROR, "Unable to malloc memory '%s'.", strFilePath.c_str());
        goto out;
    }
    pDiskFile = fopen(strFilePath.c_str(), "rb");
    if (pDiskFile == NULL) {
        COMMLOG(OS_LOG_ERROR, "Unable to open backend nas file '%s'.", strFilePath.c_str());
        goto out;
    }

    ReadDataBlockToQueue(dQueue, pDiskFile, buffer, volumeInfo, numOfBlockRead);

    if (numOfBlockRead == static_cast<mp_uint64>(volumeInfo.vecDirtyRange.size())) {
        COMMLOG(OS_LOG_INFO,
            "ReadDataLunFun exec successfully, '%d' data block has been pushed to data block queue, \
            task '%s', parent task '%s'!",
            numOfBlockRead,
            volumeInfo.strTaskID.c_str(),
            volumeInfo.strParentTaskID.c_str());
    }

out:
    if (pDiskFile != NULL) {
        fclose(pDiskFile);
        pDiskFile = NULL;
    }

    MutexLockGuard lock(mutex);
    ++condition;
    countdown.Done();
    return ((void*)0);
}

mp_void* FileStorageDevice::write(DoubleQueue& dQueue, const vmware_volume_info& volumeInfo, mp_int32& condition,
    AGENT_VMWARENATIVE_COUNTDOWN::CountDown& countdown, AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock& mutex,
    mp_uint64 backupLevel)
{
    mp_string strDiskFolder;
    FILE* pDiskFile = NULL;

    // create disk folder and remove disk file
    if (!PrepareForDiskCopy(volumeInfo, strDiskFolder, &pDiskFile, backupLevel)) {
        COMMLOG(OS_LOG_ERROR,
            "Unable to create disk folder, task '%s', parent task '%s'!",
            volumeInfo.strTaskID.c_str(),
            volumeInfo.strParentTaskID.c_str());
        goto out;
    }

    dQueue.SetNumberOfDataBlockCompleted(0);

    // loop to read data block from queue and write it to target lun
    while (true) {
        int threadCount = 0;
        {
            MutexLockGuard lock(mutex);
            threadCount = condition;
        }
        if (dQueue.DeQueueToDataLun(pDiskFile, 1, threadCount)) {
            MutexLockGuard lock(mutex);
            condition = 0;
            break;
        }
    }
    if (dQueue.GetNumberOfDataBlockCompleted() == static_cast<mp_uint64>(volumeInfo.vecDirtyRange.size())) {
        // generate disk descriptor file
        if (!GenerateDiskDescFile(backupLevel, volumeInfo, strDiskFolder)) {
            COMMLOG(OS_LOG_ERROR,
                "Unable to create disk desc file, task '%s', parent task '%s'.",
                volumeInfo.strTaskID.c_str(),
                volumeInfo.strParentTaskID.c_str());
            goto out;
        }
        COMMLOG(OS_LOG_INFO,
            "All '%ul' data block(s) have been written to target vmdk file '%s' successfully, \
            task '%s', parent task '%s'.",
            dQueue.GetNumberOfDataBlockCompleted(),
            volumeInfo.strDiskID.c_str(),
            volumeInfo.strTaskID.c_str(),
            volumeInfo.strParentTaskID.c_str());
    }

out:
    countdown.Done();

    if (pDiskFile != NULL) {
        fclose(pDiskFile);
        pDiskFile = NULL;
    }

    return ((void*)0);
}

mp_void FileStorageDevice::GenerateDescFileContent(const vmware_volume_info& volumeInfo, mp_string& strContent)
{
    std::ostringstream strParam;
    strParam << "# Disk DescriptorFile" << STR_CODE_WARP << "version" << STR_EQUAL
             << std::to_string(volumeInfo.descFileInfo.iVersion) << STR_CODE_WARP << "encoding" << STR_EQUAL
             << STR_QUOTATION_MARK << volumeInfo.descFileInfo.strEncode << STR_QUOTATION_MARK << STR_CODE_WARP << "CID"
             << STR_EQUAL << volumeInfo.descFileInfo.strCID << STR_CODE_WARP << "parentCID" << STR_EQUAL
             << volumeInfo.descFileInfo.strParentCID << STR_CODE_WARP << "createType" << STR_EQUAL << STR_QUOTATION_MARK
             << volumeInfo.descFileInfo.strCreateType << STR_QUOTATION_MARK << STR_CODE_WARP << STR_CODE_WARP
             << "# Extent description" << STR_CODE_WARP << "RW "
             << std::to_string(volumeInfo.ulDiskSize / m_iSectorSize) << STR_SPACE << "VMFS " << STR_QUOTATION_MARK
             << volumeInfo.strDiskID + m_strDiskFileExtension << STR_QUOTATION_MARK << STR_CODE_WARP << STR_CODE_WARP
             << "# The Disk Data Base" << STR_CODE_WARP << "#DDB" << STR_CODE_WARP << STR_CODE_WARP
             << " ddb.adapterType " << STR_EQUAL << STR_SPACE << STR_QUOTATION_MARK
             << volumeInfo.descFileInfo.strAdapterType << STR_QUOTATION_MARK << STR_CODE_WARP
             << " ddb.geometry.cylinders " << STR_EQUAL << STR_SPACE << STR_QUOTATION_MARK
             << volumeInfo.descFileInfo.strCylinders << STR_QUOTATION_MARK << STR_CODE_WARP << " ddb.geometry.heads "
             << STR_EQUAL << STR_SPACE << STR_QUOTATION_MARK << volumeInfo.descFileInfo.strHeads << STR_QUOTATION_MARK
             << STR_CODE_WARP << " ddb.geometry.sectors " << STR_EQUAL << STR_SPACE << STR_QUOTATION_MARK
             << volumeInfo.descFileInfo.strSectors << STR_QUOTATION_MARK << STR_CODE_WARP << " ddb.longContentID "
             << STR_EQUAL << STR_SPACE << STR_QUOTATION_MARK << volumeInfo.descFileInfo.strLogContentID
             << STR_QUOTATION_MARK << STR_CODE_WARP << " ddb.uuid " << STR_EQUAL << STR_SPACE << STR_QUOTATION_MARK
             << volumeInfo.strDiskID << STR_QUOTATION_MARK << STR_CODE_WARP << " ddb.virtualHWVersion " << STR_EQUAL
             << STR_SPACE << STR_QUOTATION_MARK << volumeInfo.descFileInfo.strVirtualHWVersion << STR_QUOTATION_MARK
             << STR_CODE_WARP;
    strContent = strParam.str();
}

// create disk folder, disk file and disk desc file
mp_bool FileStorageDevice::PrepareForDiskCopy(const vmware_volume_info &volumeInfo, mp_string &folder, FILE **file,
    mp_uint64 backupLevel)
{
    umask(0);
    // create folder if not exists
    folder = m_strNasFilesystemMnt + volumeInfo.strParentTaskID + FileStorageDevice::m_strPathSeparatorIdentifier +
             volumeInfo.strDiskID;
    if ((access(folder.c_str(), 0) == -1) && (mkdir(folder.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0)) {
        COMMLOG(OS_LOG_ERROR,
            "Unable to create folder '%s' for saving disk copy, errno[%d]: %s.",
            folder.c_str(),
            errno,
            strerror(errno));
        return false;
    }

    // remove disk file if exists only when backup level is full
    mp_string strDiskFile = folder + FileStorageDevice::m_strPathSeparatorIdentifier + volumeInfo.strDiskID +
                            m_strDiskFileExtension;

    // update dirty range if backup level is incr
    if (backupLevel == STORAGEDEVICE_BACKUPLEVEL::INCR) {
        // update file
        if (access(strDiskFile.c_str(), F_OK) == 0) {
            *file = fopen(strDiskFile.c_str(), "rb+");
            if (*file == NULL) {
                COMMLOG(OS_LOG_ERROR,
                    "Unable to open disk file '%s%s', task '%s', parent task '%s'.",
                    volumeInfo.strDiskID.c_str(),
                    m_strDiskFileExtension.c_str(),
                    volumeInfo.strTaskID.c_str(),
                    volumeInfo.strParentTaskID.c_str());
                return false;
            }
        }
        COMMLOG(OS_LOG_ERROR,
            "The previous full level disk copy '%s' does not exist, task '%s', parent task '%s'!",
            m_strDiskFileExtension.c_str(),
            volumeInfo.strTaskID.c_str(),
            volumeInfo.strParentTaskID.c_str());
        return false;
    } else if (backupLevel == STORAGEDEVICE_BACKUPLEVEL::FULL) {
        // overwrite disk file
        *file = fopen(strDiskFile.c_str(), "wb");
        if (*file == NULL) {
            COMMLOG(OS_LOG_ERROR,
                "Unable to open disk file '%s%s', task '%s', parent task '%s'.",
                volumeInfo.strDiskID.c_str(),
                m_strDiskFileExtension.c_str(),
                volumeInfo.strTaskID.c_str(),
                volumeInfo.strParentTaskID.c_str());
            return false;
        }
    }

    // the backup level only can be incr or full
    mp_bool isValidLevel = (backupLevel == STORAGEDEVICE_BACKUPLEVEL::INCR) ||
                           (backupLevel == STORAGEDEVICE_BACKUPLEVEL::FULL);
    return isValidLevel ? true : false;
}

mp_bool FileStorageDevice::GenerateDiskDescFile(
    mp_uint64 backupLevel, const vmware_volume_info& volumeInfo, const mp_string& folder)
{
    // generate disk desc file absooute path
    mp_string strDiskDescFile = folder + FileStorageDevice::m_strPathSeparatorIdentifier + volumeInfo.strDiskID +
                                m_strDiskDescFileExtension;

    FILE* pDiskDescFile = fopen(strDiskDescFile.c_str(), "wb");  // open file with write permission
    if (pDiskDescFile == NULL) {
        COMMLOG(OS_LOG_ERROR,
            "Unable to open disk desc file '%s%s', task '%s', parent task '%s'.",
            volumeInfo.strDiskID.c_str(),
            m_strDiskDescFileExtension.c_str(),
            volumeInfo.strTaskID.c_str(),
            volumeInfo.strParentTaskID.c_str());
        return false;
    }
    mp_string strContent;
    GenerateDescFileContent(volumeInfo, strContent);
    size_t len = fwrite(strContent.c_str(), strContent.length(), 1, pDiskDescFile);
    if (len == 1) {
        fflush(pDiskDescFile);
        COMMLOG(OS_LOG_INFO,
            "Write disk desc file '%s%s' successfully, task '%s', parent task '%s'.",
            volumeInfo.strDiskID.c_str(),
            m_strDiskDescFileExtension.c_str(),
            volumeInfo.strTaskID.c_str(),
            volumeInfo.strParentTaskID.c_str());
    } else {
        COMMLOG(OS_LOG_ERROR,
            "Write desc file '%s%s' failure, task '%s', parent task '%s'.",
            volumeInfo.strDiskID.c_str(),
            m_strDiskDescFileExtension.c_str(),
            volumeInfo.strTaskID.c_str(),
            volumeInfo.strParentTaskID.c_str());
    }

    if (pDiskDescFile != NULL) {
        fclose(pDiskDescFile);
        pDiskDescFile = NULL;
    }

    return true;
}