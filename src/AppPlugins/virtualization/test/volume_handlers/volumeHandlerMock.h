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
#ifndef __VOLUME_HANDLER_MOCK_H__
#define __VOLUME_HANDLER_MOCK_H__

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "common/Structs.h"
#include <volume_handlers/VolumeHandler.h>
#include "volume_handlers/common/DiskDeviceFile.h"

namespace HDT_TEST {
class VolumeHandlerMock : public VirtPlugin::VolumeHandler {
public:
    VolumeHandlerMock(std::shared_ptr<VirtPlugin::JobHandle> jobHandle, const VirtPlugin::VolInfo &volInfo) : VolumeHandler(jobHandle, volInfo, "", "") {}
    virtual ~VolumeHandlerMock() {}
    MOCK_METHOD(int32_t, GetDirtyRanges, (const VirtPlugin::VolSnapInfo &preVolSnapshot, const VirtPlugin::VolSnapInfo &curVolSnapshot,
        VirtPlugin::DirtyRanges &dirtyRanges, const uint64_t startOffset, uint64_t &endOffset), (override));
    MOCK_METHOD(std::shared_ptr<VirtPlugin::DiskDeviceFile>, GetDiskDeviceFile, (), (override));
    MOCK_METHOD(int32_t, Open, (const VirtPlugin::VolOpenMode &mode), (override));
    MOCK_METHOD(int32_t, Open, (const VirtPlugin::VolOpenMode &mode, const VirtPlugin::BackupSubJobInfo &jobInfo), (override));
    MOCK_METHOD(int32_t, Close, (), (override));
    MOCK_METHOD(int32_t, ReadBlocks, (const uint64_t &offsetInBytes, uint64_t &bufSizeInBytes, std::shared_ptr<uint8_t []> &buf, std::shared_ptr<uint8_t []> &calBuf, std::shared_ptr<uint8_t []> &readBuf), (override));
    MOCK_METHOD(int32_t, WriteBlocks, (const uint64_t &offsetInBytes, uint64_t &bufSizeInBytes, std::shared_ptr<uint8_t []> &buf), (override));
    MOCK_METHOD(uint64_t, GetVolumeSize, (), (override));
    MOCK_METHOD(int32_t, TestDeviceConnection, (const std::string &authExtendInfo, int32_t &erroCode), (override));
    MOCK_METHOD(int32_t, CleanLeftovers, (), (override));
    MOCK_METHOD(int32_t, Flush, (), (override));
};
}

#endif //__VOLUME_HANDLER_MOCK_H__
