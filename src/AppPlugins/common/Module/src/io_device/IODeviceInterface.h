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
#ifndef IO_DEVICE_INTERFACE_H
#define IO_DEVICE_INTERFACE_H

#include <boost/logic/tribool.hpp>
#include "FileDef.h"
#include "BackupIOInterface.h"

namespace Module {
class IODevice {
public:
    virtual ~IODevice(){}

    virtual std::string GetErrorInfo() = 0;
    virtual bool GetNotExistError() = 0;

    virtual bool Open(const char *filename, const char *mode, EBKFileHandle *handle) = 0;
    virtual bool Close(FileCloseStatus status) = 0;

    virtual void SetDek(const std::string &dek = "") = 0;
    virtual size_t Write(const void *ptr, size_t size, size_t count) = 0;
    virtual size_t WriteFS(const void* ptr, size_t size, size_t count) = 0;
    virtual size_t Append(const void *ptr, size_t count) = 0;
    virtual size_t Read(void *ptr, size_t size, size_t count) = 0;
    virtual size_t ReadFS(void *ptr, size_t size, size_t count) = 0;

    virtual bool UseVpp() = 0;

    virtual int Seek(long int offset, int origin = SEEK_SET) = 0;
    virtual long int Tell() = 0;
    virtual bool FileSize(const char *filename, uintmax_t &size) = 0;
    virtual int Truncate(long int position) = 0;
    virtual int Flush(bool sync = true) = 0;

    virtual bool Loaded() = 0;
    virtual bool Exists(const char *filename) = 0;
    virtual bool Remove(const char *filename) = 0;
    virtual bool RemoveAll(const char *directiryName) = 0;
    virtual bool Rename(const char *oldName, const char *newName) = 0;
    virtual bool CreateDirectory(const char *directoryName) = 0;
    virtual bool CreateCifsDirectory(const char *directoryName) = 0;
    virtual bool IsDirectory(const char *directoryName) = 0;
    virtual void Reset(const char *fileName, const char *mode) = 0;

    virtual bool Copy(const char *srcName, const char *destName) = 0;

    virtual bool GetDirectoryList(const char *directoryName, std::vector<std::string> &elementList) = 0;
    virtual bool GetFileListInDirectory(const char *directoryName, std::vector<std::string> &elementList) = 0;
    virtual bool GetObjectListWithMarker(const std::string &dir, std::string &marker, bool &isEnd, int maxKeys,
        std::vector<std::string> &elementList) = 0;
    virtual bool GetCommonPrefixList(const char *directoryName, const std::string &delimiter,
                                     std::vector<std::string> &elementList) = 0;

    virtual bool TestConnect(const char *bucketName, const int retryTimes) = 0;
    virtual bool TestSubBucketIsNotExisted(const char *bucketName, const int retryTimes) = 0;
    virtual bool GetBucketObjNum(const char *bucketName, int64_t &objectNum) = 0;
    virtual bool GetSpaceInfo(const char *pathName, uint64_t &capacity, uint64_t &free) = 0;
    virtual bool CopyDirectory(const char *srcName, const char *destName) = 0;

    virtual long int DirSize(const char *filename) = 0;
    virtual boost::tribool FileExists(const char *filename) = 0;
    virtual bool FilePrefixExists(const char *filePrefixName, std::vector<std::string> &suffixs) = 0;

    virtual bool ReadNoCache(const char *filename, const std::string &dek, size_t offset, char *buffer,
                             const size_t bufferLen, size_t &readedLen) = 0;

    virtual bool RemoveObjs(const std::vector<std::string> &directoryNames, std::vector<std::string> &objs,
                            uintmax_t &size, const uint8_t &snapVersion) = 0;

    virtual bool DownloadFile(const std::string &objName, const std::string &destFile) = 0;

    virtual bool NormalFileUpload(const std::string &localFile, const std::string &remoteFile) = 0;

    virtual void SetCacheDataType(int cacheDataType) = 0;

    virtual int GetCacheDataType() = 0;

    virtual bool TestSubBucketExisted(const char *bucketName) = 0;

    virtual bool TestBucketExisted(const char *bucketName) = 0;

    virtual void RegisterCallbackHandle(CallBackHandle &handle) = 0;

    virtual void RegisterReadFileCallbackFun(const ReadFileCallback &handle) = 0;

    virtual void SetUpLoadRateLimit(uint64_t qos) = 0;

    virtual void SetDownLoadRateLimit(uint64_t qos) = 0;
};
} // namespace Module
#endif  // IO_DEVICE_INTERFACE_H
