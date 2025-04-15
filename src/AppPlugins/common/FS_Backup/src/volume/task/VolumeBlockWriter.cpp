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
#include "common/VolumeProtectMacros.h"
#include "VolumeProtector.h"
#include "native/RawIO.h"
#include "VolumeBlockWriter.h"

using namespace volumeprotect;
using namespace volumeprotect::task;
using namespace volumeprotect::rawio;

// build a writer writing to copy file
std::shared_ptr<VolumeBlockWriter> VolumeBlockWriter::BuildCopyWriter(
    std::shared_ptr<VolumeTaskSharedConfig> sharedConfig,
    std::shared_ptr<VolumeTaskSharedContext> sharedContext)
{
    std::string copyFilePath = sharedConfig->copyFilePath;
    // init data writer
    SessionCopyRawIOParam sessionIOParam {};
    sessionIOParam.copyFormat = sharedConfig->copyFormat;
    sessionIOParam.volumeOffset = sharedConfig->sessionOffset;
    sessionIOParam.length = sharedConfig->sessionSize;
    sessionIOParam.copyFilePath = sharedConfig->copyFilePath;
    sessionIOParam.shareName = sharedConfig->shareName;

    std::shared_ptr<RawDataWriter> dataWriter = rawio::OpenRawDataCopyWriter(sessionIOParam);
    if (dataWriter == nullptr) {
        ERRLOG("failed to build copy data writer");
        return nullptr;
    }
    if (!dataWriter->Ok()) {
        ERRLOG("failed to init copy data writer, format = %d, copyfile = %s, error = %u",
            sharedConfig->copyFormat, sharedConfig->copyFilePath.c_str(), dataWriter->Error());
        return nullptr;
    }
    if (!dataWriter->LockVolume()) {
        ERRLOG("failed to lock volume!");
        return nullptr;
    }
    VolumeBlockWriterParam param {
        TargetType::COPYFILE,
        copyFilePath,
        sharedConfig,
        sharedContext,
        dataWriter
    };
    return std::make_shared<VolumeBlockWriter>(param);
}

// build a writer writing to volume
std::shared_ptr<VolumeBlockWriter> VolumeBlockWriter::BuildVolumeWriter(
    std::shared_ptr<VolumeTaskSharedConfig> sharedConfig,
    std::shared_ptr<VolumeTaskSharedContext> sharedContext,
    ErrCodeType& errorCode)
{
    std::string volumePath = sharedConfig->volumePath;
    // check target block device valid to write
    std::shared_ptr<RawDataWriter> dataWriter = rawio::OpenRawDataVolumeWriter(volumePath, sharedConfig->copyFormat);
    if (dataWriter == nullptr) {
        ERRLOG("failed to build volume data reader");
        return nullptr;
    }
    if (!dataWriter->Ok()) {
        errorCode = dataWriter->Error();
        ERRLOG("failed to init VolumeDataWriter, path = %s, error = %u",
            volumePath.c_str(), errorCode);
        return nullptr;
    }
    VolumeBlockWriterParam param {
        TargetType::VOLUME,
        volumePath,
        sharedConfig,
        sharedContext,
        dataWriter
    };
    return std::make_shared<VolumeBlockWriter>(param);
}

bool VolumeBlockWriter::Start()
{
    AssertTaskNotStarted();
    m_status = TaskStatus::RUNNING;
    // check data writer
    if (!m_dataWriter || !m_dataWriter->Ok()) {
        ERRLOG("invalid dataWriter %p, path = %s", m_dataWriter.get(), m_targetPath.c_str());
        m_status = TaskStatus::FAILED;
        return false;
    }
    m_writerThread = std::thread(&VolumeBlockWriter::MainThread, this);
    return true;
}

bool VolumeBlockWriter::Flush()
{
    return m_dataWriter->Flush();
}

VolumeBlockWriter::~VolumeBlockWriter()
{
    DBGLOG("destroy VolumeBlockWriter");
    if (m_writerThread.joinable()) {
        m_writerThread.join();
    }
    m_dataWriter.reset();
}

VolumeBlockWriter::VolumeBlockWriter(const VolumeBlockWriterParam& param)
    : m_targetType(param.targetType),
    m_targetPath(param.targetPath),
    m_sharedConfig(param.sharedConfig),
    m_sharedContext(param.sharedContext),
    m_dataWriter(param.dataWriter)
{}

bool VolumeBlockWriter::NeedToWrite(uint8_t* buffer, int length) const
{
    if (!m_sharedConfig->skipEmptyBlock) {
        return true;
    }
    if (buffer[0] == 0 && !::memcmp(buffer, buffer + 1, length - 1)) {
        // is all zero
        return false;
    }
    return true;
}

bool VolumeBlockWriter::SafeWrite(uint64_t offset, uint8_t* buffer, int length, ErrCodeType& errorCode)
{
    for (int i = 0; i < m_retryTimes; ++i) {
        if (m_dataWriter->Write(offset, buffer, length, errorCode)) {
            return true;
        }
        ERRLOG("write %d bytes at %llu failed, error code = %u, retry times is %d",
            length, offset, errorCode, i);
        Module::SleepFor(std::chrono::seconds(m_retryInterval));
        if (errorCode == ENOTCONN || errorCode == EBADF) { // dataturbo重启，fd会失效，返回ENOTCONN错误码
            m_dataWriter->ReopenFile();
            if (!m_dataWriter->Ok()) {
                ERRLOG("open failed, error code = %u, retry times is %d", errorCode, i);
            }
        }
    }
    return false;
}

void VolumeBlockWriter::MainThread()
{
    VolumeConsumeBlock consumeBlock {};
    ErrCodeType errorCode = 0;
    INFOLOG("writer thread start");

    while (true) {
        DBGLOG("writer thread check");
        if (m_abort) {
            m_status = TaskStatus::ABORTED;
            break;
        }

        if (!m_sharedContext->writeQueue->BlockingPop(consumeBlock)) {
            // queue has been finished
            m_status = TaskStatus::SUCCEED;
            break;
        }

        uint8_t* buffer = consumeBlock.ptr;
        uint64_t writerOffset = consumeBlock.volumeOffset;
        uint32_t length = consumeBlock.length;
        uint64_t index = consumeBlock.index;

        if (m_failed) {
            DBGLOG("block writer has failed, skip any write request");
            m_sharedContext->allocator->Bfree(buffer);
            ++m_sharedContext->counter->blockesWriteFailed;
            continue;
        }

        DBGLOG("write block[%llu] (%p, %llu, %u) writerOffset = %llu",
            index, buffer, consumeBlock.volumeOffset, length, writerOffset);
        if (NeedToWrite(buffer, length) &&
            !SafeWrite(writerOffset, buffer, length, errorCode)) {
            ERRLOG("write %d bytes at %llu failed, error code = %u", length, writerOffset, errorCode);
            m_sharedContext->allocator->Bfree(buffer);
            ++m_sharedContext->counter->blockesWriteFailed;
            HandleWriteError(errorCode);
            // writer should not return (otherwise writer queue may block reader)
        }

        m_sharedContext->writtenBitmap->Set(index);
        m_sharedContext->processedBitmap->Set(index);
        m_sharedContext->allocator->Bfree(buffer);
        m_sharedContext->counter->bytesWritten += length;
    }
    if (m_status == TaskStatus::SUCCEED && m_sharedContext->counter->blockesWriteFailed != 0) {
        m_status = TaskStatus::FAILED;
        ERRLOG("%llu blockes fail to write, set writer to fail", m_sharedContext->counter->blockesWriteFailed.load());
    }
    INFOLOG("writer read terminated with status %s", GetStatusString().c_str());
    m_dataWriter->UnLockVolume();
    return;
}

void VolumeBlockWriter::HandleWriteError(ErrCodeType errorCode)
{
    m_failed = true;
    m_errorCode = errorCode;
#ifdef __linux__
    if (errorCode == EACCES || errorCode == EPERM) {
        m_errorCode = (m_targetType == TargetType::COPYFILE) ?
        VOLUMEPROTECT_ERR_COPY_ACCESS_DENIED : VOLUMEPROTECT_ERR_VOLUME_ACCESS_DENIED;
    } else if (m_targetType == TargetType::COPYFILE && errorCode == ENOSPC) {
        m_errorCode = VOLUMEPROTECT_ERR_NO_SPACE;
    }
    INFOLOG("set error code to %u", m_errorCode);
#endif
}