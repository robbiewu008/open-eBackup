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
#include "PosixDirWriter.h"
#include "PosixConstants.h"
#include "ThreadPoolFactory.h"
#include "OsPlatformDefines.h"
#include "log/Log.h"
#include "FSBackupUtils.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    const int RETRY_TIME_MILLISENCOND = 1000;
}

PosixDirWriter::PosixDirWriter(
    const WriterParams &dirWriterParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder)
    : HostDirWriter(dirWriterParams, failureRecorder)
{
    INFOLOG("Construct PosixDirWriter!");
}

PosixDirWriter::~PosixDirWriter()
{
    INFOLOG("Destruct PosixDirWriter!");
    if (m_thread.joinable()) {
        m_thread.join();
    }
}
void PosixDirWriter::HandleFailedEvent(shared_ptr<OsPlatformServiceTask> taskPtr)
{
    FileHandle& fileHandle = taskPtr->m_fileHandle;
    fileHandle.m_retryCnt++;

    DBGLOG("Posix dir failed %s retry cnt %d", fileHandle.m_file->m_fileName.c_str(), fileHandle.m_retryCnt);

    if (fileHandle.m_retryCnt >= DEFAULT_ERROR_SINGLE_FILE_CNT ||
        taskPtr->IsCriticalError()) {
        FSBackupUtils::RecordFailureDetail(m_failureRecorder, taskPtr->m_errDetails);

        if (fileHandle.m_file->GetDstState() != FileDescState::WRITE_FAILED) {
            fileHandle.m_file->SetDstState(FileDescState::WRITE_FAILED);
            fileHandle.m_errNum = taskPtr->m_errDetails.second;
            m_failedList.emplace_back(fileHandle);
        }
        ++m_controlInfo->m_noOfDirFailed;
        if (!m_backupParams.commonParams.skipFailure || taskPtr->IsCriticalError()) {
            ERRLOG("set backup to failed!");
            m_controlInfo->m_failed = true;
            m_controlInfo->m_backupFailReason = taskPtr->m_backupFailReason;
        }
        ERRLOG("dir failed for dir %s %d",
            fileHandle.m_file->m_fileName.c_str(), m_controlInfo->m_noOfDirFailed.load());
        return;
    }

    m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);

    return;
}