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
#include <iostream>
#include <vector>
#include <string>
#include <system/System.hpp>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "common/Macros.h"
#include "common/Structs.h"
#include "common/DirtyRanges.h"
#include "volume_handlers/fusionstorage/FusionStorageVolumeHandler.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;

using namespace VirtPlugin;

namespace HDT_TEST {

class VolumeHandlerDescriptorTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    std::shared_ptr<FusionStorageApi> m_spDSWareApi;
    std::shared_ptr<FusionStorageBitmapHandle> m_spBitmapHandle;
    std::shared_ptr<DiskDeviceFile> m_spDeviceFile;
    std::shared_ptr<DiskDataPersistence> m_dataPersistence;
    std::shared_ptr<JobHandle> jobHandle;
};

void VolumeHandlerDescriptorTest::SetUp()
{
    std::string m_fusionStorMgrIp = "127.0.0.1";
    std::string m_poolID = "123";
    bool m_isBackup = true;

    VolInfo m_volInfo;
    m_volInfo.m_datastore.m_ip = "127.0.0.1";
    m_volInfo.m_volSizeInBytes = 8 * 1024 * 1024;  // 卷大小
    m_volInfo.m_datastore.m_poolId = "12";

    m_dataPersistence = std::make_shared<DiskDataPersistence>();
    m_spDSWareApi = std::make_shared<FusionStorageApi>(m_fusionStorMgrIp, m_poolID);
    m_spBitmapHandle = std::make_shared<FusionStorageBitmapHandle>(
        m_fusionStorMgrIp, m_poolID, m_isBackup, m_volInfo.m_uuid, m_dataPersistence);
    m_spDeviceFile = std::make_shared<DiskDeviceFile>();
}

void VolumeHandlerDescriptorTest::TearDown()
{}

void VolumeHandlerDescriptorTest::SetUpTestCase()
{}

void VolumeHandlerDescriptorTest::TearDownTestCase()
{}

static int32_t ExecStubSuccess()
{
    return SUCCESS;
}

static int32_t ExecStubFailed()
{
    return FAILED;
}
}  // namespace HDT_TEST