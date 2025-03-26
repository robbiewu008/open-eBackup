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
#ifndef VOLUMEBACKUP_NATIVE_RAW_IO_HEADER
#define VOLUMEBACKUP_NATIVE_RAW_IO_HEADER

#include "common/VolumeProtectMacros.h"
#include "VolumeProtector.h"
#include <string>


namespace volumeprotect {
/**
 * @brief this module is used to shield native I/O interface differences to provide a unified I/O layer
 */
namespace rawio {

/**
 * @brief RawDataReader provide basic raw I/O interface for VolumeDataReader, FileDataReader to implement.
 *  Implement this interface if need to access other data source, ex: cloud, tape ...
 */
class RawDataReader {
public:
    virtual bool Read(uint64_t offset, uint8_t* buffer, int length, ErrCodeType& errorCode) = 0;

    virtual bool Ok() = 0;

    virtual void ReopenFile() = 0;

    virtual ErrCodeType Error() = 0;

    virtual ~RawDataReader() = default;
};

/**
 * @brief RawDataWriter provide basic raw I/O interface for VolumeDataWriter, FileDataWriter to implement.
 *  Implement this interface if need to access other data source, ex: cloud, tape ...
 */
class RawDataWriter {
public:
    virtual bool Write(uint64_t offset, uint8_t* buffer, int length, ErrCodeType& errorCode) = 0;

    virtual bool Ok() = 0;

    virtual bool Flush() = 0;

    virtual void ReopenFile() = 0;

    virtual ErrCodeType Error() = 0;

    virtual bool LockVolume() = 0;

    virtual bool UnLockVolume() = 0;

    virtual ~RawDataWriter() = default;
};

/**
 * @brief Param struct to build RawDataReader/RawDataWriter.
 * Used to build reader/writer for each backup restore session to read/write from/to copyfile.
 */
struct SessionCopyRawIOParam {
    CopyFormat          copyFormat;     ///> format of the copy to use in current session
    std::string         copyFilePath;   ///> absolute file path of copy
    std::string         shareName;
    uint64_t            volumeOffset;   ///> volume offset in bytes
    uint64_t            length;         ///> session size in bytes
};

/**
 * @brief Builder function to build a copy file reader from specified param
 * @param param
 * @return a valid `std::shared_ptr<RawDataReader>` ptr if succeed
 * @return `nullptr` if failed
*/
std::shared_ptr<RawDataReader> OpenRawDataCopyReader(const SessionCopyRawIOParam& param);

/**
 * @brief Builder function to build a copy file writer from specified param
 * @param param
 * @return a valid `std::shared_ptr<RawDataWriter>` ptr if succeed
 * @return `nullptr` if failed
*/
std::shared_ptr<RawDataWriter> OpenRawDataCopyWriter(const SessionCopyRawIOParam& param);

/**
 * @brief Builder function to build a volume reader using given volume path
 * @param param
 * @return a valid `std::shared_ptr<RawDataReader>` ptr if succeed
 * @return `nullptr` if failed
*/
std::shared_ptr<RawDataReader> OpenRawDataVolumeReader(const std::string& volumePath, const CopyFormat& copyFormat);

/**
 * @brief Builder function to build a volume writer using given volume path
 * @param param
 * @return a valid `std::shared_ptr<RawDataWriter>` ptr if succeed
 * @return `nullptr` if failed
*/
std::shared_ptr<RawDataWriter> OpenRawDataVolumeWriter(const std::string& volumePath, const CopyFormat& copyFormat);

/**
 * @brief Truncate create a file (maybe sparse file)
 * @param path absolute file path
 * @param size file size in bytes
 * @param errorCode get error code if failed
 * @return true if file creation succeed
 * @return false if file creation failed
 */
bool TruncateCreateFile(const std::string& path, uint64_t size, ErrCodeType& errorCode);

}
};

#endif