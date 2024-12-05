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
#ifndef GUARDED_FILE_H
#define GUARDED_FILE_H

#include <limits>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <stdlib.h>

#include "system/System.hpp"
#include "IODeviceInterface.h"
#ifdef __WINDOWS__
#include <io.h>
#define fsync     _commit
#define fileno    _fileno
#define ftruncate _chsize
#endif

class IODevice;

namespace Module {
static const int TEST_CONNECT_RETRY_TIMES_DEFAULT = 3;

class GuardedFile {
public:
    GuardedFile(OBJECT_TYPE fileType = OBJECT_CACHE_DATA);

    virtual ~GuardedFile();

    std::string GetErrorInfo();

    bool GetNotExistError();

    bool Open(const char *filename, const char *mode, EBKFileHandle *handle = nullptr);

    bool Close(FileCloseStatus status = FILE_CLOSE_STATUS_FINISH);

    void Reset(const char *fileName, const char *mode = "r");

    size_t Write(const void *ptr, size_t size, size_t count);

    std::size_t WriteFS(const void* ptr, std::size_t size, std::size_t count);
    std::size_t ReadFS( void* ptr, size_t size, size_t count);

    size_t Append(const void *ptr, size_t count);

    size_t Read(void *ptr, size_t size, size_t count);

    int Seek(long int offset, int origin = SEEK_SET);

    long int Tell();

    bool FileSize(uintmax_t &size, const char *filename = "");

    long int DirSize(const char *filename = "");

    int Truncate(long int position);

    int Flush(bool sync = true);

    bool Loaded();

    bool UseVpp();

    boost::tribool FileExists(const char *filename);

    bool FilePrefixExists(const char *filePrefixName, std::vector<std::string> &suffixs);

    bool Exists(const char *filename);

    bool Remove(const char *filename);

    bool RemoveAll(const char *directoryName);

    bool Rename(const char *oldName, const char *newName);

    bool CreateDirectory(const char *directoryName);

    bool CreateCifsDirectory(const char *directoryName);

    bool IsDirectory(const char *directoryName);

    bool Copy(const char *srcName, const char *destName);

    bool CopyDirectory(const char *srcName, const char *destName);

    bool GetDirectoryList(const char *directoryName, std::vector<std::string> &elementList);

    bool GetFileListInDirectory(const char *directoryName, std::vector<std::string> &elementList);

    bool GetObjectListWithMarker(const std::string &dir, std::string &marker, bool &isEnd, int maxKeys,
        std::vector<std::string> &elementList);

    virtual bool GetCommonPrefixList(const char *directoryName, const std::string &delimiter,
                                     std::vector<std::string> &elementList);

    virtual bool TestConnect(const char *bucketName, const int retryTimes);

    virtual bool TestSubBucketIsNotExisted(const char *bucketName, const int retryTimes);

    bool GetSpaceInfo(const char *pathName, uint64_t &capacity, uint64_t &free);

    virtual bool GetBucketObjNum(const char *bucketName, int64_t &objectNum);

    void SetDek(const std::string &dek = "")
    {
        m_dek = dek;
    }

    bool ReadNoCache(const char *filename, const std::string &dek, size_t offset, char *buffer, const size_t bufferLen,
                     size_t &readedLen);

    bool TestSubBucketExisted(const char *bucketName);

    bool TestBucketExisted(const char *bucketName);

    bool RemoveObjs(const std::vector<std::string> &directoryNames, std::vector<std::string> &objs, uintmax_t &size,
                    const uint8_t &snapVersion);

    bool DownloadFile(const std::string &s3Path, const std::string &objName, const std::string &destFile);

    bool NormalFileUpload(const std::string &s3Path, const std::string &localFile, const std::string &remoteFile);

    void RegisterCallbackHandle(const std::string &s3Path, CallBackHandle &handle);

    void RegisterReadFileCallbackFun(const std::string &s3Path, const ReadFileCallback &handle);

    void SetUpLoadRateLimit(uint64_t qos);

    void SetDownLoadRateLimit(uint64_t qos);

protected:
    void CreateInstance(const std::string &fileName);

private:
    GuardedFile(const GuardedFile &);
    const GuardedFile &operator=(const GuardedFile &);

    std::shared_ptr<IODevice> m_IODevice;
    std::string m_dek;
    OBJECT_TYPE m_FileType;
    uint64_t m_UpLoadRateLimit {0};
    uint64_t m_DownLoadRateLimit {0};
};
}  // namespace Module

#endif  // GUARDED_FILE_H
