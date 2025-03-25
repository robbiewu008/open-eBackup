/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * @file ScannerImpl.h
 * @date 6/22/2022
 * @author z30016470
 * @brief
 */

#ifndef FS_SCANNER_IMPL_H
#define FS_SCANNER_IMPL_H
#include <memory>
#include "Scanner.h"
#include "ScanConsts.h"
#include "ControlGenerator.h"
#include "FolderTraversal.h"
#include "StatisticsMgr.h"
#include "FSScannerCheckPoint.h"

class ScannerImpl : public Scanner {
public:
    explicit ScannerImpl(const ScanConfig& scanConfig);
    ~ScannerImpl() override;

    SCANNER_STATUS Start() override;
    SCANNER_STATUS Abort() override;
    SCANNER_STATUS Destroy() override;
    SCANNER_STATUS Enqueue(
        const std::string& path,
        const std::string& prefix = "",
        uint8_t filterFlag = 0) override;
    SCANNER_STATUS EnqueueV2(const std::string& path) override;
#ifdef WIN32
    SCANNER_STATUS EnqueueWithoutSnapshot(
        const std::string& path,
        const std::string& prefix,
        uint8_t filterFlag) override;
#endif
    SCANNER_STATUS GetStatus() override;
    ScanStatistics GetStatistics() override;
    ErrRecorder QueryFailure() override;

private:
    /* engine initialization implemenetation */
    void InitDefaultEngine(); /* used by CONTROL_GEN to generate ctrl file from dcache/fcache */
#ifdef WIN32
    void InitWin32Engine();
#else
    void InitPosixEngine();
    void InitObjectStorageEngine();
#endif
#ifdef _NAS
    void InitCIFSEngine();
    void InitNFSEngine();
#endif
#ifdef NAS_SNAPDIFF
    void InitSnapdiffEngine();
#endif

    void PrintScanCfg() const;
    void PrintNfsScanCfg() const;
    void MainThread();
    void PreScanJob();
    void SetScanHashType();
    void SetOutputDirForScan();
    void StartGenerator();
    void PollTraversal();
    void PollGenerator();
    bool CheckAndCreateOutputPath(std::string dirPath) const;
    void MetaInProgressEvent();
    void MetaInFlushEvent();
    void MetaCompletedEvent();
    void CacheInProgressEvent();
    void CacheCompletedEvent();
    void CtrlDiffInProgressEvent();
    void CtrlDiffCompletedEvent();
    void CleanInProgressEvent();
    void CleanCompletedEvent();
    void HandleJobFailure();
    void GenerateMeta();
#ifdef NAS_SNAPDIFF
    void GenerateSnapdiffMeta();
#endif
    void GenerateDiffControl();
    void ScanCheckPoint(bool isFirstCheckPoint);
    void DebugSmbProducer() const;
    void PrintTimeStats() const;
    void ProcessIfArchive();
    // 通过获取当前线程的errno记录路径操作的错误信息
    void RecordCheckPathErrMsg(const std::string &path) const;

    /* private members */
private:
    bool m_isDestroyed = false;
    ScanInfo m_info;
    std::shared_ptr<StatisticsMgr> m_statsMgr = nullptr;
    SCANNER_STATUS m_status;
    std::unique_ptr<FolderTraversal> m_traversalPtr = nullptr;
    std::unique_ptr<ControlGenerator> m_generatorPtr = nullptr;
    std::thread m_mainThread {};
    std::shared_ptr<BufferQueue<DirectoryScan>> m_buffer = nullptr;
#ifdef NAS_SNAPDIFF
    std::shared_ptr<BufferQueue<SnapdiffResultMap>> m_snapdiffBuffer = nullptr;
#endif
    std::shared_ptr<FSScannerCheckPoint> m_chkPntMgr = nullptr;
    std::shared_ptr<ScanFilter> m_filter = nullptr;
    std::shared_ptr<Module::BackupFailureRecorder> m_failureRecorder = nullptr;
    std::mutex m_mtx {};
};

#endif