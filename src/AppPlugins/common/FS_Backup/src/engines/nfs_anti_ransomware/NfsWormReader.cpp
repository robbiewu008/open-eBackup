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
#include "NfsWormReader.h"

using namespace std;

namespace {
    const int QUEUE_TIMEOUT_MILLISECOND = 10;
    
}

NfsWormReader::NfsWormReader(const AntiReaderParams &antiReaderParams) : AntiRansomwareReader(antiReaderParams)
{
    m_advParams = dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(m_backupParams.srcAdvParams);
}

NfsWormReader::~NfsWormReader()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

BackupRetCode NfsWormReader::Start()
{
    DBGLOG("NfsWormReader start!");
    try {
        m_thread = thread(&NfsWormReader::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode NfsWormReader::Abort()
{
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus NfsWormReader::GetStatus()
{
    return FSBackupUtils::GetReaderStatus(m_controlInfo, m_abort);
}

bool NfsWormReader::IsAbort() const
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        return true;
    }
    return false;
}

bool NfsWormReader::IsComplete() const
{
    if ((m_controlInfo->m_controlFileReaderProduce == m_controlInfo->m_readConsume) &&
        m_controlInfo->m_controlReaderPhaseComplete &&
        m_readQueue->Empty()) {
        INFOLOG("NfsWormReader complete");
        return true;
    }
    return false;
}

void NfsWormReader::HandleComplete()
{
    INFOLOG("Complete NfsWormReader");
    m_controlInfo->m_readPhaseComplete = true;
}

void NfsWormReader::ThreadFunc()
{
    INFOLOG("NfsWormReader main thread start!");
    while (true) {
        if (IsAbort()) {
            WARNLOG("NfsWormReader main thread abort!");
            break;
        }
        FileHandle fileHandle;
        bool ret = m_readQueue->WaitAndPop(fileHandle, QUEUE_TIMEOUT_MILLISECOND);
        if (ret) {
            DBGLOG("get file desc from read queue :%s.", fileHandle.m_file->m_fileName.c_str());
            ++m_controlInfo->m_readConsume;
            m_writeQueue->WaitAndPush(fileHandle);
            ++m_controlInfo->m_readProduce;
        }

        if (IsComplete()) {
            break;
        }
    }
    HandleComplete();
    INFOLOG("NfsWormReader main thread end!");
    return;
}

int NfsWormReader::OpenFile(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return Module::SUCCESS;
}

int NfsWormReader::ReadData(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return Module::SUCCESS;
}

int NfsWormReader::ReadMeta(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return Module::SUCCESS;
}

int NfsWormReader::CloseFile(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return Module::SUCCESS;
}