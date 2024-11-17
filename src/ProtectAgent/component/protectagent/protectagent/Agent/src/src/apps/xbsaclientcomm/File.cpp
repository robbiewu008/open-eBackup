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
#include "xbsaclientcomm/File.h"

#include <cstdlib>
#include <libgen.h>
#include "xbsaclientcomm/ThriftClientMgr.h"
#include "common/ConfigXmlParse.h"
using namespace std;

namespace {
const unsigned int UMASK_PRIVILEGE = 0022;
const unsigned int GET_FIL_RETRY_TIMES = 3;
};

File::File()
{}

File::~File()
{
    Close();
}

int File::ConnectArchive(long bsaHandle, BsaHandleContext &context)
{
    mp_int32 iret = MP_SUCCESS;

    DBGLOG("[bsaHandle:%ld] get data from archive.", bsaHandle);
    context.streamService.Init(context.workingObj.archiveBackupId, std::to_string(bsaHandle), "");
    std::string archiveServerIp = context.workingObj.archiveServerIp;
    int archiveServerPort = context.workingObj.archiveServerPort;
    bool openSSL = true;
    if (context.workingObj.archiveOpenSSL == 0) {
        openSSL = false;
    }

    DBGLOG("[bsaHandle:%ld] get data from archive  archiveServerIp:%s, archiveServerPort:%d  openssl:%d.",
        bsaHandle, archiveServerIp.c_str(), archiveServerPort, openSSL);
    iret = context.streamService.Connect(archiveServerIp, archiveServerPort, openSSL);

    return iret;
}

EXTER_ATTACK int File::ReadFromArchive(long bsaHandle, BSA_DataBlock32 *dataBlockPtr, BsaHandleContext &context)
{
    if (dataBlockPtr == nullptr) {
        ERRLOG("Data block is nullptr.");
        return MP_FAILED;
    }
    ArchiveStreamGetFileRsq &fileInfo = context.workingObj.stFileInfo;
    if (fileInfo.fileSize < dataBlockPtr->bufferLen && fileInfo.readEnd != 1) {
        ArchiveStreamGetFileReq stGetFileFromArchiveReq;
        fileInfo.offset = fileInfo.offset - fileInfo.fileSize;
        fileInfo.fileSize = 0;
        DBGLOG("current offset:%d", fileInfo.offset);
        stGetFileFromArchiveReq.fileOffset = fileInfo.offset;
        stGetFileFromArchiveReq.taskID = std::to_string(bsaHandle);
        stGetFileFromArchiveReq.fsID = context.workingObj.fsID;
        stGetFileFromArchiveReq.filePath = context.workingObj.storgePath;
        mp_int32 readSize;
        mp_int32 maxResponseSize;
        CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_READ_FILE_SIZE, readSize); // 文件大小写成枚举值
        CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_READ_FILE_RESPONSE_SIZE, maxResponseSize);
        stGetFileFromArchiveReq.readSize = readSize;
        stGetFileFromArchiveReq.maxResponseSize = maxResponseSize;
        for (int i = 0; i < GET_FIL_RETRY_TIMES; i++) {
            if (context.streamService.GetFileData(stGetFileFromArchiveReq, fileInfo)
                != MP_SUCCESS) {
                ERRLOG("[bsaHandle:%ld] get data from archive failed.", bsaHandle);
                continue;
            }
            break;
        }
    }
    dataBlockPtr->numBytes = min((mp_int64)dataBlockPtr->bufferLen, (mp_int64)fileInfo.fileSize);
    
    mp_int32 ret = MP_SUCCESS;
    char *p = (char*)dataBlockPtr->bufferPtr + dataBlockPtr->headerBytes;
    memcpy_s(p, dataBlockPtr->bufferLen, fileInfo.data, dataBlockPtr->numBytes);
    fileInfo.data = fileInfo.data + dataBlockPtr->numBytes;
    fileInfo.fileSize = fileInfo.fileSize - dataBlockPtr->numBytes;
    DBGLOG("[bsaHandle:%ld],  bufferLen:%d, numBytes:%d, fileSize:%d, offset:%d, storgePath：%s",
        bsaHandle, dataBlockPtr->bufferLen, dataBlockPtr->numBytes,
        fileInfo.fileSize, fileInfo.offset, context.workingObj.storgePath.c_str());
    return ret;
}

int File::OpenForWrite(long bsaHandle, const std::string &storgePath)
{
    if (GetWriteStatus() == FileIoStatus::CLOSE) {
        if (!FileExist(storgePath)) {
            if (CreateFile(storgePath) != MP_SUCCESS) {
                ERRLOG("[bsaHandle:%ld] storgePath '%s' does not exist.", bsaHandle, storgePath.c_str());
                return MP_FAILED;
            }
        }
    }

    return MP_SUCCESS;
}

int File::OpenForWriteWithSameFile(long bsaHandle, const std::string &storgePath, mp_string writeType)
{
    INFOLOG("Start to open file for write with same file");
    if (GetWriteStatus() == FileIoStatus::CLOSE) {
        if (!FileExist(storgePath)) {
            INFOLOG("Start to open file for write with same file, file not exist");
            if (CreateFile(storgePath) != MP_SUCCESS) {
                ERRLOG("[bsaHandle:%ld] storgePath '%s' does not exist.", bsaHandle, storgePath.c_str());
                return MP_FAILED;
            }
        } else {
            INFOLOG("Start to open file for write with same file, file exist");
            if (CreateFileWhenFileNotExist(storgePath, writeType) != MP_SUCCESS) {
                ERRLOG("[bsaHandle:%ld] storgePath '%s' does not exist.", bsaHandle, storgePath.c_str());
                return MP_FAILED;
            }
        }
    }

    return MP_SUCCESS;
}

int File::OpenForRead(long bsaHandle, const std::string &storgePath)
{
    if (GetReadStatus() == FileIoStatus::CLOSE) {
        if (!FileExist(storgePath)) {
            ERRLOG("[bsaHandle:%ld] storgePath=%s not exist,errno=%d.", bsaHandle, storgePath.c_str(), errno);
            return MP_FAILED;
        }

        m_fp = fopen(storgePath.c_str(), "rb");
        if (m_fp == NULL) {
            ERRLOG("[bsaHandle:%ld] Open file=%s failed.errno=%d.", bsaHandle, storgePath.c_str(), errno);
            return MP_FAILED;
        }
    }

    return MP_SUCCESS;
}

int File::FileExist(const std::string &pszFilePath)
{
    struct stat fileStat = {0};
    if (stat(pszFilePath.c_str(), &fileStat) != 0) {
        DBGLOG("Stat fail!errno=%d.", errno);
        return MP_FALSE;
    }

    // 目录返回false
    if (S_ISDIR(fileStat.st_mode)) {
        DBGLOG("File is dir.");
        return MP_FALSE;
    }
    DBGLOG("File size=%llu.", fileStat.st_size);
    return MP_TRUE;
}

int File::CreateFile(const std::string &pszFilePath)
{
    if (pszFilePath.empty()) {
        ERRLOG("File path is empty.");
        return MP_FAILED;
    }

    if (FileExist(pszFilePath)) {
        // 安全消减：新建XBSA对象文件时，该文件肯定不存在，如果存在，可能是软链接提权攻击，需报错
        ERRLOG("File exist,not allowed.");
        return MP_FAILED;
    }

    mp_string pFilePath = pszFilePath;
    mp_string pDirPath = dirname(const_cast<char*>(pszFilePath.c_str()));

    umask(UMASK_PRIVILEGE);
    mp_string strCmd = "mkdir -p " + pDirPath; // 创建挂载目录有点长

    std::vector<std::string> strEcho;
    mp_int32 ret = ExecSystemCmd(strCmd, strEcho);
    if (ret != MP_SUCCESS) {
        ERRLOG("exec cmd(%s) fail.", strCmd.c_str());
        return ret;
    }

    m_fp = fopen(pFilePath.c_str(), "ab+");
    if (m_fp == NULL) {
        ERRLOG("Open file %s failed, errno[%d]:%s.", pFilePath.c_str(), errno, strerror(errno));
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int File::CreateFileWhenFileNotExist(const std::string &pszFilePath, mp_string writeType)
{
    if (pszFilePath.empty()) {
        ERRLOG("File path is empty.");
        return MP_FAILED;
    }

    mp_string pFilePath = pszFilePath;

    m_fp = fopen(pFilePath.c_str(), writeType.c_str());
    if (m_fp == NULL) {
        ERRLOG("Open file %s failed, errno[%d]:%s.", pFilePath.c_str(), errno, strerror(errno));
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int File::Read(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    char *p = (char*)dataBlockPtr->bufferPtr + dataBlockPtr->headerBytes;
    size_t rc = fread((void*)p, sizeof(char), dataBlockPtr->bufferLen, m_fp);

    dataBlockPtr->numBytes = rc;

    DBGLOG("[bsaHandle:%ld] readData info, bufferLen: %u, headerBytes: %u numBytes: %u",
        bsaHandle,
        dataBlockPtr->bufferLen,
        dataBlockPtr->headerBytes,
        rc);
    mp_int32 ret = MP_SUCCESS;
    return ret;
}

int File::Write(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    char *p = (char*)dataBlockPtr->bufferPtr + dataBlockPtr->headerBytes;
    size_t rc = fwrite((void*)p, sizeof(char), dataBlockPtr->numBytes, m_fp);
    if (rc != dataBlockPtr->numBytes) {
        ERRLOG("[bsaHandle:%ld] numBytes:%u, not written bytes:%u",
            bsaHandle,
            dataBlockPtr->numBytes,
            dataBlockPtr->numBytes - rc);
        Close();
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int File::ExecSystemCmd(const std::string& strCommand, std::vector<std::string>& strEcho)
{
    FILE* pStream = popen(strCommand.c_str(), "r");
    if (NULL == pStream) {
        ERRLOG("popen failed.");
        return MP_FAILED;
    }

    while (!feof(pStream)) {
        char tmpBuf[1600] = {0};
        fgets(tmpBuf, sizeof(tmpBuf), pStream);
        if (strlen(tmpBuf) >= 0) {
            tmpBuf[strlen(tmpBuf) - 1] = 0;
        }

        bool bFlag = (tmpBuf[0] == 0) || (tmpBuf[0] == '\n');
        if (bFlag) {
            continue;
        }
        strEcho.push_back(string(tmpBuf));
    }

    for (size_t i = 0; i < strEcho.size(); ++i) {
        DBGLOG("command echo is:%s", strEcho[i].c_str());
    }

    (void)pclose(pStream);
    return MP_SUCCESS;
}

void File::Close()
{
    if (m_fp != NULL) {
        fclose(m_fp);
        m_fp = NULL;
    }
}

void File::SetWriteStatus(const FileIoStatus &status)
{
    m_writeStatus = status;
}

void File::SetReadStatus(const FileIoStatus &status)
{
    m_readStatus = status;
}

FileIoStatus File::GetWriteStatus()
{
    return m_writeStatus;
}

FileIoStatus File::GetReadStatus()
{
    return m_readStatus;
}

time_t File::GetLastTime()
{
    return m_lastTime;
}

void File::UpdateLastTime()
{
    m_lastTime = CMpTime::GetTimeSec();
}

time_t File::GetNowTime()
{
    return CMpTime::GetTimeSec();
}