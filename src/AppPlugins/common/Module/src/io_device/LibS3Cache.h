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
#ifndef LIBS3_CACHE_H
#define LIBS3_CACHE_H

#include <string>
#include <stdlib.h>
#include <securec.h>
#include <boost/scoped_array.hpp>
#include "S3Base.h"
#include "FileDef.h"
#include "IOCommonDef.h"
// #include "protocol/BasicTypes.pb.h"

#define LIBS3 "libs3IO"

namespace Module {
static const size_t g_dataBlockSize = 4 * 1024 * 1024;  // 4M

typedef struct external_buf
{
    char *buffer;
    uint64_t offset;
    uint64_t buffer_size;
} external_buf;

class ICache {
public:
    virtual bool CreateHandle(EBKFileHandle *handle)
    {
        return true;
    }
    virtual ~ICache(){}
    virtual void Init(char *buf, const size_t bufSize){}
    virtual std::size_t Read(char *buf, std::size_t bufSize) = 0;
    virtual std::size_t ReadFS(char *buf, std::size_t bufSize) = 0;
    virtual std::size_t Write(char *buf, std::size_t bufSize) = 0;
    virtual std::size_t WriteFS(char* buf, std::size_t bufSize) = 0;
    virtual std::size_t Append(EBKFileHandle *handle, char *buf, std::size_t bufSize) = 0;
    virtual std::size_t ReadSegement(char *buf, std::size_t bufSize) = 0;
    virtual std::size_t WriteSegement(char *buf, std::size_t bufSize) = 0;
    virtual void Truncate(long int position) = 0;
    virtual void Reset() = 0;
    virtual std::size_t Size() const = 0;
    virtual void SetSize(std::size_t size) = 0;
    virtual std::size_t Capacity() const = 0;
    virtual std::size_t Offset() const = 0;
    virtual void SetOffset(long int offset) = 0;
    virtual void SetObjectKey(const std::string &objKey){}
    virtual void SaveOffset() = 0;
    virtual void RecoverOffset() = 0;
    virtual char* GetData() const = 0;
    virtual void SetUpLoadRateLimit(uint64_t qos){}
    virtual void SetDownLoadRateLimit(uint64_t qos){}

    virtual bool DumpToLocalFile(std::string localPath)
    {
        return false;
    };

    // virtual void SetObjectKey(const std::string& objKey){}
    virtual void SetDEK(const std::string &dek){}

    virtual bool Finish(EBKFileHandle *handle, FileCloseStatus status)
    {
        return true;
    }
};

class Cache : public ICache {
public:
    Cache() : m_data(nullptr), m_dataBufferSize(0), m_dataSize(0), m_offset(0), m_oldOffset(0)
    {
        HCP_Log(DEBUG, LIBS3) << "Enter construct..." << HCPENDLOG;
    }

    virtual ~Cache()
    {
        HCP_Log(DEBUG, LIBS3) << "Enter disconstruct..." << HCPENDLOG;
        if (m_data != nullptr) {
            delete[] m_data;
            m_data = nullptr;
        }
    }

    virtual std::size_t Read(char *buf, std::size_t bufSize);
    virtual std::size_t ReadFS(char *buf, std::size_t bufSize);
    virtual std::size_t Write(char *buf, std::size_t bufSize);
    virtual std::size_t WriteFS(char* buf, std::size_t bufSize);
    virtual std::size_t Append(EBKFileHandle *handle, char *buf, std::size_t bufSize);
    virtual std::size_t ReadSegement(char *buf, std::size_t bufSize)
    {
        return Read(buf, bufSize);
    }
    virtual std::size_t WriteSegement(char *buf, std::size_t bufSize)
    {
        return Append(nullptr, buf, bufSize);
    }
    virtual void Truncate(long int position);
    virtual void Reset();
    virtual std::size_t Size() const
    {
        return m_dataSize;
    }  // bytes
    virtual char* GetData() const
    {
        return m_data;
    }
    virtual void SetSize(std::size_t size){}
    virtual std::size_t Capacity() const
    {
        return m_dataBufferSize;
    }
    virtual std::size_t Offset() const
    {
        return m_offset;
    }
    virtual void SetOffset(long int offset)
    {
        m_offset = offset;
    }
    virtual void SaveOffset()
    {
        m_oldOffset = m_offset;
    }
    virtual void RecoverOffset()
    {
        m_offset = m_oldOffset;
    }

    bool DumpToLocalFile(std::string fileName) override;

private:
    Cache(const Cache &);
    const Cache &operator=(const Cache &);

    char *m_data;
    std::size_t m_dataBufferSize;
    std::size_t m_dataSize;
    std::size_t m_offset;
    std::size_t m_oldOffset;
};

class NoCacheBuffer : public ICache, protected S3Base {
public:
    NoCacheBuffer(S3IOParams &params) :
        m_buffer(nullptr),
        m_bufferLen(0),
        m_dataLen(0),
        m_dataOffset(0),
        m_fileOffset(0),
        m_partNum(0),
        m_oldDataLen(0),
        m_oldDataOffset(0),
        m_oldFileOffset(0),
        m_oldPartNum(0),
        m_s3Params(params),
        m_fileCloseStatus(false){}
    virtual ~NoCacheBuffer(){}
    void Init(char *buffer, const size_t bufferLen);
    virtual bool CreateHandle(EBKFileHandle *handle);
    std::size_t Read(char *buf, std::size_t bufSize);
    std::size_t ReadFS(char *buf, std::size_t bufSize);
    std::size_t Write(char *buf, std::size_t bufSize);
    std::size_t WriteFS(char* buf, std::size_t bufSize);
    std::size_t Append(EBKFileHandle *handle, char *buf, std::size_t bufSize);
    virtual bool Finish(EBKFileHandle *handle, FileCloseStatus status);
    virtual std::size_t ReadSegement(char *buf, std::size_t bufSize);
    virtual std::size_t WriteSegement(char *buf, std::size_t bufSize);
    virtual size_t PartCallbackRead(char *buf, size_t bufSize);

    void Truncate(long int position);
    // add w90006072 fix  DTS2019022612416
    bool UseVpp()
    {
        return m_useVpp;
    }
    // void	Destory(){}
    void Reset();
    std::size_t Size() const
    {
        return m_dataLen;
    }  // bytes
    void SetSize(std::size_t size)
    {
        m_dataLen = size;
    }
    std::size_t Capacity() const
    {
        return m_bufferLen;
    }
    std::size_t Offset() const
    {
        return m_fileOffset;
    }
    void SetOffset(long int offset)
    {
        m_fileOffset = offset;
    }
    void SetDEK(const std::string &dek)
    {
        m_dek = dek;
    }
    void SetObjectKey(const std::string &objKey)
    {
        S3Base::SetObjectKey(objKey);
    }
    virtual void SaveOffset();
    virtual void RecoverOffset();

    virtual void RWBegin()
    {
        SaveOffset();
    }
    virtual void RWRetry(bool resetFlag)
    {
        RecoverOffset();
    }
    virtual bool BeginUpload()
    {
        RWBegin();
        return true;
    }
    virtual bool RetryUpload(uint64_t segmentNum, uint64_t segmentSize)
    {
        RWRetry(false);
        return true;
    }
    virtual char* GetData() const
    {
        return m_buffer;
    }

    virtual void SetUpLoadRateLimit(uint64_t qos)
    {
        m_s3Params.uploadRateLimit = qos;
    }
    virtual void SetDownLoadRateLimit(uint64_t qos)
    {
        m_s3Params.downloadRateLimit = qos;
    }

protected:
    virtual size_t GetUploadFileData(char *buffer, int dataLen);
    virtual ssize_t CallbackWrite(const char *buf, size_t bufSize);
    virtual size_t CallbackRead(char *buf, size_t bufSize);
    virtual size_t WritePartDataToS3(size_t dataLen);
    virtual bool DeletePartDataFromS3();

private:
    char *m_buffer;
    std::shared_ptr<char> partFileContent;
    size_t m_bufferLen;
    size_t m_dataLen;
    size_t m_dataOffset;
    size_t m_fileOffset;
    uint64_t m_partNum;

    size_t m_oldDataLen;
    size_t m_oldDataOffset;
    size_t m_oldFileOffset;
    uint64_t m_oldPartNum;

    S3IOParams &m_s3Params;
    bool m_fileCloseStatus;
};
} // namespace Module
#endif
