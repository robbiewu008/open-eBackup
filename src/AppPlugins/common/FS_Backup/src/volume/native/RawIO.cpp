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
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>
#include "VolumeProtector.h"
#include "log/Log.h"

#ifdef _WIN32
#include "win32/Win32RawIO.h"
#endif

#ifdef  __linux__
#include "linux/PosixRawIO.h"
#endif

#include "native/RawIO.h"

using namespace volumeprotect;
using namespace volumeprotect::rawio;

#ifdef _WIN32
using OsPlatformRawDataReader = rawio::win32::Win32RawDataReader;
using OsPlatformRawDataWriter = rawio::win32::Win32RawDataWriter;
#endif

#ifdef  __linux__
using OsPlatformRawDataReader = rawio::posix::PosixRawDataReader;
using OsPlatformRawDataWriter = rawio::posix::PosixRawDataWriter;
#endif

namespace {
    constexpr auto DUMMY_SESSION_INDEX = 999;
}

std::shared_ptr<rawio::RawDataReader> rawio::OpenRawDataCopyReader(const SessionCopyRawIOParam& param)
{
    CopyFormat copyFormat = param.copyFormat;
    std::string copyFilePath = param.copyFilePath;

    switch (copyFormat) {
        case CopyFormat::BIN : {
            return std::make_shared<OsPlatformRawDataReader>(copyFilePath, -1, param.volumeOffset);
        }
        case CopyFormat::IMAGE : {
            return std::make_shared<OsPlatformRawDataReader>(copyFilePath, 0, 0);
        }
#ifdef _WIN32
        case CopyFormat::FILE :
            return std::make_shared<OsPlatformRawDataReader>(copyFilePath, 0, 0, true);
        case CopyFormat::VHD_FIXED :
        case CopyFormat::VHD_DYNAMIC :
        case CopyFormat::VHDX_FIXED :
        case CopyFormat::VHDX_DYNAMIC : {
            // need virtual disk be attached and inited ahead, this should be guaranteed by TaskResourceManager
            return std::make_shared<rawio::win32::Win32VirtualDiskVolumeRawDataReader>(copyFilePath, false, param.shareName);
            break;
        }
#endif
        default: ERRLOG("open unsupport copy format %d for read", static_cast<int>(copyFormat));
    }
    return nullptr;
}

std::shared_ptr<RawDataWriter> rawio::OpenRawDataCopyWriter(const SessionCopyRawIOParam& param)
{
    CopyFormat copyFormat = param.copyFormat;
    std::string copyFilePath = param.copyFilePath;

    switch (copyFormat) {
        case CopyFormat::BIN : {
            return std::make_shared<OsPlatformRawDataWriter>(copyFilePath, -1, param.volumeOffset);
        }
        case CopyFormat::IMAGE : {
            return std::make_shared<OsPlatformRawDataWriter>(copyFilePath, 0, 0);
        }
#ifdef _WIN32
        case CopyFormat::FILE :
            return std::make_shared<OsPlatformRawDataWriter>(copyFilePath, 0, 0, true);
        case CopyFormat::VHD_FIXED :
        case CopyFormat::VHD_DYNAMIC :
        case CopyFormat::VHDX_FIXED :
        case CopyFormat::VHDX_DYNAMIC : {
            // need virtual disk be attached and inited ahead, this should be guaranteed by TaskResourceManager
            return std::make_shared<rawio::win32::Win32VirtualDiskVolumeRawDataWriter>(copyFilePath, false, param.shareName);
        }
#endif
        default: ERRLOG("open unsupport copy format %d for write", static_cast<int>(copyFormat));
    }
    return nullptr;
}

std::shared_ptr<RawDataReader> rawio::OpenRawDataVolumeReader(const std::string& volumePath,
    const CopyFormat& copyFormat)
{
#ifdef WIN32
    if (copyFormat == CopyFormat::FILE) {
        return std::make_shared<OsPlatformRawDataReader>(volumePath, 0, 0, true);
    }
#endif
    return std::make_shared<OsPlatformRawDataReader>(volumePath, 0, 0);
}

std::shared_ptr<RawDataWriter> rawio::OpenRawDataVolumeWriter(const std::string& volumePath,
    const CopyFormat& copyFormat)
{
#ifdef WIN32
    if (copyFormat == CopyFormat::FILE) {
        return std::make_shared<OsPlatformRawDataWriter>(volumePath, 0, 0, true);
    }
#endif
    return std::make_shared<OsPlatformRawDataWriter>(volumePath, 0, 0);
}