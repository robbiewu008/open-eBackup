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
#include "ArchiveServiceTask.h"
#include <ctime>
#include "log/Log.h"
#include "system/System.hpp"
#include "FSBackupUtils.h"

using namespace std;
using namespace FS_Backup;

namespace {
    const int SUCCESS = 0;
    const int FAILED = -1;
}

// 去除文件名前缀 / . 适配根目录文件的 //a.out 的情况
std::string ArchiveServiceTask::CutPrefixSlash(const std::string& path) const
{
    int offset = 0;
    std::string p;
    for (std::size_t i = 0; i < path.size(); i++) {
        p = path[offset];
        if (p == "/" || p == ".") {
            offset++;
            continue;
        }
        break;
    }
    return path.substr(offset, path.size());
}

void ArchiveServiceTask::Exec()
{
    if (m_bufferMapPtr == nullptr) {
        DBGLOG("Buffer map is nullptr");
        return;
    }
    switch (m_event) {
        case ArchiveEvent::OPEN_SRC: {
            HandleOpenFile();
            break;
        }
        case ArchiveEvent::READ_DATA: {
            HandleReadData();
            break;
        }
        case ArchiveEvent::CLOSE_SRC: {
            HandleCloseSrc();
            break;
        }
        default:
            DBGLOG("Event default");
            break;
    }
}

void ArchiveServiceTask::HandleReadData()
{
    std::string fileName = m_fileHandle.m_file->m_fileName;
    DBGLOG("Enter HandleReadData: %s", fileName.c_str());

    if (FSBackupUtils::IsSymLinkFile(m_fileHandle)) {
        m_result = ProcessReadSoftLinkData();
        return;
    }

    if (FSBackupUtils::IsSpecialFile(m_fileHandle.m_file->m_mode)) {
        m_result = ProcessReadSpecialFileData();
        return;
    }

    if (m_archiveClient == nullptr) {
        ERRLOG("m_archiveClient is nullptr");
        m_result = FAILED;
        return;
    }

    if (m_fileHandle.m_block.m_size == 0) {
        DBGLOG("Skip empty file: %s", fileName.c_str());
        m_result = SUCCESS;
        return;
    }

    fileName = CutPrefixSlash(fileName);

    ArchiveRequest req;
    req.m_fileName = m_params.srcRootPath + "/" + fileName; // no-agg: /source_*_Context/** or agg: /jobid/**
    req.m_offset = m_fileHandle.m_block.m_offset;
    req.m_size = m_fileHandle.m_block.m_size; // size is determined by nfs or cifs
    req.m_buffer = m_fileHandle.m_block.m_buffer; // send buffer to req to get data

    ArchiveResponse rsp;
    int retVal = m_archiveClient->GetFileData(req, rsp);
    if (retVal != SUCCESS) {
        ERRLOG("Get file data from archive failed, file name: %s, offset: %llu, size: %llu, retVal: %d",
            req.m_fileName.c_str(), req.m_offset, req.m_size, retVal);
        m_result = retVal;
        return;
    }
    m_result = SUCCESS;
    DBGLOG("ReadData %s %d %llu success!", fileName.c_str(), m_fileHandle.m_block.m_size,
        m_fileHandle.m_block.m_offset);
    return;
}

int ArchiveServiceTask::ProcessReadSoftLinkData()
{
#ifdef WIN32
    /* windows reading symlink target from XMeta instead if reading from stream */
    return SUCCESS;
#else
    std::string fileName = m_fileHandle.m_file->m_fileName;
    DBGLOG("Enter HandleSoftLinkData: %s", fileName.c_str());
    if (m_archiveClient == nullptr) {
        ERRLOG("m_archiveClient is nullptr");
        return FAILED;
    }

    fileName = CutPrefixSlash(fileName);

    ArchiveRequest req;
    req.m_fileName = m_params.srcRootPath + "/" + fileName; // no-agg: /source_*_Context/** or agg: /jobid/**
    req.m_offset = m_fileHandle.m_block.m_offset;
    req.m_size = m_fileHandle.m_block.m_size; // size is determined by nfs or cifs
    req.m_buffer = m_fileHandle.m_block.m_buffer; // send buffer to req to get data

    ArchiveResponse rsp;
    int ret = m_archiveClient->GetFileData(req, rsp);
    if (ret != SUCCESS) {
        ERRLOG("Get file data from archive failed, file name: %s, offset: %llu, size: %llu",
            req.m_fileName.c_str(), req.m_offset, req.m_size);
        return FAILED;
    }

    uint64_t cnt = rsp.m_size;
    m_fileHandle.m_block.m_size = cnt;
    m_fileHandle.m_block.m_buffer[cnt] = '\0';
    DBGLOG("Readlink name %s success!", m_fileHandle.m_block.m_buffer);
    DBGLOG("Readlink %s %d %llu success!", fileName.c_str(), m_fileHandle.m_block.m_size,
        m_fileHandle.m_block.m_offset);
    return SUCCESS;
#endif
}

int ArchiveServiceTask::ProcessReadSpecialFileData() const
{
    DBGLOG("Enter ProcessReadSpecialFileData: %s", m_fileHandle.m_file->m_fileName.c_str());
    return SUCCESS;
}

void ArchiveServiceTask::HandleOpenFile()
{
    DBGLOG("Enter HandleOpenFile: %s", m_fileHandle.m_file->m_fileName.c_str());
    m_result = SUCCESS;
}

void ArchiveServiceTask::HandleCloseSrc()
{
    DBGLOG("Enter HandleCloseSrc: %s", m_fileHandle.m_file->m_fileName.c_str());
    m_result = SUCCESS;
}
