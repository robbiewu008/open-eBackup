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
#ifndef FS_SCANNER_CONTROL_GENERATOR_H
#define FS_SCANNER_CONTROL_GENERATOR_H
#include <memory>
#include <vector>
#include <list>
#include "CommonService.h"
#include "DiffControlService.h"
#include "DiffControlThread.h"
#include "BufferQueue.h"
#include "ConsumerThread.h"
#include "ScanConsts.h"
#include "ScanFilter.h"
#include "FSScannerCheckPoint.h"
#include "DirMtimeService.h"
#include "CtrlFilterManager.h"

#ifdef _NAS
#include "NfsDirMtimeService.h"
#endif


#ifdef NAS_SNAPDIFF
#include "SnapdiffBackupEntry.h"
#endif

class ControlGenerator {
public:
#ifdef NAS_SNAPDIFF
    explicit ControlGenerator(ScanJobType type,
        std::shared_ptr<BufferQueue<SnapdiffResultMap>> m_snapdiffBuffer,
        std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<FSScannerCheckPoint> chkPntMgr,
        std::shared_ptr<ScanFilter> scanFilter);
#endif

    explicit ControlGenerator(ScanJobType type,
        std::shared_ptr<BufferQueue<DirectoryScan>> buffer,
        std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<FSScannerCheckPoint> chkPntMgr,
        std::shared_ptr<ScanFilter> scanFilter);
    ~ControlGenerator();

    SCANNER_STATUS Poll();
    SCANNER_STATUS GenerateMeta(ScanConfig &config, ScanInfo &info);
    SCANNER_STATUS GenerateUniqueDcache(ScanConfig &config, ScanInfo &info);
    SCANNER_STATUS GenerateDiffControl(ScanConfig &config);
    SCANNER_STATUS GenerateDiffControlPart2();
    void WriteNewDelDirectories(std::queue<Module::DirCache> &dcQueue, std::string flag, bool isCur);
#ifdef NAS_SNAPDIFF
    SCANNER_STATUS GenerateSnapdiffMeta(ScanConfig &config);
#endif
    SCANNER_STATUS FlushMeta();
    SCANNER_STATUS Clean(const std::string &ctrlPath, ScanConfig &config);
    SCANNER_STATUS CheckPointFlush();
    void PreconditionSatisfied();
    void SetStatus(SCANNER_STATUS status);
    SCANNER_STATUS GetStatus();
    void Abort();

private:
    const ScanJobType m_type;
    std::shared_ptr<BufferQueue<DirectoryScan>> m_bufferQueue {};
    std::vector<std::shared_ptr<DiffControlThread>> m_diffThreads {};
    int32_t m_diffThreadIndex {0};
    Module::Hash m_prevDirPathHash {0};
    Module::Hash m_curDirPathHash {0};
    SCAN_HASH_TYPE m_scanHashType = SHA_1;
#ifdef NAS_SNAPDIFF
    std::shared_ptr<BufferQueue<SnapdiffResultMap>> m_snapdiffBufferQueue;
#endif
    SCANNER_STATUS m_state;
    std::shared_ptr<StatisticsMgr> m_statsMgr;
    std::shared_ptr<FSScannerCheckPoint> m_chkPntMgr;
    std::shared_ptr<ScanFilter> m_scanFilter;
    std::shared_ptr<MetaFileManager> m_metafileManager;
    std::shared_ptr<CtrlFilterManager> m_ctrlFilterManager;
    std::vector<std::shared_ptr<ConsumerThread>> m_threads;
    std::vector<std::shared_ptr<CommonService>> m_commonservices {};
    bool m_precondition = false;
    std::string m_finalDcacheFile {};
    bool m_useFilter = false;
    uint64_t m_curDcacheIndex = 0;
    uint64_t m_prevDcacheIndex = 0;

    bool InitCommonService(ScanConfig &config, ScanInfo &info);
    SCANNER_STATUS PollMetaGenerate();
    bool RenameMtimeFiles(const std::string &ctrlPath, ScanConfig &config);
#ifdef NAS_SNAPDIFF
    SCANNER_STATUS PollSnapdiff();
#endif
#ifdef WIN32
    bool RenameTmpControlFiles(const std::string& ctrlPath, const std::string& ctrlFilePrefix);
#endif
    bool RenameDeleteFiles(const std::string &ctrlPath);
    SCANNER_STATUS ProcessMtimeFiles(ScanConfig &config);
    std::shared_ptr<DirMtimeService> CreateMtimeProducerObj(ScanConfig &config);
    void ReadCheckPointHardLinkFiles(std::shared_ptr<CommonService> servicePtr);
    SCANNER_STATUS StartDiff(std::string finalDcacheFile, bool isFull, ScanConfig &config);
    auto GetDiffThread(bool sendPrevThread = false);
    void SendDiffTask(CompareDirectory &dirObj);
    bool CheckUseSameThread(CompareDirectory &dirObj);
    bool CheckUseSameThreadWithCrc(CompareDirectory &dirObj);
    bool CheckUseSameThreadWithSha(CompareDirectory &dirObj);
    SCANNER_STATUS FindAllModifiedDirs(std::shared_ptr<Module::DirCacheParser> currDcacheObj,
        std::shared_ptr<Module::DirCacheParser> prevDcacheObj);
    SCANNER_STATUS ProcessCurrentMetaDir(std::shared_ptr<Module::DirCacheParser> dcQueue);
    void CompareDirPathHash(std::queue<Module::DirCache> &dcQueue1, std::queue<Module::DirCache> &dcQueue2);
    void CompareDirPathCRCHash(Module::DirCache &curDcache, Module::DirCache &prevDcache,
        std::queue<Module::DirCache> &dcQueue1, std::queue<Module::DirCache> &dcQueue2);
    void CompareDirPathSHA1Hash(Module::DirCache &curDcache, Module::DirCache &prevDcache,
        std::queue<Module::DirCache> &dcQueue1, std::queue<Module::DirCache> &dcQueue2);
    void ClearDiffThreads();
    void PollDiffThreads() const;
    void ExitAllDiffThreads() const;
    void FlushHardlinkMap() const;
    bool CheckIfMatch(const Module::DirCache& dcache);
    bool InitMetaFileManager(std::shared_ptr<MetaFileManager> metafileManager, ScanConfig &config, bool isFull);
    bool OpenMetaFiles(const std::string& metaDir,
        const std::tuple<uint32_t, uint32_t, uint32_t>& metafileCount,
        std::vector<std::shared_ptr<Module::MetaParser>>& metafiles,
        std::vector<std::shared_ptr<Module::XMetaParser>>& xmetafiles,
        std::vector<std::shared_ptr<Module::FileCacheParser>>& fcachefiles);
    bool ValidateMetaFiles(const std::string& metaDir,
        uint32_t& metaFileCount,
        uint32_t& xMetaFileCount,
        uint32_t& fcacheFileCount,
        std::string& metaVersion);
    bool FillDirMetaWrapper(std::pair<uint16_t, uint64_t> metaInfo,
        Module::DirMetaWrapper &dmWrapper, bool isCur);
    std::string GetDirPathByDCache(const Module::DirCache &dcache);
    bool CheckIfDCacheRangeMatch(std::queue<Module::DirCache> &dcQueue, bool isCur);
};

#endif
