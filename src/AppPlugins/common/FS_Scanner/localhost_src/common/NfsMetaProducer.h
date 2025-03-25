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
#ifndef FS_SCANNER_NFS_META_PRODUCER_H
#define FS_SCANNER_NFS_META_PRODUCER_H

#include <thread>
#include <dirent.h>
#include "NfsContextWrapper.h"
#include "NFSSyncCbData.h"
#include "ScanConfig.h"
#include "ScanInfo.h"
#include "OpendirResData.h"
#include "BufferQueue.h"
#include "ScanStructs.h"
#include "StatisticsMgr.h"
#include "ScanConsts.h"
#include "MetaProducer.h"

struct ScanDateDetail {
    int32_t aTime = 0;
    int32_t cTime = 0;
    int32_t mTime = 0;
};

class NfsMetaProducer : public MetaProducer {
public:
    uint32_t m_totFailedDirs {0};
    explicit NfsMetaProducer(std::shared_ptr<ScanQueue> scanQueue,
        std::shared_ptr<BufferQueue<DirectoryScan>> output,
        ScanConfig &config,
        std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<ScanFilter> scanFilter,
        std::shared_ptr<FSScannerCheckPoint> chkPntMgr)
        : MetaProducer(scanQueue, output, statsMgr, scanFilter, chkPntMgr),
        m_config(config)
    {
    };
    ~NfsMetaProducer() override {};

    void Produce(int count) override;
    SCANNER_STATUS InitContext() override;
    static void ScannerOpendirAsyncCb(int status, struct nfs_context *nfs, void *nfsDir, void *privateData);
    SCANNER_STATUS DestroyContext() override;
    SCANNER_STATUS RetryProtectedServer() override;

private:
    std::shared_ptr<Module::NfsContextWrapper> m_nfsCtx;
    ScanConfig &m_config;

    void ConvertDirstatToDirmeta(Module::DirMeta &dmeta, const DirStat &dirStat);
    bool SetDirStat(DirStat &dirStat);
    NFSSyncCbData* MallocCbData(const DirStat &dirStat, nfs_fh_scan &fh, Module::DirMeta &dmeta);
    bool SendOpendirReqAsync(DirStat &dirstat);
    void ReadDir(OpendirResData resData);
    void GetAclInfo(nfsdirent *&nfsdirent, std::string &aclText);
    void PostScanDir(DirectoryScan &dirScanObj, OpendirResData &resData);
    void HandleFailedDirectory(OpendirResData resData);
    bool IsDirSkip(struct nfsdirent*& nfsdirent, std::string &fullPath);
    bool IsFileModeSkip(const struct nfsdirent* nfsdirent) const;
    SCANNER_STATUS CopyDirmetaToDirstat(DirStat &dstat,
        Module::DirMeta &dmeta, nfs_fh_scan &fh, const std::string basePath);
    void GetAcl(struct nfsdirent* &nfsdirent, std::string &aclText);
    void ScanDir(NfsdirPath nfsdirPath, DirectoryScan &dirScanObj, uint8_t& baseFilterFlag);
    uint64_t GetNfs4InodeForNetapp(const std::string& fullPath);
    void QueueInfoPush(DirectoryScan &dirScanObj, const std::string &dirPath, uint8_t baseFilterFlag);
    void FreeNFSSyncCbData(NFSSyncCbData *cbData);
    SCANNER_STATUS SendOpendirResumeAsync(OpendirResData &opendirObj);
    void AmendMissingParentDirectories(const std::string &dirPath);
    SCANNER_STATUS WrapFile(Module::FileMetaWrapper &fileWrapper, struct nfsdirent *nfsdirent,
        Module::DirMetaWrapper &dirWrapper);
    SCANNER_STATUS ConvertDirentToDirstat(DirStat &dstat, struct nfsdirent*& nfsdirent, std::string &path);
    SCANNER_STATUS CopyFileMeta(Module::FileMeta &fd, struct nfsdirent*& nfsdirent);
    void FillDateDetails(struct ScanDateDetail &dateDetails, struct nfsdirent*& nfsdirent) const;
    inline bool IsRetryMount(int32_t errorCode) const;
    bool CheckAndFillDirStat(DirStat &dirStat);
    void SendDirToWriteQueue(DirStat &dirStat);
    bool IsRetriableError(int errorCode) const;
    void PostScanDir(bool isDirScanCompleted, OpendirResData &resData, int status);
    SCANNER_STATUS ProcessDirectoryEntry(DirectoryScan &dirScanObj, uint8_t& baseFilterFlag,
        struct nfsdirent *nfsdirent, std::string fullPath);
    bool NfsDealSkip(struct nfsdirent*& nfsdirent);
};
#endif