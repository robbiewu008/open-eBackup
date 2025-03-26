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
#ifndef VOLUMEBACKUP_BLOCK_WRITER_HEADER
#define VOLUMEBACKUP_BLOCK_WRITER_HEADER

#include "common/VolumeProtectMacros.h"
#include "VolumeProtectTaskContext.h"
#include "common/Thread.h"
#include "native/RawIO.h"

namespace volumeprotect {
namespace task {

enum class TargetType {
    VOLUME = 0,
    COPYFILE = 1
};

/**
 * @brief param to build a block reader
 */
struct VolumeBlockWriterParam {
    TargetType      targetType;
    std::string     targetPath;
    std::shared_ptr<VolumeTaskSharedConfig>     sharedConfig;
    std::shared_ptr<VolumeTaskSharedContext>    sharedContext;
    std::shared_ptr<rawio::RawDataWriter>         dataWriter;
};

/**
 * @brief Independent routine to keep consuming block from queue and perform write operation to volume of copy file
 */
class VolumeBlockWriter : public StatefulTask {
public:
    // build a writer writing to copy file
    static std::shared_ptr<VolumeBlockWriter> BuildCopyWriter(
        std::shared_ptr<VolumeTaskSharedConfig> sharedConfig,
        std::shared_ptr<VolumeTaskSharedContext> sharedContext);

    // build a writer writing to volume
    static std::shared_ptr<VolumeBlockWriter> BuildVolumeWriter(
        std::shared_ptr<VolumeTaskSharedConfig> sharedConfig,
        std::shared_ptr<VolumeTaskSharedContext> sharedContext,
        ErrCodeType& errorCode);

    bool Start();

    ~VolumeBlockWriter();

    explicit VolumeBlockWriter(const VolumeBlockWriterParam& param);

    bool Flush();

private:
    bool NeedToWrite(uint8_t* buffer, int length) const;

    void MainThread();

    void HandleWriteError(ErrCodeType errorCode);

    bool SafeWrite(uint64_t offset, uint8_t* buffer, int length, ErrCodeType& errorCode);

private:
    // immutable fields
    TargetType      m_targetType;
    std::string     m_targetPath;
    std::shared_ptr<VolumeTaskSharedConfig>                 m_sharedConfig  { nullptr };

    // mutable fields
    std::shared_ptr<VolumeTaskSharedContext>                m_sharedContext { nullptr };
    std::thread                                             m_writerThread;
    std::shared_ptr<volumeprotect::rawio::RawDataWriter>    m_dataWriter    { nullptr };
    uint8_t m_retryInterval { DEFAULT_RETRY_SECOND };
    uint8_t m_retryTimes    { DEFAULT_RETRY_TIMES };
};

}
}

#endif