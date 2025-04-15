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
#ifndef S3_OBJ_H
#define S3_OBJ_H

#include <string>
#include <iostream>
#include <vector>

#include <boost/thread/mutex.hpp>
#include <boost/scoped_array.hpp>
#include <boost/logic/tribool.hpp>

#include "log/Log.h"
#include "LibS3Cache.h"

#include "stdio.h"
#include "IODeviceInterface.h"

namespace Module {
const std::string CERTIFICATE_PATH = "/opt/huawei-data-protection/ebackup/conf/uds.crt";
const std::string CERTIFICATE_TMP_PATH = "/opt/huawei-data-protection/ebackup/conf/uds_tmp.crt";

class EnableVpp {
public:
    static EnableVpp &GetInstance();
    void SetEnableVppSpeedUp(bool enableVppSpeedUp);
    bool GetEnableVppSpeedUp();

private:
    EnableVpp();
    virtual ~EnableVpp() {};
    EnableVpp(const EnableVpp &);
    const EnableVpp &operator=(const EnableVpp &);

private:
    bool m_enableVppSpeedUp;
};

class libs3IO : public S3Base {
public:
    libs3IO(const S3IOParams &params, int objType = OBJECT_CACHE_DATA);
    virtual ~libs3IO();

    std::string GetErrorInfo();
    bool GetNotExistError();

    bool Open(const char *fileName, const char *mode, EBKFileHandle *handle = nullptr);
    bool Close(FileCloseStatus status = FILE_CLOSE_STATUS_FINISH);
    void SetDek(const std::string &dek = "");
    void ConvertHex2ASCII(const std::string &in, std::string &out);
    bool FilePrefixExists(const char *filePrefixName, std::vector<std::string> &suffixs);
    void FillHttpProxyParam(S3BucketContextProxy& bucketCtx, std::string& host, std::string& user);

    std::size_t Read(char *buf, std::size_t bufSize);
    std::size_t ReadFS(char *buf, std::size_t bufSize);
    std::size_t Write(char *buf, std::size_t bufSize);
    std::size_t WriteFS(char* buf, std::size_t bufSize);
    std::size_t Append(char *buf, std::size_t bufSize);

    bool UseVpp();

    long int Tell();

    bool Loaded();

    int Truncate(long int position);
    int Seek(long int offset, int origin = SEEK_SET);

    bool CreateDirectory(const char *directoryName);
    bool TestConnect(const char *bucketName, const int retryTimes = 3);
    /* BEGIN: add by cfy 2017-10-10. PN:for multi-buckets */
    bool TestSubBucketIsNotExisted(const char *bucketName, const int retryTimes = 3);
    /* END: add by cfy 2017-10-10. PN:for multi-buckets */

    obs_status TestBucket(S3BucketContextProxy &bucketCtx, const int retryTimes = 3);
    int Flush(bool sync = true);

    virtual boost::tribool FileExists(const char *fileName);
    virtual boost::tribool FileExists(S3BucketContextProxy &bucketContext, const std::string& fileName);

    bool Exists(const char *fileName);
    bool Exists(S3BucketContextProxy &bucketContext, const std::string &fileName);
    bool IsDirectory(const char *pathName);
    bool Rename(S3BucketContextProxy &bucketContext, const std::string &oldName, const std::string &newName);
    bool Rename(const char *oldName, const char *newName);
#ifndef EBKSDK_LIBS3_COMPILE
    boost::tribool FileExists(const std::string& fileName, uint64_t *contentLength, int64_t *lastModified, std::string &etag);
    bool Copy(S3BucketContextProxy& bucketContext,const std::string& oldName, const std::string& newName,
            int metaDataCount = 0, obs_name_value *metaData = 0);
    bool Copy(const char *oldName, const char *newName);
    bool GetBucketObjects(S3BucketContextProxy& bucketCtx, std::vector <std::string>& elementList);
    bool GetMetadata(S3BucketContextProxy &bucketCtx, const std::string &objName, std::vector<obj_metadata> &obj_metadata);
    bool NormalFileUpload(const std::string &local_objName, const std::string &remote_objName);
    bool FileDownload(const std::string &objName, const std::string &destFile);
    bool FileDownload(S3BucketContextProxy &bucketCtx, const std::string &objName, const std::string &destPath,
      uint64_t startOffset = 0, uint64_t rangeInBytes = 0);
    bool UploadObject(S3BucketContextProxy &bucketCtx, const std::string &objName, const char *buffer, size_t bufSize,
      int metaDataCount = 0, obs_name_value *metaData = 0);
    bool ReadPartialObject(S3BucketContextProxy &bucketCtx, const std::string &objName,
      uint64_t startOffset, uint64_t rangeInBytes, char *buffer, uint64_t buffer_size);
    bool CompleteMultiPartUpload(S3BucketContextProxy& bucketCtx,
      const std::string& remote_objName, const std::string& uploadTaskID);
    bool CompleteMultipartUploadInternalList(S3BucketContextProxy& bucketCtx, const std::string& objName, const std::string& uploadTaskID);
    bool UploadOnePart(S3BucketContextProxy& bucketCtx, const std::string& objName,
      const std::string& uploadTaskID, char *buffer, uint64_t buffer_size, int part_index);
    bool InitiateMultiPartUpload(S3BucketContextProxy& bucketCtx,const std::string& objName,std::string& uploadTaskID);
    bool CopyOnePart(S3BucketContextProxy& bucketCtx, const std::string& srcObjName,
                    const std::string& destBucket, const std::string& destObjName, uint64_t startByte,uint64_t byteCount,
                    const std::string& uploadTaskID, int partIndex);
    
    obs_status CallbackListMultipartUploads(int isTruncated, const char *nextMarker,const char*nextUploadIdMarker, 
                                   int uploadsCount, const obs_list_multipart_upload *uploads, int commonPrefixesCount, const char **commonPrefixes);
    bool ListMultiPartUploads(S3BucketContextProxy& bucketCtx, const std::string& prefix);
    bool GetuploadTaskMapByPath(S3BucketContextProxy& bucketCtx, const std::string& prefix, std::map<int, singleUploadTask>& uploadTaskMap);
#else
    bool Copy(S3BucketContextProxy& bucketContext, const std::string& oldName, const std::string& newName);
    bool FileDownload(S3BucketContextProxy& bucketCtx, const std::string& objName, const std::string& destFile);
#endif

    bool Remove(const char *fileName);
    bool RemoveBatchFile(S3BucketContextProxy &bucketContext, const std::string &fileName);
    bool RemoveAll(const char *pathName);
    int64_t GetAllSize(const char * pathName);
    void Reset(const char *fileName, const char *mode);
    bool FileSize(const char *fileName, uintmax_t &size);
    bool FileSize(S3BucketContextProxy &bucketContext, const std::string &fileName, uintmax_t &size);
    long int DirSize(const char *fileName);
    long int DirSize(S3BucketContextProxy &bucketContext, const std::string &fileName);

    bool GetDirectoryList(const char *directoryName, std::vector<std::string> &elementList);
    bool GetDirectoryList(S3BucketContextProxy &bucketContext, const std::string &directoryName,
                          std::vector<std::string> &elementList);
    bool GetFileListInDirectory(const char *directoryName, std::vector<std::string> &elementList);
    bool GetFileListInDirectory(S3BucketContextProxy &bucketContext, const std::string &directoryName,
                                std::vector<std::string> &elementList);
    bool GetObjectListWithMarker(const std::string &dir, std::string &marker, bool &isEnd, int maxKeys,
        std::vector<std::string> &elementList);

    /* BEGIN: add by w90006072. PN: for US2018091300339 */
    bool GetCommonPrefixList(const char *directoryName,
                             const std::string &delimiter,
                             std::vector<std::string> &elementList);
    bool GetCommonPrefixList(S3BucketContextProxy &bucketContext,
                             const std::string &directoryName,
                             const std::string &delimiter,
                             std::vector<std::string> &elementList);

    bool GetObjectContent(const std::string &filename, obs_list_objects_content &objContent);
    bool GetObjectContent(S3BucketContextProxy &bucketContext,
                          const std::string &filename,
                          obs_list_objects_content &objContent);
    /* END: add by w90006072 # US2018091300339. */

    void JudgeStatus(obs_status status, uint64_t& capacity, bool& ret);
    bool GetCapacity(uint64_t &capacity);
    /* BEGIN: add by cfy 00377603. PN: for multi-buckets */
    bool GetBucketObjNum(const char *bucketName, int64_t &objectNum);
    /* END: add by cfy 00377603. */
    bool GetUsed(uint64_t &capacity);
    virtual ssize_t CallbackWrite(const char *buf, std::size_t bufSize);
    virtual std::size_t CallbackRead(char *buf, std::size_t bufSize);

    bool UploadFile(const std::string &objName, const std::string &destFile);
    bool NormalFileUpload(S3BucketContextProxy &bucketCtx, const std::string &local_objName,
                          const std::string &remote_objName);
    bool DownloadFile(const std::string &objName, const std::string &destFile);
    void SetCacheDataType(int cacheDataType)
    {
        m_objType = cacheDataType;
    }
    int GetCacheDataType()
    {
        return m_objType;
    }

    /* add by axf begin */
    bool SetUploadPartInfo(const std::string &objName, uint64_t segmentSize);
    int uploadData(int bufferSize, char *buffer);
    int CopyFromExternalBuf(char *buffer, int bufferSize);
    ssize_t CopyToExternalBuf(const char* buf, size_t bufSize);
    obs_status listParts(int isTruncated, int partsCount, const obs_list_parts *parts);

    bool GetBucketObjNumber(const S3IOParams &params, char *objectNum);
    /* add by cfy 00377603 end */

    bool ReadNoCache(const std::string &filename, const std::string &dek, size_t offset, char *buffer,
                     const size_t bufferLen, size_t &readedLen);

    bool RemoveObjs(const std::vector<std::string> &directoryNames, std::vector<std::string> &objs, uintmax_t &size,
                    const uint8_t &snapVersion);
    bool MultiBlocksDelete4Multiple(const std::vector<std::string> &directoryNames, std::vector<std::string> &objs,
                                    uintmax_t &size, const uint8_t &snapVersion);
    bool MultiBlocksDelete4Single(const std::vector<std::string> &directoryNames, std::vector<std::string> &objs,
                                  uintmax_t &size, const uint8_t &snapVersion);
    bool CalcSize(S3BucketContextProxy &bucketContext, std::string &s3ObjPath, uintmax_t &size);

    bool TestSubBucketExisted(const char *bucketName);
    bool TestBucketExisted(const char *bucketName);
    bool ListObjectName(S3IOParams params, std::string prefix, int maxKeys, std::vector<std::string> &objectNames);
    void RegisterCallbackHandle(CallBackHandle &handle);
    void RegisterReadFileCallbackFun(const ReadFileCallback &handle);
    void SetUpLoadRateLimit(uint64_t qos) {
        m_S3IOInfo.uploadRateLimit = qos;
    }

    void SetDownLoadRateLimit(uint64_t qos) {
        m_S3IOInfo.downloadRateLimit = qos;
    }

private:
    bool RemoveFilesMatched(S3BucketContextProxy &bucketContext);
    bool OpenSub(EBKFileHandle *handle);
    const char *GetObjectKey(const std::string &fileName);
    void SendData();  // put a object
    void SendDataInner(S3BucketContextProxy &bucketContext, size_t &sendSize);
    void RecvData();                                                         // get a object
    void RecvDataNoCache(const size_t startBytes, const size_t countBytes);  // get a object
    bool CheckLocalChainDBIntegrity();

    virtual void RWBegin()
    {
        m_cache->SaveOffset();
    }
    virtual void RWRetry(bool resetFlag)
    {
        m_cache->RecoverOffset();
        if (resetFlag == true){
            m_cache->Reset();
        }
    }
    virtual bool RetryUpload(uint64_t segmentNum, uint64_t segmentSize);
    virtual size_t GetUploadFileData(char *buffer, int dataLen);

    bool IsChaindbFile();

private:
    bool m_dataLoaded;

    std::string m_mode;
    
    EBKFileHandle *m_fileHandle;
    std::string m_localFileName;

    S3IOParams m_S3IOInfo;
    libs3IO(const libs3IO &c);
    external_buf m_external_buf;
};
} // namespace Module
#endif
