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
#include <string>
#include <cstdlib>
#include <cstdio>
#include "secodeFuzz.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "PluginMain.h"
#include "common/utils/Utils.h"
#include "common/Constants.h"
#include "common/model/ResponseModel.h"
#include "protect_engines/hcs/common/HcsHttpStatus.h"
#include "common/httpclient/HttpClient.h"
#include "common/model/ModelBase.h"
#include "protect_engines/hcs/utils/HCSTokenMgr.h"
#include "protect_engines/hcs/HCSProtectEngine.h"
#include "protect_engines/kubernetes/KubernetesProtectEngine.h"
#include "repository_handlers/mock/FileSystemHandlerMock.h"
#include "repository_handlers/factory/RepositoryFactory.h"
#include "repository_handlers/filesystem/FileSystemHandler.h"

using ::testing::_;
using testing::AllOf;
using ::testing::AnyNumber;
using ::testing::AtLeast;
using testing::ByMove;
using testing::DoAll;
using ::testing::Eq;
using ::testing::Field;
using ::testing::Ge;
using ::testing::Gt;
using testing::InitGoogleMock;
using ::testing::Invoke;
using ::testing::Ne;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::SetArgumentPointee;
using ::testing::Throw;
using HcsPlugin::HCSProtectEngine;
using KubernetesPlugin::KubernetesProtectEngine;
using namespace AppProtect;
using namespace VirtPlugin;
using namespace Module;

using FuncCheckBackTypeHCS = void (*)(HCSProtectEngine*, ActionResult&, const BackupJob&);
using FuncCheckBackTypeKuber = void (*)(KubernetesProtectEngine*, ActionResult&, const BackupJob&);

using FuncPathExists = bool (*)(FileSystemHandler*, const std::string&);

namespace {
const int MAX_UUID_LENGTH = 36;
const int MAX_STRING_LENGTH = 100;
const int VECTOR_SIZE_EMPTY = 0;
const int MAX_PATH_LENGTH = 260;
const std::string HCS_TYPE_STRING = "HCSContainer";
const std::string KUBER_TYPE_STRING = "Kubernetes";
}

namespace DUZZ_TEST {
class Plugin_Main_dt_fuzz : public testing::Test {
public:
    void SetUp() {}
    void TearDown() {}
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
public:
    Stub stub;
};


static int32_t StubCheckBackupJobTypeDefault(ActionResult&, const AppProtect::BackupJob&)
{
    return VirtPlugin::SUCCESS;
}


static bool StubExistsDefault(const std::string &fileName)
{
    return true;
}

// interface AppInit
TEST_F(Plugin_Main_dt_fuzz, AppInitDefault)
{
    DT_Enable_Leak_Check(0, 0);
    DT_FUZZ_START(0, 100000, "AppInitDefault", 0)
    {
        std::string strintPath = DT_SetGetString(&g_Element[0], 6, MAX_PATH_LENGTH, "/home");
        AppInit(strintPath);
    }
    DT_FUZZ_END()
}

int StubLoadFileToStruct(std::shared_ptr<RepositoryHandler> repoHandler, const std::string &file, SnapshotInfo& t)
{
    return VirtPlugin::SUCCESS;
}

// interface CheckBackupJobType
TEST_F(Plugin_Main_dt_fuzz, Check_Backup_JobType_HCS_dt_fuzz)
{
    DT_Enable_Leak_Check(0, 0);
    ActionResult retValue;
    BackupJob job;

    FuncCheckBackTypeHCS funcPtr = (FuncCheckBackTypeHCS)(&HCSProtectEngine::CheckBackupJobType);
    FuncPathExists funcExist = (FuncPathExists)(&FileSystemHandler::Exists);
    stub.set(funcExist, StubExistsDefault);
    stub.set(funcPtr, StubCheckBackupJobTypeDefault);
    stub.set((int(*)(std::shared_ptr<RepositoryHandler>, const std::string&, SnapshotInfo&))(Utils::LoadFileToStruct),
        StubLoadFileToStruct);
    DT_FUZZ_START(0, 100000, "Check_Backup_JobType_HCS_dt_fuzz", 0)
    {
        uint32_t backType = *(uint32_t*)DT_SetGetU32(&g_Element[0], 0);
        job.jobParam.backupType = static_cast<::BackupJobType::type>(backType);
        job.protectEnv.subType = HCS_TYPE_STRING;

        StorageRepository storage;
        storage.repositoryType = RepositoryDataType::META_REPOSITORY;

        uint32_t protocol = *(uint32_t*)DT_SetGetU32(&g_Element[1], 0);
        storage.protocol = static_cast<::RepositoryProtocolType::type>(protocol);
        job.repositories.push_back(storage);

        std::string path = DT_SetGetString(&g_Element[2], 10, MAX_PATH_LENGTH, "/home/etc");
        storage.path.push_back(path);
        CheckBackupJobType(retValue, job);
    }
    DT_FUZZ_END()
    stub.reset(funcExist);
    stub.reset(funcPtr);
    stub.reset(
        (int(*)(std::shared_ptr<RepositoryHandler>, const std::string&, SnapshotInfo&))(Utils::LoadFileToStruct));
}

// interface CheckBackupJobType
TEST_F(Plugin_Main_dt_fuzz, Check_Backup_JobType_Kubernets_dt_fuzz)
{
    DT_Enable_Leak_Check(0, 0);
    ActionResult retValue;
    BackupJob job;

    FuncCheckBackTypeKuber funcPtr = (FuncCheckBackTypeKuber)(&KubernetesProtectEngine::CheckBackupJobType);
    FuncPathExists funcExist = (FuncPathExists)(&FileSystemHandler::Exists);
    stub.set(funcExist, StubExistsDefault);
    stub.set(funcPtr, StubCheckBackupJobTypeDefault);
    stub.set((int(*)(std::shared_ptr<RepositoryHandler>, const std::string&, SnapshotInfo&))(Utils::LoadFileToStruct),
        StubLoadFileToStruct);
    DT_FUZZ_START(0, 100000, "Check_Backup_JobType_Kubernets_dt_fuzz", 0)
    {
        uint32_t backType = *(uint32_t*)DT_SetGetU32(&g_Element[0], 1);
        job.jobParam.backupType = static_cast<::BackupJobType::type>(backType);
        job.protectEnv.subType = KUBER_TYPE_STRING;

        StorageRepository storage;
        storage.repositoryType = RepositoryDataType::META_REPOSITORY;

        uint32_t protocol = *(uint32_t*)DT_SetGetU32(&g_Element[1], 0);
        storage.protocol = static_cast<::RepositoryProtocolType::type>(protocol);
        job.repositories.push_back(storage);

        std::string path = DT_SetGetString(&g_Element[2], 10, MAX_PATH_LENGTH, "/home/etc");
        storage.path.push_back(path);
        CheckBackupJobType(retValue, job);
    }
    DT_FUZZ_END()
    stub.reset(funcExist);
    stub.reset(funcPtr);
    stub.reset(
        (int(*)(std::shared_ptr<RepositoryHandler>, const std::string&, SnapshotInfo&))(Utils::LoadFileToStruct));
}
}
