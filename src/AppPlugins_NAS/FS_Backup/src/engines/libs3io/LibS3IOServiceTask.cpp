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
#include "LibS3IOServiceTask.h"
#include <ctime>
#include "log/Log.h"
#include "system/System.hpp"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace {
    const int THREAD_COUNT = 2;
}

void LibS3IOServiceTask::Exec()
{
    if (m_bufferMapPtr == nullptr) {
        return;
    }
    switch (m_event) {
        case LibS3IOEvent::READ_STRING: {
            HandleReadString();
            break;
        }
        case LibS3IOEvent::READ_FILE: {
            HandleReadFile();
            break;
        }
        case LibS3IOEvent::READ_DATA: {
            HandleReadData();
            break;
        }
        case LibS3IOEvent::READ_DATA_BY_OFFSET: {
            HandleReadDataByOffset();
            break;
        }
        case LibS3IOEvent::WRITE_FILE: {
            HandleWriteFile();
            break;
        }
        case LibS3IOEvent::WRITE_DATA: {
            HandleWriteData();
            break;
        }
        case LibS3IOEvent::DELETE: {
            HandleDelete();
            break;
        }
        case LibS3IOEvent::DELETE_ALL: {
            HandleDeleteAll();
            break;
        }
        default:
            break;
    }
}

void LibS3IOServiceTask::HandleReadString()
{
    const string remoteFile = m_params.s3PathPrefix + m_params.s3WorkPath + m_fileHandle.m_file->m_fileName;
    string buffer;
    if (!m_io->ReadFile(remoteFile, buffer)) {
        ERRLOG("read string failed from object %s", remoteFile.c_str());
        m_result = FAILED;
        return;
    }

    m_fileHandle.m_block.m_buffer = new uint8_t[buffer.length()];
    if (memcpy_s(m_fileHandle.m_block.m_buffer, buffer.length(), buffer.c_str(), buffer.length()) != 0) {
        ERRLOG("memcpy failed failed for %s", remoteFile.c_str());
        m_result = FAILED;
        return;
    }
    m_result = SUCCESS;
    return;
}

void LibS3IOServiceTask::HandleReadFile()
{
    const string remoteFile = m_fileHandle.m_file->m_fileName;
    const string localFile = m_params.localPath + m_fileHandle.m_file->m_fileName;
    if (!m_io->ReadFile(remoteFile, localFile)) {
        ERRLOG("read file failed for object %s", remoteFile.c_str());
        m_result = FAILED;
        return;
    }
    m_result = SUCCESS;
    return;
}

void LibS3IOServiceTask::HandleReadData()
{
    BinaryData binData;
    binData.data = m_fileHandle.m_block.m_buffer;
    binData.length = m_fileHandle.m_block.m_size;
    const string remoteFile = m_fileHandle.m_file->m_fileName;
    int cnt = m_io->ReadFile(remoteFile, binData);
    if (cnt < 0) {
        ERRLOG("read data failed from object %s", remoteFile.c_str());
        m_result = FAILED;
        return;
    }
    m_fileHandle.m_block.m_size = cnt;
    m_result = SUCCESS;
    return;
}

void LibS3IOServiceTask::HandleReadDataByOffset()
{
    uint64_t offset = m_fileHandle.m_block.m_offset;
    BinaryData binData;
    binData.data = m_fileHandle.m_block.m_buffer;
    binData.length = m_fileHandle.m_block.m_size;
    const string remoteFile = m_fileHandle.m_file->m_fileName;
    int cnt = m_io->ReadFile(remoteFile, offset, binData);
    if (cnt < 0) {
        ERRLOG("read data failed from object %s, offset %d", remoteFile.c_str(), offset);
        m_result = FAILED;
        return;
    }
    m_fileHandle.m_block.m_size = cnt;
    m_result = SUCCESS;
    return;
}

void LibS3IOServiceTask::HandleWriteFile()
{
    const string remoteFile = m_fileHandle.m_file->m_fileName;
    const string localFile = m_params.localPath + m_fileHandle.m_file->m_fileName;
    bool res = m_io->WriteFile(localFile, remoteFile, THREAD_COUNT, m_params.cbHandle);
    if (!res) {
        ERRLOG("write file failed from local file %s to remote object %s", localFile.c_str(), remoteFile.c_str());
        m_result = FAILED;
        return;
    }
    m_result = SUCCESS;
    return;
}

void LibS3IOServiceTask::HandleWriteData()
{
    BinaryData binData;
    binData.data = m_fileHandle.m_block.m_buffer;
    binData.length = m_fileHandle.m_block.m_size;
    const string remoteFile = m_fileHandle.m_file->m_fileName;
    bool res = m_io->WriteFile(binData, remoteFile, m_params.cbHandle);
    if (!res) {
        ERRLOG("write data failed to remote object %s", remoteFile.c_str());
        m_result = FAILED;
        return;
    }
    m_result = SUCCESS;
    return;
}

void LibS3IOServiceTask::HandleDelete()
{
    const string remotePath = m_fileHandle.m_file->m_fileName;
    bool res = m_io->Delete(remotePath);
    if (!res) {
        ERRLOG("delete failed for object %s", remotePath.c_str());
        m_result = FAILED;
        return;
    }
    m_result = SUCCESS;
    return;
}

void LibS3IOServiceTask::HandleDeleteAll()
{
    const string remotePath = m_fileHandle.m_file->m_fileName;
    bool res = m_io->DeleteAll(remotePath, m_params.cbHandle);
    if (!res) {
        ERRLOG("delete all failed for object %s", remotePath.c_str());
        m_result = FAILED;
        return;
    }
    m_result = SUCCESS;
    return;
}