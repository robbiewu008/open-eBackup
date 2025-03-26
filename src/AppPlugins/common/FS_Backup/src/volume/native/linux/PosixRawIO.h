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
#ifndef VOLUMEBACKUP_NATIVE_POSIX_RAW_IO_HEADER
#define VOLUMEBACKUP_NATIVE_POSIX_RAW_IO_HEADER

#ifdef __linux__

#include "common/VolumeProtectMacros.h"
#include "RawIO.h"

// Raw I/O Reader/Writer for *unix platform posix API implementation
namespace volumeprotect {
namespace rawio {
namespace posix {

// PosixRawDataReader can read from any block device or common file at given offset
class PosixRawDataReader : public RawDataReader {
public:
    explicit PosixRawDataReader(const std::string& path, int flag = 0, uint64_t shiftOffset = 0);
    ~PosixRawDataReader();
    bool Read(uint64_t offset, uint8_t* buffer, int length, ErrCodeType& errorCode) override;
    void ReopenFile() override;
    bool Ok() override;
    ErrCodeType Error() override;

private:
    int m_fd {};
    int m_flag { 0 };
    uint64_t m_shiftOffset { 0 };
    std::string m_path;
};

// PosixRawDataWriter can write to any block device or common file at give offset
class PosixRawDataWriter : public RawDataWriter {
public:
    explicit PosixRawDataWriter(const std::string& path, int flag = 0, uint64_t shiftOffset = 0);
    ~PosixRawDataWriter();
    bool Write(uint64_t offset, uint8_t* buffer, int length, ErrCodeType& errorCode) override;
    void ReopenFile() override;
    bool Ok() override;
    bool Flush() override;
    bool LockVolume() override;
    bool UnLockVolume() override;
    ErrCodeType Error() override;

private:
    int m_fd {};
    int m_flag { 0 };
    uint64_t m_shiftOffset { 0 };
    std::string m_path;
};

}
}
}

#endif
#endif