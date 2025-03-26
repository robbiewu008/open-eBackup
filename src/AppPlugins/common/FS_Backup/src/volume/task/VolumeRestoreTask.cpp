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
#include "log/Log.h"
#include "VolumeProtector.h"
#include "VolumeProtectTaskContext.h"
#include "common/VolumeUtils.h"
#include "VolumeBlockReader.h"
#include "VolumeBlockWriter.h"
#include "common/BlockingQueue.h"
#include "VolumeRestoreTask.h"

using namespace volumeprotect;
using namespace volumeprotect::task;
using namespace volumeprotect::common;

namespace {
    constexpr auto TASK_CHECK_SLEEP_INTERVAL = std::chrono::seconds(1);
}

static std::vector<std::string> GetCopyFilesFromCopyMeta(const VolumeCopyMeta& volumeCopyMeta)
{
    std::vector<std::string> files;
    for (const auto& segment : volumeCopyMeta.segments) {
        files.push_back(segment.copyDataFile);
    }
    return files;
}

VolumeRestoreTask::VolumeRestoreTask(const VolumeRestoreConfig& restoreConfig, const VolumeCopyMeta& volumeCopyMeta)
    : m_restoreConfig(std::make_shared<VolumeRestoreConfig>(restoreConfig)),
    m_volumeCopyMeta(std::make_shared<VolumeCopyMeta>(volumeCopyMeta)),
    m_resourceManager(TaskResourceManager::BuildRestoreTaskResourceManager(RestoreTaskResourceManagerParams {
        static_cast<CopyFormat>(volumeCopyMeta.copyFormat),
        restoreConfig.copyDataDirPath,
        volumeCopyMeta.copyName,
        restoreConfig.shareName,
        GetCopyFilesFromCopyMeta(volumeCopyMeta)
    }))
{}

VolumeRestoreTask::~VolumeRestoreTask()
{
    DBGLOG("destroy volume restore task, wait main thread to join");
    if (m_thread.joinable()) {
        m_thread.join();
    }
    DBGLOG("reset restore resource manager");
    m_resourceManager.reset();
    DBGLOG("volume restore task destroyed");
}

bool VolumeRestoreTask::Start()
{
    AssertTaskNotStarted();
    if (!Prepare()) {
        DetachCopyResource();
        ERRLOG("prepare task failed");
        m_status = TaskStatus::FAILED;
        return false;
    }
    INFOLOG("restore %s checkpoint enabled %d",
        m_volumeCopyMeta->volumePath.c_str(), m_restoreConfig->enableCheckpoint);
    m_status = TaskStatus::RUNNING;
    m_thread = std::thread(&VolumeRestoreTask::ThreadFunc, this);
    return true;
}

TaskStatistics VolumeRestoreTask::GetStatistics() const
{
    std::lock_guard<std::mutex> lock(m_statisticMutex);
    return m_completedSessionStatistics + m_currentSessionStatistics;
}

// split session and write back
bool VolumeRestoreTask::Prepare()
{
    std::string volumePath = m_restoreConfig->volumePath;

    // 2. prepare restore resource
    if (!m_resourceManager->PrepareCopyResource()) {
        ERRLOG("failed to prepare copy resource for restore task");
        return false;
    }

    // 4. split session
    uint64_t volumeSize = m_volumeCopyMeta->volumeSize;
    CopyFormat copyFormat = static_cast<CopyFormat>(m_volumeCopyMeta->copyFormat);
    for (const CopySegment& segment: m_volumeCopyMeta->segments) {
        uint64_t sessionOffset = segment.offset;
        uint64_t sessionSize = segment.length;
        int sessionIndex = segment.index;
        INFOLOG("Size = %llu sessionOffset %d sessionSize %d", volumeSize, sessionOffset, sessionSize);
        std::string copyFilePath = common::GetCopyDataFilePath(
            m_restoreConfig->copyDataDirPath, m_volumeCopyMeta->copyName, copyFormat, sessionIndex);
        VolumeTaskSession session {};
        session.sharedConfig = std::make_shared<VolumeTaskSharedConfig>();
        session.sharedConfig->copyFormat = static_cast<CopyFormat>(m_volumeCopyMeta->copyFormat);
        session.sharedConfig->volumePath = volumePath;
        session.sharedConfig->hasherEnabled = false;
        session.sharedConfig->blockSize = m_volumeCopyMeta->blockSize;
        session.sharedConfig->sessionOffset = sessionOffset;
        session.sharedConfig->sessionSize = sessionSize;
        session.sharedConfig->copyFilePath = copyFilePath;
        session.sharedConfig->checkpointEnabled = m_restoreConfig->enableCheckpoint;
        session.sharedConfig->skipEmptyBlock = false;
        session.sharedConfig->shareName = m_restoreConfig->shareName;
        m_sessionQueue.push(session);
    }

    return true;
}

bool VolumeRestoreTask::InitRestoreSessionTaskExecutor(std::shared_ptr<VolumeTaskSession> session)
{
    session->readerTask = VolumeBlockReader::BuildCopyReader(
        session->sharedConfig,
        session->sharedContext);
    if (session->readerTask == nullptr) {
        ERRLOG("restore session failed to init reader task");
        return false;
    }
    ErrCodeType errorCode;
    // 4. check and init writer
    session->writerTask = VolumeBlockWriter::BuildVolumeWriter(
        session->sharedConfig,
        session->sharedContext,
        errorCode);
    if (session->writerTask == nullptr) {
        ERRLOG("restore session failed to init writer task");
        m_errorCode = errorCode;
        return false;
    }
    return true;
}

bool VolumeRestoreTask::InitRestoreSessionContext(std::shared_ptr<VolumeTaskSession> session)
{
    DBGLOG("init restore session context");
    // 1. init basic restore container
    session->sharedContext = std::make_shared<VolumeTaskSharedContext>();
    session->sharedContext->counter = std::make_shared<SessionCounter>();
    session->sharedContext->allocator = std::make_shared<VolumeBlockAllocator>(
        session->sharedConfig->blockSize,
        DEFAULT_ALLOCATOR_BLOCK_NUM);
    session->sharedContext->writeQueue = std::make_shared<BlockingQueue<VolumeConsumeBlock>>(DEFAULT_QUEUE_SIZE);
    InitSessionBitmap(session);
    // 2. restore checkpoint if restarted
    RestoreSessionCheckpoint(session);
    // 3. check and init task executor
    return InitRestoreSessionTaskExecutor(session);
}

bool VolumeRestoreTask::StartRestoreSession(std::shared_ptr<VolumeTaskSession> session)
{
    DBGLOG("start restore session");
    if (session->readerTask == nullptr || session->writerTask == nullptr) {
        ERRLOG("restore session member nullptr! readerTask: %p writerTask: %p ",
            session->readerTask.get(), session->writerTask.get());
        return false;
    }
    DBGLOG("start restore session reader");
    if (!session->readerTask->Start()) {
        ERRLOG("restore session readerTask start failed");
        return false;
    }
    DBGLOG("start restore session writer");
    if (!session->writerTask->Start()) {
        ERRLOG("restore session writerTask start failed");
        return false;
    }
    return true;
}

void VolumeRestoreTask::ThreadFunc()
{
    DBGLOG("start task main thread");
    while (!m_sessionQueue.empty()) {
        if (m_abort) {
            m_status = TaskStatus::ABORTED;
            return;
        }

        // pop a session from session queue to init a new session
        std::shared_ptr<VolumeTaskSession> session = std::make_shared<VolumeTaskSession>(m_sessionQueue.front());
        m_sessionQueue.pop();

        if (!InitRestoreSessionContext(session)) {
            m_status = TaskStatus::FAILED;
            return;
        }
        RestoreSessionCheckpoint(session);
        if (!StartRestoreSession(session)) {
            session->Abort();
            m_status = TaskStatus::FAILED;
            return;
        }
        // block the thread
        while (true) {
            if (m_abort) {
                session->Abort();
                m_status = TaskStatus::ABORTED;
                return;
            }
            if (session->IsFailed()) {
                ERRLOG("session failed");
                m_status = TaskStatus::FAILED;
                m_errorCode = session->GetErrorCode();
                return;
            }
            if (session->IsTerminated())  {
                break;
            }
            UpdateRunningSessionStatistics(session);
            RefreshSessionCheckpoint(session);
            std::this_thread::sleep_for(TASK_CHECK_SLEEP_INTERVAL);
        }
        DBGLOG("session complete successfully");
        FlushSessionWriter(session);
        FlushSessionBitmap(session);
        UpdateCompletedSessionStatistics(session);
    }
    DetachCopyResource();
    m_status = TaskStatus::SUCCEED;
    return;
}

void VolumeRestoreTask::DetachCopyResource()
{
    if (!m_resourceManager->DetachCopyResource()) {
        ERRLOG("Detach copy resource failed!");
        m_status = TaskStatus::FAILED;
    }
    return;
}