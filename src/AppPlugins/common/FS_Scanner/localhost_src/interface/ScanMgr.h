/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * @file Scanner.h
 * @date 8/9/2022
 * @author z30016470
 * @brief
 */
#ifndef FS_SCANNER_MGR_H
#define FS_SCANNER_MGR_H

#include <memory>
#include <list>
#include <vector>
#include <map>
#include <mutex>
#include "Scanner.h"
#include "define/Defines.h"

struct Task {
    std::string jobId;
    int jobPriority;
    Task(const std::string& jobId, int jobPriority) : jobId(jobId), jobPriority(jobPriority) {}
};

class AGENT_API ScanMgr {
public:
    static ScanMgr& GetInstance();
    static std::unique_ptr<Scanner> CreateScanInst(const ScanConfig& scanConfig);
    static Scanner *NewScanInst(const ScanConfig& scanConfig);
    static void InitLog(const std::string& fullLogPath, int logLevel);
    static bool ValidateScanConfig(const ScanConfig& scanConfig);
    // method to init path mapper
    static std::shared_ptr<PathMapper> BuildPrefixPathMapper(const std::string& prefix);
    static std::shared_ptr<PathMapper> BuildInfixPathMapper(std::size_t pos, const std::string& infixString);

    void RegisterTask(ScanTaskLevel priority, const std::string& jobId);
    bool HoldRunningTask(const std::string& jobId);
    bool ReleaseRunningTask(const std::string& jobId);
    bool Initiate(const ScanConfig& scanConfig);
    void Enqueue(const std::string& directory, const std::string& prefix = "", uint8_t filterFlag = 0);
    SCANNER_STATUS EnqueueV2(const std::string& path);
    SCANNER_STATUS StartScan();
    SCANNER_STATUS GetScanStatus();
    ScanStatistics GetScanStats();
    SCANNER_STATUS AbortScan();
    void Destroy();
    ErrRecorder QueryFail();

private:
    using PriorityTaskQueue = std::map<int, std::list<std::string>>;
    ScanMgr();
    ~ScanMgr();
    ScanMgr(const ScanMgr&);
    ScanMgr& operator=(const ScanMgr&);

    void PushJobToTaskList(ScanTaskLevel priority, const std::string& jobId);
    std::string TopTask();
    void TopTaskLevelUpToRunning(const std::string& jobId);
    void CancelTask(const std::string& jobId);
    void RemoveRunningTask();
    void PrintDebug();

    void InsertTask(const Task& task);
    bool removeTask(std::string jobId);

    std::unique_ptr<Scanner> m_scanner = nullptr;
    std::list<Task> m_taskList;
    std::mutex m_mtx;
};

#endif