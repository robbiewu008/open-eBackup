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
#ifndef FS_SCANNER_SMB_META_PRODUCER_H
#define FS_SCANNER_SMB_META_PRODUCER_H

#include <dirent.h>
#include "MetaProducer.h"
#include "DirectoryCacheMap.h"
#include "SmbContextWrapper.h"

class SmbMetaProducer : public MetaProducer {
public:
    explicit SmbMetaProducer(std::shared_ptr<ScanQueue> scanQueue,
        std::shared_ptr<BufferQueue<DirectoryScan>> output,
        std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<ScanFilter> scanFilter,
        std::shared_ptr<FSScannerCheckPoint> chkPntMgr,
        Module::SmbContextArgs args,
        ScanConfig &config)
        : MetaProducer(scanQueue, output, statsMgr, scanFilter, chkPntMgr),
          m_args(args),
          m_config(config)
    {};
    SmbMetaProducer() = delete;
    ~SmbMetaProducer() override {};

    static void AccessDirCallback(struct smb2_context *smb, int status, void *data, void *privateData);
    static void QueryDirCallback(struct smb2_context *smb, int status, void *data, void *privateData);
    static void CloseDirCallbck(struct smb2_context *smb, int status, void *data, void *privateData);
    static void StatDirCallback(struct smb2_context *smb, int status, void *data, void *privateData);
    static void StatFileCallback(struct smb2_context *smb, int status, void *data, void *privateData);

    bool ResetSmbClient();
    void Produce(int count) override;
    SCANNER_STATUS InitContext() override;
    void AccessDirResponse(int status, struct smb2_query_directory_data *queryData, DirectoryCacheMap *cacheMap);
    void QueryDirResponse(int status, struct smb2_query_directory_data *queryData, DirectoryCacheMap *cacheMap);
    void CloseDirResponse(int status, DirectoryCacheMap *cacheMap);
    SCANNER_STATUS DestroyContext() override;
    void StatDirResponse(int status, struct SMB2_ALL_INFO *allInfo, DirectoryCacheMap *cacheMap);
    void StatFileResponse(int status, struct SMB2_ALL_INFO *allInfo, DirectoryCacheMap *cacheMap);
    void GetDataRangeResponse(int status, struct SMB2_DATA_RANGE *dataRange, DirectoryCacheMap *cacheMap);

private:
    std::shared_ptr<Module::SmbContextWrapper> m_client = nullptr;
    std::queue<DirectoryCacheMap*> m_retryQueue;
    Module::SmbContextArgs m_args {};
    ScanConfig &m_config;
    bool m_resetSmb = false;
    int m_waitPendingTimes = 0;

    bool CheckPendingRequestReachedLimit();
    bool CheckScanStatus();
    bool ProcessPendingRequest();
    bool ProcessRetry();
    void ProcessScanQueue(int count);
    bool RequestAccessDir(const std::string& path, DirectoryCacheMap *cacheMap);
    bool RequestQueryDir(struct smb2_query_directory_data *queryData, DirectoryCacheMap *cacheMap);
    bool RequestCloseDir(struct smb2_query_directory_data *queryData, DirectoryCacheMap *cacheMap);
    bool RequestStatDir(const std::string& path, DirectoryCacheMap *cacheMap);
    bool RequestStatFile(const std::string& path, DirectoryCacheMap *cacheMap);
    std::pair<uint16_t, uint64_t> ReadEntry(struct smb2dir *smbDir,
        DirectoryCacheMap *cacheMap, uint8_t baseFilterFlag, std::string basepath);
    void ReadSmbDir(struct smb2_query_directory_data *queryData, DirectoryCacheMap *cacheMap);
    void CacheDirectory(struct SMB2_ALL_INFO *allInfo, DirectoryCacheMap *cacheMap);
    void CacheFile(struct smb2dirent *smbdirent, DirectoryCacheMap *cacheMap);
    void CacheFile(struct SMB2_ALL_INFO *allInfo, DirectoryCacheMap *cacheMap);
    void ReturnDirToScanQueue(DirectoryCacheMap *cacheMap);
    void PushCacheMapToWriteQueue(DirectoryCacheMap *cacheMap, bool isComplete);
    bool SkipDirEntry(std::string name, std::string fullPath) const;
    void CopySmbAllInfoToDirMeta(const struct SMB2_ALL_INFO *allInfo, Module::DirMeta &dmeta);
    void CopySmbAllInfoToFileMeta(const struct SMB2_ALL_INFO *allInfo, Module::FileMeta &fmeta);
    void CopyAcl(const struct SMB2_ALL_INFO *allInfo, std::string &aclText);
    void ClearCacheMap(const std::string &fullPath, DirectoryCacheMap *cacheMap);
    void FreeSmbAllInfo(struct SMB2_ALL_INFO *allInfo) const;
    void WrapDirectory(Module::DirMetaWrapper &dirWrapper, DirectoryCacheMap *cacheMap);
    void WrapFile(Module::FileMetaWrapper &fileWrapper, DirectoryCacheMap *cacheMap);
    void SendDirToWriteQueue(const DirStat& dirStat);
    void FillDirStat(DirStat &dirStat);
    void HandleQueryDirError(int status, const std::string& path,
        struct smb2_query_directory_data *queryData, DirectoryCacheMap *cacheMap);
    bool SmbDealSkip(struct smb2dirent *smbdirent);
    void GetFileDataRange(std::string name, uint64_t fileSize, std::string &sparseStr);
};
#endif
