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
#include "dataprocess/ioscheduler/FileIOEngine.h"
#include <thread>
#include "common/Log.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "dataprocess/vmwarenative/VMwareDiskApiDefine.h"
#include "dataprocess/vmwarenative/Define.h"
namespace {
const mp_string NAS_MNT_POINT = "/opt/advbackup/vmware/data/";
const mp_string DISK_FILE_EXT = "-flat.vmdk";
const mp_string DISK_DESC_EXT = ".vmdk";
const mp_int32 SECTOR_SIZE = 512;
const mp_int32 INCR = 1;
const mp_int32 FULL = 2;
const mp_int32 NO_CHANGE_USER_GROUP_ID = -1;
const mp_string IGNORE_BLOCK = "RecoverIgnoreBadBlock";
}

mp_string FileIOEngine::GetFileName()
{
    mp_string fileName = NAS_MNT_POINT + m_volInfo.strTaskID + PATH_SEPARATOR + m_volInfo.strBackupedDiskID +
                         PATH_SEPARATOR + m_volInfo.strBackupedDiskID + DISK_FILE_EXT;
    return fileName;
};

mp_string FileIOEngine::GetFileNameForWrite()
{
    mp_string fileName = NAS_MNT_POINT + m_volInfo.strTaskID + PATH_SEPARATOR + m_volInfo.strDiskID +
                         PATH_SEPARATOR + m_volInfo.strDiskID + DISK_FILE_EXT;
    return fileName;
};

mp_int32 FileIOEngine::OpenForRead()
{
    CConfigXmlParser::GetInstance().GetValueString(CFG_DATAPROCESS_SECTION, IGNORE_BLOCK, m_ifIgnoreBadBlock);
    mp_string diskFile = NAS_MNT_POINT + m_volInfo.strTaskID + PATH_SEPARATOR + m_volInfo.strBackupedDiskID +
                         PATH_SEPARATOR + m_volInfo.strBackupedDiskID + DISK_FILE_EXT;
    if (!CMpFile::FileExist(diskFile)) {
        COMMLOG(OS_LOG_ERROR, "Disk file '%s' not exist.", diskFile.c_str());
        return MP_FAILED;
    }
    m_fp = fopen(diskFile.c_str(), "rb");
    if (m_fp == NULL) {
        COMMLOG(OS_LOG_ERROR, "Open disk file '%s' failed.", diskFile.c_str());
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "Open disk file '%s' success.", diskFile.c_str());
    return MP_SUCCESS;
}

mp_int32 FileIOEngine::OpenForNFSRead()
{
    mp_string diskFile = m_volInfo.nasIOFlag + m_volInfo.strDiskRelativePath;
    mp_string diskflatFile;
    mp_string diskvmdkFile;
    int posVmdk = diskFile.find(DISK_DESC_EXT);
    if (posVmdk == std::string::npos) {
        COMMLOG(OS_LOG_ERROR, "Disk file '%s' not correct.", diskFile.c_str());
        return MP_FAILED;
    }

    int posFlat = diskFile.find(DISK_FILE_EXT);
    if (posFlat == std::string::npos) {
        diskvmdkFile = diskFile;
        diskflatFile = diskFile.replace(posVmdk, DISK_DESC_EXT.length(), DISK_FILE_EXT);
    } else {
        diskflatFile = diskFile;
        diskvmdkFile = diskFile.replace(posFlat, DISK_FILE_EXT.length(), DISK_DESC_EXT);
    }

    if (CMpFile::FileExist(diskflatFile)) {
        m_fp = fopen(diskflatFile.c_str(), "rb");
        if (m_fp == NULL) {
            COMMLOG(OS_LOG_ERROR, "Open disk file '%s' failed.", diskflatFile.c_str());
            return MP_FAILED;
        }
    } else {
        COMMLOG(OS_LOG_DEBUG, "flat file '%s' not exist.", diskflatFile.c_str());
        if (!CMpFile::FileExist(diskvmdkFile)) {
            COMMLOG(OS_LOG_ERROR, "Open disk file '%s' failed.", diskvmdkFile.c_str());
            return MP_FAILED;
        }
        m_fp = fopen(diskvmdkFile.c_str(), "rb");
        if (m_fp == NULL) {
            COMMLOG(OS_LOG_ERROR, "Open disk file '%s' failed.", diskvmdkFile.c_str());
            return MP_FAILED;
        }
    }
    COMMLOG(OS_LOG_DEBUG, "Open disk file '%s' success.", diskFile.c_str());
    return MP_SUCCESS;
}

EXTER_ATTACK mp_int32 FileIOEngine::OpenForWrite()
{
    mp_int32 deeGroupId = NO_CHANGE_USER_GROUP_ID;
    if (CConfigXmlParser::GetInstance().GetValueInt32(CFG_DATAPROCESS_SECTION, CFG_DEE_GID, deeGroupId) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "parse dee gid config failed, set default value %d.", deeGroupId);
        return MP_FAILED;
    }
    mp_string folder = NAS_MNT_POINT + m_volInfo.strTaskID + PATH_SEPARATOR + m_volInfo.strDiskID;
    if (!CMpFile::DirExist(folder.c_str())) {
        if (CMpFile::CreateDir(folder.c_str()) != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Mountpoint '%s' does not exist, but create folder failure.", folder.c_str());
            return MP_FAILED;
        }
        if (ChownFile(folder, NO_CHANGE_USER_GROUP_ID, deeGroupId) != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "chowd folder failed, folder %s.", folder.c_str());
            return MP_FAILED;
        }
        if (ChmodFile(folder, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP) != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "chmod folder failed, folder %s.", folder.c_str());
            return MP_FAILED;
        }
    }

    mp_string diskFile = folder + PATH_SEPARATOR + m_volInfo.strDiskID + DISK_FILE_EXT;
    if (CreateDiskFile(diskFile, deeGroupId) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Disk file '%s' does not exist, but create file failure.", diskFile.c_str());
        return MP_FAILED;
    }

    if (m_volInfo.vecDirtyRange.empty()) {
        COMMLOG(OS_LOG_INFO, "No need to open file '%s'.", diskFile.c_str());
        return MP_SUCCESS;
    }

    m_fp = fopen(diskFile.c_str(), "rb+");
    if (m_fp == NULL) {
        COMMLOG(OS_LOG_ERROR, "Open disk copy file '%s' failed.", diskFile.c_str());
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "Open disk file '%s' success.", diskFile.c_str());
    return MP_SUCCESS;
}

mp_int32 FileIOEngine::CreateDiskFile(const mp_string &diskFile, mp_int32 deeGroupId)
{
    if (CMpFile::FileExist(diskFile)) {
        mp_uint64 diskSize = 0;
        if (CMpFile::FileSize(diskFile.c_str(), diskSize) != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get disk file size failed, disk: %s", diskFile.c_str());
            return MP_FAILED;
        }
        if (m_volInfo.ulDiskSize == diskSize) {
            return MP_SUCCESS;
        }
        COMMLOG(OS_LOG_WARN, "Disk file size changed, need recreate, disk: %s, old disk size: %ld, new disk size: %ld",
                diskFile.c_str(), diskSize, m_volInfo.ulDiskSize);
    }

    if (CMpFile::CreateFile(diskFile) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Create file '%s' failure.", diskFile.c_str());
        return MP_FAILED;
    }
    // ensure file size as expected
    if (truncate64(diskFile.c_str(), m_volInfo.ulDiskSize) != 0) {
        COMMLOG(OS_LOG_ERROR, "Extend file '%s' to size '%llu' failed, errno[%d].",
            diskFile.c_str(), m_volInfo.ulDiskSize, errno);
        return MP_FAILED;
    }

    if (ChownFile(diskFile, NO_CHANGE_USER_GROUP_ID, deeGroupId) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "chmod file failed, folder %s.", diskFile.c_str());
        return MP_FAILED;
    }
    if (ChmodFile(diskFile, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "chmod file failed, file %s.", diskFile.c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 FileIOEngine::Open()
{
    switch (m_protectType) {
        case VMWARE_VM_BACKUP:
            return OpenForWrite();
        case VMWARE_VM_RECOVERY:
            return OpenForRead();
        case VMWARE_VM_NFS_BACKUP:
            return OpenForNFSRead();
        default:
            COMMLOG(OS_LOG_WARN, "Unknown protect type '%d'.", m_protectType);
            return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 FileIOEngine::Close()
{
    if (m_fp == NULL) {
        COMMLOG(OS_LOG_WARN, "Disk handle is null");
        return MP_SUCCESS;
    }

    fflush(m_fp);
    fsync(fileno(m_fp));

    if (fclose(m_fp) != 0) {
        COMMLOG(OS_LOG_ERROR, "Close file failed, error: '%s'", strerror(errno));
        return MP_FAILED;
    }
    m_fp = NULL;
    COMMLOG(OS_LOG_DEBUG, "Close disk file successfully");
    return MP_SUCCESS;
}

mp_int32 FileIOEngine::Read(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, unsigned char* buffer)
{
    if (m_fp == NULL) {
        COMMLOG(OS_LOG_ERROR, "Disk handle is null");
        return MP_FAILED;
    }

    return RetryOp(
        std::bind(&FileIOEngine::DoRead, this, std::cref(offsetInBytes), std::ref(bufferSizeInBytes), buffer));
}

mp_int32 FileIOEngine::Write(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, unsigned char* buffer)
{
    if (m_fp == NULL) {
        COMMLOG(OS_LOG_ERROR, "Disk handle is null");
        return MP_FAILED;
    }

    return RetryOp(
        std::bind(&FileIOEngine::DoWrite, this, std::cref(offsetInBytes), std::ref(bufferSizeInBytes), buffer));
}

mp_int32 FileIOEngine::DoRead(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, unsigned char* buffer)
{
    if (fseek(m_fp, offsetInBytes, SEEK_SET) != 0) {
        COMMLOG(OS_LOG_ERROR,
            "Seek file to '%llu' failed, err: '%d:%s', task '%s', parent task '%s'",
            offsetInBytes,
            errno,
            strerror(errno),
            m_volInfo.strTaskID.c_str(),
            m_volInfo.strParentTaskID.c_str());
        return MP_FAILED;
    }
    size_t rc = fread(buffer, bufferSizeInBytes, 1, m_fp);
    if (rc != 1) {
        if (m_protectType == VMWARE_VM_RECOVERY && m_ifIgnoreBadBlock == "yes") {
            COMMLOG(OS_LOG_ERROR, "Skip bad block, offsetInBytes: %d, blockSize: %d", offsetInBytes, bufferSizeInBytes);
            return MP_SUCCESS;
        }
        COMMLOG(OS_LOG_ERROR,
            "Read file failed, rc: %llu, bytes to read: %llu, err: '%d:%s', task '%s', parent task '%s'.",
            rc,
            bufferSizeInBytes,
            errno,
            strerror(errno),
            m_volInfo.strTaskID.c_str(),
            m_volInfo.strParentTaskID.c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 FileIOEngine::DoWrite(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, unsigned char* buffer)
{
    if (fseek(m_fp, offsetInBytes, SEEK_SET) != 0) {
        COMMLOG(OS_LOG_ERROR,
            "Seek file to '%llu' failed, err: '%s', task '%s', parent task '%s'",
            offsetInBytes,
            strerror(errno),
            m_volInfo.strTaskID.c_str(),
            m_volInfo.strParentTaskID.c_str());
        return MP_FAILED;
    }
    size_t rc = fwrite(buffer, bufferSizeInBytes, 1, m_fp);
    const char* bufferPtr = reinterpret_cast<const char*>(buffer);
    COMMLOG(OS_LOG_DEBUG, "fwrite result %d: offset: %d, bufferSizeInBytes: %ld, buffersize: %ld, buffer: %p,'%s'", rc,
        offsetInBytes, bufferSizeInBytes, strlen(bufferPtr), buffer, m_volInfo.strTaskID.c_str());
    if (rc != 1) {
        COMMLOG(OS_LOG_ERROR,
            "Write file failed, err: '%s', task '%s', parent task '%s'",
            strerror(errno),
            m_volInfo.strTaskID.c_str(),
            m_volInfo.strParentTaskID.c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 FileIOEngine::RetryOp(std::function<mp_int32()> internalOp)
{
    mp_int32 res = MP_FAILED;
    int retryN = 1;
    const mp_int32 nInterval = 2;
    const mp_int32 nRetries = 3;
    while (retryN <= nRetries) {
        res = internalOp();
        if (res == MP_SUCCESS) {
            break;
        }
        retryN++;
        SleepFor(std::chrono::seconds(nInterval));
    }

    return res;
}

// 备份任务完成后生成磁盘描述文件
mp_int32 FileIOEngine::PostBackup()
{
    COMMLOG(OS_LOG_DEBUG, "Doing post work for task: '%s'", m_volInfo.strTaskID.c_str());
    return RetryOp(std::bind(&FileIOEngine::GenerateDiskDescFile, this));
}

mp_void FileIOEngine::GenerateDescFileContent(mp_string& strContent)
{
    std::ostringstream strParam;
    strParam << "# Disk DescriptorFile" << STR_CODE_WARP << "version" << STR_EQUAL
             << std::to_string(m_volInfo.descFileInfo.iVersion) << STR_CODE_WARP << "encoding" << STR_EQUAL
             << STR_QUOTATION_MARK << m_volInfo.descFileInfo.strEncode << STR_QUOTATION_MARK << STR_CODE_WARP << "CID"
             << STR_EQUAL << m_volInfo.descFileInfo.strCID << STR_CODE_WARP << "parentCID" << STR_EQUAL
             << m_volInfo.descFileInfo.strParentCID << STR_CODE_WARP << "createType" << STR_EQUAL << STR_QUOTATION_MARK
             << m_volInfo.descFileInfo.strCreateType << STR_QUOTATION_MARK << STR_CODE_WARP << STR_CODE_WARP
             << "# Extent description" << STR_CODE_WARP << "RW " << std::to_string(m_volInfo.ulDiskSize / SECTOR_SIZE)
             << STR_SPACE << "VMFS " << STR_QUOTATION_MARK << m_volInfo.strDiskID + DISK_FILE_EXT << STR_QUOTATION_MARK
             << STR_CODE_WARP << STR_CODE_WARP << "# The Disk Data Base" << STR_CODE_WARP << "#DDB" << STR_CODE_WARP
             << STR_CODE_WARP << " ddb.adapterType " << STR_EQUAL << STR_SPACE << STR_QUOTATION_MARK
             << m_volInfo.descFileInfo.strAdapterType << STR_QUOTATION_MARK << STR_CODE_WARP
             << " ddb.geometry.cylinders " << STR_EQUAL << STR_SPACE << STR_QUOTATION_MARK
             << m_volInfo.descFileInfo.strCylinders << STR_QUOTATION_MARK << STR_CODE_WARP << " ddb.geometry.heads "
             << STR_EQUAL << STR_SPACE << STR_QUOTATION_MARK << m_volInfo.descFileInfo.strHeads << STR_QUOTATION_MARK
             << STR_CODE_WARP << " ddb.geometry.sectors " << STR_EQUAL << STR_SPACE << STR_QUOTATION_MARK
             << m_volInfo.descFileInfo.strSectors << STR_QUOTATION_MARK << STR_CODE_WARP << " ddb.longContentID "
             << STR_EQUAL << STR_SPACE << STR_QUOTATION_MARK << m_volInfo.descFileInfo.strLogContentID
             << STR_QUOTATION_MARK << STR_CODE_WARP << " ddb.uuid " << STR_EQUAL << STR_SPACE << STR_QUOTATION_MARK
             << m_volInfo.strDiskID << STR_QUOTATION_MARK << STR_CODE_WARP << " ddb.virtualHWVersion " << STR_EQUAL
             << STR_SPACE << STR_QUOTATION_MARK << m_volInfo.descFileInfo.strVirtualHWVersion << STR_QUOTATION_MARK
             << STR_CODE_WARP;
    strContent = strParam.str();
}

EXTER_ATTACK mp_int32 FileIOEngine::GenerateDiskDescFile()
{
    // generate disk desc file absooute path
    mp_string folder = NAS_MNT_POINT + m_volInfo.strTaskID + PATH_SEPARATOR + m_volInfo.strDiskID;
    mp_string strDiskDescFile = folder + PATH_SEPARATOR + m_volInfo.strDiskID + DISK_DESC_EXT;

    FILE* pDiskDescFile = fopen(strDiskDescFile.c_str(), "wb");  // open file with write permission
    if (pDiskDescFile == NULL) {
        COMMLOG(OS_LOG_ERROR,
            "Unable to open disk desc file '%s', task '%s', parent task '%s'.",
            strDiskDescFile.c_str(),
            m_volInfo.strTaskID.c_str(),
            m_volInfo.strParentTaskID.c_str());
        return MP_FAILED;
    }
    mp_int32 deeGroupId = NO_CHANGE_USER_GROUP_ID;
    if (CConfigXmlParser::GetInstance().GetValueInt32(CFG_DATAPROCESS_SECTION, CFG_DEE_GID, deeGroupId) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "parse dee gid config failed, set default value %d.", deeGroupId);
        fclose(pDiskDescFile);
        return MP_FAILED;
    }
    if (ChownFile(strDiskDescFile.c_str(), NO_CHANGE_USER_GROUP_ID, deeGroupId) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "chmod file failed, folder %s.", strDiskDescFile.c_str());
        fclose(pDiskDescFile);
        return MP_FAILED;
    }
    if (ChmodFile(strDiskDescFile, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "chmod file failed, file %s.", strDiskDescFile.c_str());
        fclose(pDiskDescFile);
        return MP_FAILED;
    }
    mp_string strContent;
    GenerateDescFileContent(strContent);
    size_t len = fwrite(strContent.c_str(), strContent.length(), 1, pDiskDescFile);
    if (len == 1) {
        fflush(pDiskDescFile);
        COMMLOG(OS_LOG_INFO,
            "Write disk desc file '%s' successfully, task '%s', parent task '%s'.",
            strDiskDescFile.c_str(),
            m_volInfo.strTaskID.c_str(),
            m_volInfo.strParentTaskID.c_str());
        fclose(pDiskDescFile);
        return MP_SUCCESS;
    } else {
        COMMLOG(OS_LOG_ERROR,
            "Write desc file '%s' failure, task '%s', parent task '%s'.",
            strDiskDescFile.c_str(),
            m_volInfo.strTaskID.c_str(),
            m_volInfo.strParentTaskID.c_str());
        fclose(pDiskDescFile);
        return MP_FAILED;
    }
}