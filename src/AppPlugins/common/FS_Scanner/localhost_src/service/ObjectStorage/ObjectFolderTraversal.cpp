/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
* Description: Object storage bucket scanner.
* Author: w00444223
* Create: 2023-12-04
*/

#include "ObjectFolderTraversal.h"
#include "log/Log.h"
#include "ScanCommon.h"
#include "ObjectUtils.h"
#include "ObjectLogProcess.h"

using namespace std;
using namespace Module;
using namespace FS_SCANNER;

ObjectFolderTraversal::ObjectFolderTraversal(std::shared_ptr<StatisticsMgr> statsMgr,
    std::shared_ptr<FSScannerCheckPoint> chkPntMgr,
    std::shared_ptr<ScanFilter> scanFilter,
    std::shared_ptr<BufferQueue<DirectoryScan>> buffer,
    ScanConfig config) : FolderTraversal(statsMgr, chkPntMgr, scanFilter, buffer)
{
    m_config = config;
    m_input = make_shared<BufferQueue<DirStat>>(m_config.maxScanQueueSize);
    string scanOutputDir = m_config.metaPath + LATEST;
    m_scanQueue = make_shared<ScanQueue>(m_input, scanOutputDir, m_config.maxScanQueueSize, m_config.minScanQueueSize);
}

ObjectFolderTraversal::~ObjectFolderTraversal()
{}

void ObjectFolderTraversal::PushDirToScanQueue(string path, string prefix, uint8_t filterFlag)
{
    DirStat dirStat {};
    dirStat.m_path = path;
    dirStat.m_prefix = prefix;
    dirStat.m_filterFlag = filterFlag;

    dirStat.flag = static_cast<uint8_t>(DirStatFlag::BUCKET);

    m_statsMgr->IncrCommStatsByType(CommStatsType::TOTAL_DIRS);
    INFOLOG("Enqueue dir: %s, prefix: %s to scan queue", path.c_str(), prefix.c_str());
    m_input->Push(dirStat);
    return;
}

SCANNER_STATUS ObjectFolderTraversal::EnqueueDirToScanQueue(string path, string prefix, uint8_t filterFlag)
{
    m_chkPntMgr->m_prefix = prefix;
    PushDirToScanQueue(path, prefix, filterFlag);
    return SCANNER_STATUS::SUCCESS;
}

/*
 * 待备份的内容包括两部分：
 * 1、bucket桶: path = "bucketname", prefix = ""
 * 2、桶内设置的前缀prefix: path = "bucketprefix", prefix = "/bucketname"
 */
SCANNER_STATUS ObjectFolderTraversal::Enqueue(const string& path, const string& prefix, uint8_t filterFlag)
{
    INFOLOG("Enqueue path %s, prefix %s, filterFlag %u", path.c_str(), prefix.c_str(), filterFlag);
    return SCANNER_STATUS::SUCCESS;
}

SCANNER_STATUS ObjectFolderTraversal::Start()
{
    HCPTSP::getInstance().reset(m_config.reqID);
    if (m_config.obs.buckets.size() > MAX_BUCKET_NUM_ONCE) {
        ERRLOG("Over max bucket number (%d) for once scan.", MAX_BUCKET_NUM_ONCE);
        return SCANNER_STATUS::FAILED;
    }

    if (m_config.obs.buckets.size() == 0) {
        ERRLOG("There is no bucket.");
        return SCANNER_STATUS::FAILED;
    }

    if (m_config.obs.IncUseLog) {
        m_logTraversalThread = std::make_shared<std::thread>(std::bind(&ObjectFolderTraversal::LogTraversal, this));
        return SCANNER_STATUS::SUCCESS;
    }

    CreateMetaProducerThreads(EnqueueDirAndCount());

    return SCANNER_STATUS::SUCCESS;
}

SCANNER_STATUS ObjectFolderTraversal::Poll()
{
    if (m_status == TraversalStatus::Failed) {
        return SCANNER_STATUS::FAILED;
    }
    if (m_config.obs.IncUseLog && m_status == TraversalStatus::DeFault) {
            return SCANNER_STATUS::SCAN_IN_PROGRESS;
    }

    DBGLOG("EnableProduce: %d, QueueSize : %d, (inputPop: %d, dirDirectPush: %d) "
        "(bufferPush: %d, failedDir: %d, filterDiscardDir: %d)",
        m_config.enableProduce, m_input->GetSize(), m_input->GetPopCount(),
        m_statsMgr->GetCommStatsByType(CommStatsType::WRITE_QUEUE_DIRECTLY_PUSH_COUNT),
        m_bufferQueue->GetPushCount(), m_statsMgr->GetCommStatsByType(CommStatsType::TOTAL_FAILED_DIRS),
        m_statsMgr->GetCommStatsByType(CommStatsType::FILTER_DISCARD_DIR_COUNT)
    );

    if (m_input->Empty()) {
        ObjectStorageBucket bucket = m_config.obs.buckets.front();
        if (!m_config.enableProduce) {
            // when enableProduce is disabled, there's no need to check DirectoryScan queue (always empty)
            return SCANNER_STATUS::SCAN_READ_COMPLETED;
        }

        uint64_t toBeHandledCnt = m_statsMgr->GetCommStatsByType(CommStatsType::TOTAL_DIRS);
        uint64_t completeCnt = m_statsMgr->GetCommStatsByType(CommStatsType::WRITE_QUEUE_DIRECTLY_PUSH_COUNT)
            + m_statsMgr->GetCommStatsByType(CommStatsType::TOTAL_FAILED_DIRS);
        DBGLOG("toBeHandledCnt %lld, completeCnt %lld", toBeHandledCnt, completeCnt);

        if (toBeHandledCnt == completeCnt) {
            return SCANNER_STATUS::SCAN_READ_COMPLETED;
        }
    }

    return SCANNER_STATUS::SCAN_IN_PROGRESS;
}

void ObjectFolderTraversal::CreateMetaProducerThreads(int threadCount)
{
    DBGLOG("threadCount %d", threadCount);
    m_statsMgr->SetCommStatsByType(CommStatsType::SCAN_START_TIME, (Time::Now().MicroSeconds()));
    if (!m_scanFilter->AllFiltersDisabled() && !m_chkPntMgr->IsScanRestarted()) {
        WARNLOG("Filter path will be ignored.");
    }

    ProduceParams args {m_scanQueue, m_bufferQueue, m_statsMgr, m_scanFilter, m_chkPntMgr};
    
    for (int i = 0; i < threadCount; ++i) {
        auto workerThread = make_shared<ObjectProducerThread>(args, m_config);
        if (!workerThread->Start()) {
            m_status = TraversalStatus::Failed;
            return;
        }
        m_threads.push_back(workerThread);
    }
    m_status = TraversalStatus::Success;
}

void ObjectFolderTraversal::LogTraversal()
{
    auto logProcInst = std::make_shared<ObjectLogProcess>(m_config, m_bufferQueue, m_statsMgr);
    if (logProcInst->SortByPrefix() != Module::SUCCESS) {
        ERRLOG("Sort prefix failed.");
        m_status = TraversalStatus::Failed;
        return;
    }
    if (logProcInst->GenNonModifyMeta() != Module::SUCCESS) {
        ERRLOG("Generate the no modified meta failed.");
        m_status = TraversalStatus::Failed;
        return;
    }
    EnqueueDirAndCount();
    CreateMetaProducerThreads(m_config.producerThreadCount);
}

void ObjectFolderTraversal::ProcessCheckPointContainers()
{
    INFOLOG("Processing FSScannerCheckPoint Containers Started");

    for (auto workerThread : m_threads) {
        while (workerThread->m_isMetaProducerExecuting) {
            INFOLOG("Producer still running...");
            sleep(1);
        }
    }

    m_chkPntMgr->ProcessOutputQueue(m_bufferQueue);
    m_chkPntMgr->ProcessScanInProgressList();
    m_chkPntMgr->ProcessScanQueue(m_input);
    m_chkPntMgr->ProcessScanQueueBuffer(m_scanQueue->m_queueBuffer);
    m_chkPntMgr->ProcessScanQueueFiles(m_scanQueue->m_fileList);
    m_chkPntMgr->m_isProcessingQueuesDone = true;
    INFOLOG("Processing FSScannerCheckPoint Containers End");
}

int ObjectFolderTraversal::EnqueueDirAndCount()
{
    int count = 0;
    for (auto &item : m_config.obs.buckets) { // 放根目录
        if (item.prefix.empty()) {
            INFOLOG("There is no prefix for bucket %s.", item.bucketName.c_str());
            EnqueueDirToScanQueue(item.bucketName, "", 0);
            ++count;
            continue;
        }
        // 桶前缀的对象都放在以bucketname为根目录下，因此在dcache中增加bucketname目录
        
        for (auto &prefix : item.prefix) {
            DBGLOG("buckets name: %s, prefix: %s", item.bucketName.c_str(), prefix.c_str());
            EnqueueDirToScanQueue(item.bucketName, prefix, FLAG_NON_RECURSIVE);
            ++count;
        }
    }
    return count;
}

SCANNER_STATUS ObjectFolderTraversal::Suspend()
{
    return SCANNER_STATUS::SUCCESS;
}

SCANNER_STATUS ObjectFolderTraversal::Resume()
{
    return SCANNER_STATUS::SUCCESS;
}

SCANNER_STATUS ObjectFolderTraversal::Abort()
{
    return SCANNER_STATUS::SUCCESS;
}

SCANNER_STATUS ObjectFolderTraversal::Destroy()
{
    for (auto workerThread : m_threads) {
        workerThread->Exit();
    }
    m_threads.clear();

    if (m_logTraversalThread != nullptr && m_logTraversalThread->joinable()) {
        m_logTraversalThread->join();
    }
    FS_SCANNER::MemoryTrim();
    return SCANNER_STATUS::SUCCESS;
}
