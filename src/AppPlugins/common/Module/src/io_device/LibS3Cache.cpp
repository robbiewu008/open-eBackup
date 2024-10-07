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
#include "LibS3Cache.h"

#include <iostream>
#include <string>
#include <cstdlib>
#include <algorithm>

#include "system/System.hpp"
#include "log/Log.h"

using namespace std;
using namespace Module;

const int MSG_MAX_LENGTH = 1024;
#define CHECK_RETURN_IF(expr, funname, line, retvalue)                                                  \
    {                                                                                                   \
        if ((expr) != 0) {                                                                              \
            char ctMsg[MSG_MAX_LENGTH + 1] = { 0 };                                                     \
            int len = snprintf_s(ctMsg, sizeof(ctMsg), MSG_MAX_LENGTH, "%s %d failed.", funname, line); \
            if (len <= 0) {                                                                             \
                HCP_Log(ERR, LIBS3) << "snprintf_s failed, return len = " << len << HCPENDLOG;          \
                return retvalue;                                                                        \
            }                                                                                           \
            HCP_Log(ERR, LIBS3) << ctMsg << HCPENDLOG;                                                  \
            return retvalue;                                                                            \
        }                                                                                               \
    }

static char *Renew(void *ptr, size_t oldsize, size_t newsize)
{
    char *oldTemp = (char *)ptr;
    char *newtemp = new (nothrow) char[newsize];

    if (newtemp == nullptr) {
        delete[] oldTemp;
        return nullptr;
    }

    (void)memset_s(newtemp, newsize, 0, newsize);
    int retVal = memcpy_s(newtemp, newsize, oldTemp, oldsize);
    if (retVal != 0) {
        delete[] newtemp;
        newtemp = nullptr;
    }

    delete[] oldTemp;
    oldTemp = nullptr;
    return newtemp;
}

static void *MyRealloc(void *ptr, size_t oldsize, size_t size)
{
    if (ptr != nullptr) {
        return Renew(ptr, oldsize, size);
    } else {
        char *newTmp = new (nothrow) char[size];
        if (newTmp == nullptr) {
            return nullptr;
        }

        (void)memset_s(newTmp, size, 0, size);
        return newTmp;
    }
}

S3IOParams::~S3IOParams()
{
    CleanMemoryPwd(passWord);
    CleanMemoryPwd(cert);
}

size_t Cache::Read(char *buf, size_t bufSize)
{
    size_t readSize = 0;
    if (m_dataSize <= m_offset) {
        HCP_Log(DEBUG, LIBS3) << "m_dataSize: " << m_dataSize << " is less than m_offset: " << m_offset << HCPENDLOG;
        return 0;
    }
    size_t ableReadsize = m_dataSize - m_offset;

    HCP_Log(DEBUG, LIBS3)
            << "m_dataSize: " << m_dataSize << "m_offset: " << m_offset
            << "bufSize: " << bufSize << HCPENDLOG;

    if (ableReadsize < bufSize) {
        readSize = ableReadsize;
    } else {
        readSize = bufSize;
    }

    if (readSize > 0) {
        int retVal = memcpy_s(buf, readSize, &(m_data[m_offset]), readSize);
        CHECK_RETURN_IF(retVal, __FUNCTION__, __LINE__, 0);
        m_offset += readSize;
    }

    return readSize;
}

size_t Cache::ReadFS(char *buf, size_t bufSize)
{
    HCP_Log(DEBUG, LIBS3) << "FLOW: Entered Cache::ReadFS " << HCPENDLOG;
    size_t readSize = 0;
    if (m_dataSize <= m_offset) {
        HCP_Log(DEBUG, LIBS3) << "m_dataSize: " << m_dataSize << " is less than m_offset: " << m_offset << HCPENDLOG;
        return 0;
    }
    size_t ableReadsize = m_dataSize - m_offset;

    HCP_Log(DEBUG, LIBS3)
            << "m_dataSize: " << m_dataSize << "m_offset: " << m_offset
            << "bufSize: " << bufSize << HCPENDLOG;

    if (ableReadsize < bufSize) {
        readSize = ableReadsize;
    } else {
        readSize = bufSize;
    }

    if (readSize > 0) {
        int retVal = memcpy_s(buf, readSize, &(m_data[m_offset]), readSize);
        CHECK_RETURN_IF(retVal, __FUNCTION__, __LINE__, 0);
        m_offset += readSize;
    }

    return readSize;
}

size_t Cache::Write(char *buf, size_t bufSize)
{
    HCP_Log(DEBUG, LIBS3)
            << "m_dataSize: " << m_dataSize << "m_offset: " << m_offset
            << "bufSize: " << bufSize << HCPENDLOG;

    size_t newSize = m_offset + bufSize;
    if (newSize <= m_dataBufferSize) {
        int retVal = memcpy_s(&(m_data[m_offset]),
                              (m_dataBufferSize - m_offset), buf, bufSize);
        CHECK_RETURN_IF(retVal, __FUNCTION__, __LINE__, 0);
        if (newSize > m_dataSize) {
            m_dataSize = newSize;
        }

        m_offset = newSize;
    } else {
        size_t newBufferSize = (newSize + g_dataBlockSize - 1) & ~(g_dataBlockSize - 1);
        m_data = (char *)MyRealloc(m_data, m_dataSize, newBufferSize);
        if (m_data) {
            int retVal = memcpy_s(&(m_data[m_offset]),
                                  (newBufferSize - m_offset), buf, bufSize);
            CHECK_RETURN_IF(retVal, __FUNCTION__, __LINE__, 0);
            if (newSize > m_dataSize) {
                m_dataSize = newSize;
            }

            m_offset = newSize;
            m_dataBufferSize = newBufferSize;
        } else {
            return 0;
        }
    }

    return bufSize;
}

size_t Cache::WriteFS(char* buf, size_t bufSize)
{
    HCP_Log(DEBUG, LIBS3) << "Reached Cache::WriteFS " << HCPENDLOG;
    HCP_Log(DEBUG, LIBS3) << "FLOW: Reached Cache::WriteFS " << HCPENDLOG;
}

size_t Cache::Append(EBKFileHandle *handle, char *buf, size_t bufSize)
{
    HCP_Log(DEBUG, LIBS3)
            << "m_dataSize: " << m_dataSize << "m_offset: " << m_offset
            << "bufSize: " << bufSize << HCPENDLOG;

    size_t newSize = m_dataSize + bufSize;
    if (newSize <= m_dataBufferSize) {
        int retVal = memcpy_s(&(m_data[m_dataSize]),
                              (m_dataBufferSize - m_dataSize), buf, bufSize);
        CHECK_RETURN_IF(retVal, __FUNCTION__, __LINE__, 0);
        m_dataSize += bufSize;
    } else {
        size_t newBufferSize = (newSize + g_dataBlockSize - 1) & ~(g_dataBlockSize - 1);
        m_data = (char *)MyRealloc(m_data, m_dataSize, newBufferSize);
        if (m_data) {
            m_dataBufferSize = newBufferSize;
            int retVal = memcpy_s(&(m_data[m_dataSize]),
                                  (m_dataBufferSize - m_dataSize), buf, bufSize);
            CHECK_RETURN_IF(retVal, __FUNCTION__, __LINE__, 0);
            m_dataSize += bufSize;
        } else {
            return 0;
        }
    }

    return bufSize;
}

void Cache::Truncate(long int pos)
{
    size_t position = size_t(pos);
    if (m_dataSize == position) {
        return;
    }

    if (m_dataSize > position) {
        size_t nNewSize = m_dataSize - position;
        (void)memset_s(&(m_data[position]), nNewSize, 0, nNewSize);
        m_dataSize = position;
    } else {
        if (m_dataBufferSize < position) {
            size_t newBufferSize = (position + g_dataBlockSize - 1) & ~(g_dataBlockSize - 1);
            m_data = (char *)MyRealloc(m_data, m_dataSize, newBufferSize);
            if (m_data) {
                m_dataSize = position;
                m_dataBufferSize = newBufferSize;
            }
        } else {
            size_t nNewSize = position - m_dataSize;
            (void)memset_s(&(m_data[m_dataSize]), nNewSize, 0, nNewSize);
            m_dataSize = position;
        }
    }
}

bool Cache::DumpToLocalFile(string fileName)
{
    bool rv = true;
    FILE *fp = fopen(fileName.c_str(), "w");
    if (fp == nullptr) {
        HCP_Log(ERR, LIBS3) << "open file failed:" << DBG(fileName) << HCPENDLOG;
        return false;
    }
    size_t result = fwrite(m_data, 1, m_dataSize, fp);
    if (result != m_dataSize) {
        HCP_Log(ERR, LIBS3) << "write to file failed:" << DBG(fileName) << DBG(result) << DBG(m_dataSize) << HCPENDLOG;
        rv = false;
    }
    fclose(fp);
    HCP_Log(DEBUG, LIBS3) << "DumpToLocalFile complete." << DBG(fileName) << DBG(rv) << HCPENDLOG;
    return rv;
}

void Cache::Reset()
{
    m_dataSize = 0;
    m_offset = 0;
    m_dataBufferSize = 0;
    if (m_data) {
        delete[] m_data;
        m_data = nullptr;
    }
}

// static callback functions
int PutPartObjectDataCallback(int bufferSize, char *buffer, void *callbackData)
{
    HCP_Log(DEBUG, LIBS3) << "Enter PutObjectDataCallback, bufferSize = "
                          << bufferSize << HCPENDLOG;

    S3Base *s3Obj = static_cast<S3Base *>(callbackData);
    if (s3Obj != nullptr) {
        size_t readSize = s3Obj->PartCallbackRead(buffer, bufferSize);
        HCP_Log(DEBUG, LIBS3) << "Expect bufferSize:" << bufferSize << ",Get object data, size = " << readSize <<
                              HCPENDLOG;
        if (readSize != static_cast<size_t>(bufferSize)) {
            HCP_Log(WARN, LIBS3) << "the expect size if not the same read from buffer! " << HCPENDLOG;
        }
        return static_cast<int>(readSize);
    } else {
        HCP_Log(CRIT, LIBS3) << "PutObjectDataCallback failed. " << HCPENDLOG;
    }

    return -1;
}
bool Comp(const S3ObjectContent &obj1, const S3ObjectContent &obj2)
{
    char partNum1[32] = { 0 };
    char partNum2[32] = { 0 };

    // e.g. ac30b60c-c92a-4594-922f-952ac83d1f0c.snap_1
    const char *pStr = obj1.m_ObjectName.c_str();
    int posStart = obj1.m_ObjectName.find_last_of('_');
    int posEnd = obj1.m_ObjectName.size();
    pStr += (posStart + 1);
    errno_t iRet = strncpy_s(partNum1, sizeof(partNum1), pStr, posEnd - posStart);
    if (iRet != 0) {
        HCP_Log(WARN, LIBS3) << "copy part num1 failed" << HCPENDLOG;
    }

    pStr = obj2.m_ObjectName.c_str();
    posStart = obj2.m_ObjectName.find_last_of('_');
    posEnd = obj2.m_ObjectName.size();
    pStr += (posStart + 1);
    iRet = strncpy_s(partNum2, sizeof(partNum2), pStr, posEnd - posStart);
    if (iRet != 0) {
        HCP_Log(WARN, LIBS3) << "copy part num2 failed" << HCPENDLOG;
    }

    return atol(partNum1) < atol(partNum2);
}
void NoCacheBuffer::Init(char *buffer, const size_t bufferLen)
{
    m_buffer = buffer;
    m_bufferLen = bufferLen;
}

void NoCacheBuffer::SaveOffset()
{
    m_oldDataOffset = m_dataOffset;
    m_oldDataLen = m_dataLen;
    m_oldFileOffset = m_fileOffset;
    m_oldPartNum = m_partNum;
}

void NoCacheBuffer::RecoverOffset()
{
    m_dataOffset = m_oldDataOffset;
    m_dataLen = m_oldDataLen;
    m_fileOffset = m_oldFileOffset;
    m_partNum = m_oldPartNum;
}

ssize_t NoCacheBuffer::CallbackWrite(const char *buf, size_t bufSize)
{
    HCP_Log(DEBUG, LIBS3) << "m_dataOffset:" << m_dataOffset << ",m_fileOffset:" << m_fileOffset << ",m_dataLen:" <<
                          m_dataLen << ",bufSize:" << bufSize << ",pBuf:" << (long)buf << HCPENDLOG;
    size_t bufLeftSize = m_bufferLen - m_dataOffset;
    if (bufLeftSize >= bufSize) {
        if (memcpy_s(&m_buffer[m_dataOffset], bufSize, buf, bufSize) != 0) {
            HCP_Log(ERR, LIBS3) << "memcpy_s failed" << HCPENDLOG;
        }
        m_dataOffset += bufSize;
        m_dataLen += bufSize;
        return bufSize;
    } else if ((bufLeftSize < bufSize) && bufLeftSize != 0) {
        if (memcpy_s(&m_buffer[m_dataOffset], bufSize, buf, bufLeftSize) != 0) {
            HCP_Log(ERR, LIBS3) << "memcpy_s failed" << HCPENDLOG;
        }
        m_dataOffset += bufLeftSize;
        m_dataLen += bufLeftSize;
        return bufLeftSize;
    }

    HCP_Log(ERR, LIBS3) << "m_dataOffset:" << m_dataOffset << ",m_fileOffset:" << m_fileOffset << ",m_dataLen:" <<
                        m_dataLen << HCPENDLOG;

    return 0;
}

size_t NoCacheBuffer::CallbackRead(char *buf, size_t bufSize)
{
    size_t bufLeftSize = m_bufferLen - m_dataOffset;
    if (bufLeftSize >= bufSize) {
        if (memcpy_s(buf, bufSize, &m_buffer[m_dataOffset], bufSize) != 0) {
            HCP_Log(ERR, LIBS3) << "memcpy_s failed" << HCPENDLOG;
        }
        m_dataOffset += bufSize;
        return bufSize;
    } else if ((bufLeftSize < bufSize) && bufLeftSize != 0) {
        if (memcpy_s(buf, bufLeftSize, &m_buffer[m_dataOffset], bufLeftSize) != 0) {
            HCP_Log(ERR, LIBS3) << "memcpy_s failed" << HCPENDLOG;
        }
        m_dataOffset += bufLeftSize;
        return bufLeftSize;
    }

    return 0;
}

bool NoCacheBuffer::CreateHandle(EBKFileHandle *handle)
{
    if (handle == nullptr) {
        HCP_Log(ERR, LIBS3) << "FileHandle must be not null." << HCPENDLOG;
        return false;
    }

    if (!handle->handle.empty()) {
        HCP_Log(DEBUG, LIBS3) << "TaskID is exist.." << HCPENDLOG;
        return true;
    }

    m_partNum = handle->partNum;

    return true;
}

size_t NoCacheBuffer::PartCallbackRead(char *buf, size_t bufSize)
{
    HCP_Log(DEBUG, LIBS3) << "part callback read.bufSize:" << bufSize << HCPENDLOG;
    size_t readLength = 0;
    while (bufSize >= 0 && m_partNum < m_objectInfoList.size()) {
        // read data from buffer first,if read end of the buffer,load next part
        if (partFileContent == nullptr || m_fileOffset == m_dataLen) {
            uint64_t objSize = m_objectInfoList[m_partNum].size;
            partFileContent = shared_ptr<char>(new(nothrow) char[objSize], default_delete<char[]>());
            if (partFileContent == nullptr) {
                HCP_Log(WARN, LIBS3) << "memory alloc failed." << DBG(objSize) << HCPENDLOG;
                return readLength;
            }

            Init(partFileContent.get(), objSize);
            m_dataOffset = 0;
            m_dataLen = 0;
            m_fileOffset = 0;
            S3BucketContextProxy bucketContext(m_s3Params);
            SetObjectKey(m_objectInfoList[m_partNum].m_ObjectName);
            HCP_Log(DEBUG, LIBS3) << "read part file from s3." << DBG(objSize) << DBG(m_partNum) << HCPENDLOG;
            ReadDataFromS3(bucketContext, 0, objSize);
            // ReadDataFromS3 refreshes oldPartNum to latest partNum,
            // It must be reset to 0 here so that it will start from the first part when retrying.
            m_oldPartNum = 0;
        }
        HCP_Log(DEBUG, LIBS3) << DBG(m_dataLen) << DBG(m_fileOffset) << DBG(bufSize) << DBG(readLength) << HCPENDLOG;
        size_t leftDataSize = m_dataLen - m_fileOffset;
        if (leftDataSize >= bufSize) {
            if (memcpy_s(buf + readLength, bufSize, m_buffer + m_fileOffset, bufSize) != 0) {
                HCP_Log(ERR, LIBS3) << "memcpy_s failed" << HCPENDLOG;
            }
            m_fileOffset += bufSize;
            readLength += bufSize;
            break;
        } else {
            if (memcpy_s(buf + readLength, leftDataSize, m_buffer + m_fileOffset, leftDataSize)) {
                HCP_Log(ERR, LIBS3) << "memcpy_s failed" << HCPENDLOG;
            }
            readLength += leftDataSize;
            bufSize -= leftDataSize;
            m_fileOffset += leftDataSize;
            // read end of the part file,reset memory;
            partFileContent.reset();
            m_partNum++;
        }
    }
    return readLength;
}

size_t NoCacheBuffer::WritePartDataToS3(size_t dataLen)
{
    obs_put_object_handler putObjectHandler = {
        { &ResponseCallback, &CompleteCallback },
        &PutPartObjectDataCallback
    };

    S3BucketContextProxy bucketCtx(m_s3Params);
    return WriteDataToS3(bucketCtx, putObjectHandler, m_objKey, dataLen);
}

bool NoCacheBuffer::DeletePartDataFromS3()
{
    S3BucketContextProxy bucketCtx(m_s3Params);
    for (uint64_t i = 0; i < m_matchObjList.size(); i++) {
        if (!RemoveFile(bucketCtx, m_matchObjList[i])) {
            HCP_Log(ERR, LIBS3) << "Delete file from S3 failed.fileName:" << m_matchObjList[i] << HCPENDLOG;
            return false;
        }
    }

    return true;
}

size_t NoCacheBuffer::GetUploadFileData(char *buffer, int dataLen)
{
    return ReadSegement(buffer, dataLen);
}

size_t NoCacheBuffer::ReadSegement(char *buf, size_t bufSize)
{
    HCP_Log(DEBUG, LIBS3) << "m_dataSize: " << m_dataLen << "m_offset: " << m_dataOffset << "bufSize: " << bufSize
                          << HCPENDLOG;

    size_t readSize = 0;
    if (m_dataLen <= m_dataOffset) {
        HCP_Log(DEBUG, LIBS3) << "m_dataSize: " << m_dataLen << " is less than m_offset: " << m_dataOffset << HCPENDLOG;
        return 0;
    }
    size_t ableReadsize = m_dataLen - m_dataOffset;

    if (ableReadsize < bufSize) {
        readSize = ableReadsize;
    } else {
        readSize = bufSize;
    }

    if (readSize > 0) {
        int retVal = memcpy_s(buf, readSize, &(m_buffer[m_dataOffset]), readSize);
        CHECK_RETURN_IF(retVal, __FUNCTION__, __LINE__, 0);
        m_dataOffset += readSize;
        m_fileOffset += readSize;
    }

    return readSize;
}

size_t NoCacheBuffer::WriteSegement(char *buf, size_t bufSize)
{
    HCP_Log(DEBUG, LIBS3) << "m_dataSize: " << m_dataLen << "m_offset: " << m_dataOffset << "bufSize: " << bufSize
                          << HCPENDLOG;

    size_t newSize = m_dataOffset + bufSize;
    if (newSize <= m_bufferLen) {
        int retVal = memcpy_s(&(m_buffer[m_dataOffset]), (m_bufferLen - m_dataOffset), buf, bufSize);
        CHECK_RETURN_IF(retVal, __FUNCTION__, __LINE__, 0);

        m_dataLen += bufSize;
        m_dataOffset += bufSize;
        m_fileOffset += bufSize;

        return bufSize;
    } else {
        size_t dataLen = m_bufferLen - m_dataOffset;
        int retVal = memcpy_s(&(m_buffer[m_dataOffset]), dataLen, buf, dataLen);
        CHECK_RETURN_IF(retVal, __FUNCTION__, __LINE__, 0);

        m_dataOffset += dataLen;
        m_dataLen += dataLen;
        m_fileOffset += dataLen;

        return dataLen;
    }
}

size_t NoCacheBuffer::Read(char *buf, size_t bufSize)
{
    Init(buf, bufSize);
    m_dataLen = 0;
    m_dataOffset = 0;
    S3BucketContextProxy bucketContext(m_s3Params);
    ReadDataFromS3(bucketContext, m_fileOffset, 0);
    if (!CheckStatus()) {
        HCP_Log(ERR, LIBS3) << "Failed to read. bufSize=" << bufSize << HCPENDLOG;
        return 0;
    }

    m_fileOffset += m_dataLen;

    return m_dataLen;
}

size_t NoCacheBuffer::ReadFS(char *buf, size_t bufSize)
{
    HCP_Log(DEBUG, LIBS3) << "FOW: Entered NoCacheBuffer::ReadFS" << HCPENDLOG;
    Init(buf, bufSize);
    m_dataLen = 0;
    m_dataOffset = 0;
    S3BucketContextProxy bucketContext(m_s3Params);
    ReadDataFromS3(bucketContext, m_fileOffset, 0);
    if (!CheckStatus()) {
        HCP_Log(ERR, LIBS3) << "Failed to read. bufSize=" << bufSize << HCPENDLOG;
        return 0;
    }

    m_fileOffset += m_dataLen;

    return m_dataLen;
}

size_t NoCacheBuffer::Write(char *buf, size_t bufSize)
{
    Init(buf, bufSize);
    m_dataLen = bufSize;
    m_dataOffset = 0;
    S3BucketContextProxy bucketContext(m_s3Params);
    size_t dataLen = WriteDataToS3(bucketContext, m_objKey, bufSize);

    m_fileOffset += dataLen;

    return dataLen;
}

size_t NoCacheBuffer::WriteFS(char* buf, size_t bufSize)
{
    HCP_Log(DEBUG, LIBS3) << "FOW: Entered NoCacheBuffer::WriteFS" << HCPENDLOG;
    Init(buf, bufSize);
    m_dataLen = bufSize;
    m_dataOffset = 0;
    S3BucketContextProxy bucketContext(m_s3Params);
    size_t dataLen = WriteDataToS3(bucketContext, m_objKey, bufSize);

    m_fileOffset += dataLen;
    HCP_Log(DEBUG, LIBS3) << "FOW: Exited NoCacheBuffer::WriteFS" << HCPENDLOG;

    return dataLen;
}

size_t NoCacheBuffer::Append(EBKFileHandle *handle, char *buf, size_t bufSize)
{
    if (handle == nullptr) {
        HCP_Log(ERR, LIBS3) << "FileHandle is null." << HCPENDLOG;
        return 0;
    }
    Init(buf, bufSize);
    S3BucketContextProxy bucketContext(m_s3Params);
    m_dataLen = bufSize;
    m_dataOffset = 0;

    stringstream stream;
    stream << m_objKey << "_" << m_partNum;
    string partFileName = stream.str();
    if (bufSize != WriteDataToS3(bucketContext, partFileName, bufSize)) {
        HCP_Log(ERR, LIBS3) << "Write data to s3 failed.." << HCPENDLOG;
        return 0;
    }

    m_fileOffset += bufSize;
    m_partNum++;

    return bufSize;
}

void NoCacheBuffer::Truncate(long int pos)
{
}

void NoCacheBuffer::Reset()
{
    m_dataLen = 0;
    m_dataOffset = 0;
    m_fileOffset = 0;
}

bool NoCacheBuffer::Finish(EBKFileHandle *handle, FileCloseStatus status)
{
    if (handle == nullptr) {
        HCP_Log(ERR, LIBS3) << "FileHandle is null." << HCPENDLOG;
        return false;
    }

    if (m_fileCloseStatus) {
        HCP_Log(DEBUG, LIBS3) << "File already closed." << HCPENDLOG;
        return true;
    }

    if (status == FILE_CLOSE_STATUS_PAUSE) {
        HCP_Log(DEBUG, LIBS3) << "Pause multipart upload." << HCPENDLOG;
        m_fileCloseStatus = true;
        return true;
    }

    // list parts
    string filePrefix = m_objKey;
    filePrefix += "_";
    if (!ListBucket(m_s3Params, filePrefix.c_str())) {
        HCP_Log(ERR, LIBS3) << "List objects failed." << HCPENDLOG;
        return false;
    }

    S3BucketContextProxy bucketContext(m_s3Params);
    if (FileExists(bucketContext, m_objKey)) {
        HCP_Log(INFO, LIBS3) << "snap file exist, file name is: " << m_objKey << HCPENDLOG;
        if (m_objectInfoList.size() == 0) {
            HCP_Log(DEBUG, LIBS3) << "List objects has been delete!." << HCPENDLOG;
            m_fileCloseStatus = true;
            return true;
        } else {
            m_fileCloseStatus = true;
            return DeletePartDataFromS3();
        }
    }

    HCP_Log(DEBUG, LIBS3) << "snap file not exist, file name is : " << m_objKey << HCPENDLOG;

    if (m_objectInfoList.size() == 0) {
        HCP_Log(DEBUG, LIBS3) << "List objects failed." << HCPENDLOG;
        return false;
    }

    sort(m_objectInfoList.begin(), m_objectInfoList.end(), Comp);

    m_partNum = 0;
    m_dataOffset = 0;
    m_fileOffset = 0;
    m_dataLen = 0;
    size_t dataLen = WritePartDataToS3(m_ObjectsTotalSize);
    if (dataLen != m_ObjectsTotalSize) {
        HCP_Log(ERR, LIBS3) << "Write part data to S3 failed." << HCPENDLOG;
        return false;
    }
    m_fileCloseStatus = true;
    return DeletePartDataFromS3();
}
