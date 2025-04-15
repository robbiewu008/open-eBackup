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
#ifndef VOLUMEBACKUP_BLOCK_READER_HEADER
#define VOLUMEBACKUP_BLOCK_READER_HEADER

#include "VolumeProtectTaskContext.h"
#include "common/Thread.h"
#include "native/RawIO.h"

namespace volumeprotect {
namespace task {

enum class SourceType {
    VOLUME = 0,
    COPYFILE = 1
};

/**
 * @brief param to build a block reader
 */
struct VolumeBlockReaderParam {
    SourceType  sourceType;
    std::string sourcePath;
    uint64_t    sourceOffset;
    std::shared_ptr<rawio::RawDataReader>         dataReader;
    std::shared_ptr<VolumeTaskSharedConfig>     sharedConfig;
    std::shared_ptr<VolumeTaskSharedContext>    sharedContext;
};

/**
 * @brief Independent routine to keep reading block from volume or copy file and then push to queue
 */
class VolumeBlockReader : public StatefulTask {
public:
    // build a reader reading from volume (block device)
    static std::shared_ptr<VolumeBlockReader> BuildVolumeReader(
        std::shared_ptr<VolumeTaskSharedConfig> sharedConfig,
        std::shared_ptr<VolumeTaskSharedContext> sharedContext);

    // build a reader reading from volume copy
    static std::shared_ptr<VolumeBlockReader> BuildCopyReader(
        std::shared_ptr<VolumeTaskSharedConfig> sharedConfig,
        std::shared_ptr<VolumeTaskSharedContext> sharedContext);

    bool Start();

    ~VolumeBlockReader();

    // provide to gmock or builder, not recommended to use
    explicit VolumeBlockReader(const VolumeBlockReaderParam& param);

    // block reading for updating checkpoint
    void Pause();

    void Resume();

private:
    void MainThread();

    uint64_t InitCurrentIndex() const;

    void BlockingPushForward(const VolumeConsumeBlock& consumeBlock) const;

    bool SkipReadingBlock() const;

    bool IsReadCompleted() const;

    void RevertNextBlock();

    uint8_t* FetchBlockBuffer(std::chrono::seconds timeout) const;

    bool ReadBlock(uint8_t* buffer, uint32_t& nBytesReaded);

    void HandleReadError(ErrCodeType errorCode);

    bool SafeRead(uint64_t offset, uint8_t* buffer, int length, ErrCodeType& errorCode);

private:
    // immutable fields
    SourceType  m_sourceType;
    std::string m_sourcePath;
    uint64_t    m_baseOffset;     // base offset
    std::shared_ptr<VolumeTaskSharedConfig>                 m_sharedConfig;

    // mutable fields
    std::shared_ptr<VolumeTaskSharedContext>                m_sharedContext;
    std::thread                                             m_readerThread;
    std::shared_ptr<volumeprotect::rawio::RawDataReader>    m_dataReader;

    uint64_t    m_maxIndex      { 0 };
    uint64_t    m_currentIndex  { 0 };
    bool        m_pause         { false };
    uint8_t     m_retryInterval { DEFAULT_RETRY_SECOND };
    uint8_t     m_retryTimes    { DEFAULT_RETRY_TIMES };
};

}
}

#endif