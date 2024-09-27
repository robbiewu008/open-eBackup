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
#ifndef _JOBMANAGER_TEST_H_
#define _JOBMANAGER_TEST_H_

#define protected public
#include <vector>
#include <algorithm>
#include "stub.h"
#include "gtest/gtest.h"
#include "cunitpub/publicInc.h"
#include "apps/appprotect/plugininterface/ProtectService.h"
using namespace AppProtect;

class StubProtectServiceClient : virtual public AppProtect::ProtectServiceClient {
public:
    StubProtectServiceClient(std::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
        ProtectServiceClient(prot) {}

    void AsyncAbortJob(ActionResult& _return, const std::string& jobId, const std::string& subJobId, const std::string& appType) override
    {
        _return.code = MP_SUCCESS;
    }
    
    void CheckBackupJobType(ActionResult& _return, const BackupJob& job) override
    {
        _return.code = MP_SUCCESS;
    }
    
    void AllowBackupInLocalNode(ActionResult& _return, const BackupJob& job, const BackupLimit::type limit) override
    {
        _return.code = MP_SUCCESS;
    }
    
    void AllowBackupSubJobInLocalNode(ActionResult& _return, const BackupJob& job, const SubJob& subjob) override
    {
        _return.code = MP_SUCCESS;
    }

    void AllowRestoreInLocalNode(ActionResult& _return, const RestoreJob& job) override
    {
        _return.code = MP_SUCCESS;
    }
    
    void AllowRestoreSubJobInLocalNode(ActionResult& _return, const RestoreJob& job, const SubJob& subJob) override
    {
        _return.code = MP_SUCCESS;
    }

    void AsyncBackupPrerequisite(ActionResult& _return, const BackupJob& job) override
    {
        _return.code = MP_SUCCESS;
    }
    
    void AsyncBackupGenerateSubJob(ActionResult& _return, const BackupJob& job, const int32_t nodeNum) override
    {
        _return.code = MP_SUCCESS;
    }
    
    void AsyncExecuteBackupSubJob(ActionResult& _return, const BackupJob& job, const SubJob& subJob) override
    {
        _return.code = MP_SUCCESS;
    }
    
    void AsyncBackupPostJob(ActionResult& _return, const BackupJob& job, const SubJob& subJob, const JobResult::type backupJobResult) override
    {
        _return.code = MP_SUCCESS;
    }
    
    void AsyncRestorePrerequisite(ActionResult& _return, const RestoreJob& job) override
    {
        _return.code = MP_SUCCESS;
    }
    
    void AsyncRestoreGenerateSubJob(ActionResult& _return, const RestoreJob& job, const int32_t nodeNum) override
    {
        _return.code = MP_SUCCESS;
    }
    
    void AsyncExecuteRestoreSubJob(ActionResult& _return, const RestoreJob& job, const SubJob& subJob) override
    {
        _return.code = MP_SUCCESS;
    }
    
    void AsyncRestorePostJob(ActionResult& _return, const RestoreJob& job, const SubJob& subJob, const JobResult::type restoreJobResult) override
    {
        _return.code = MP_SUCCESS;
    }
    
    void AsyncLivemountGenerateSubJob(ActionResult& _return, const LivemountJob& job, const int32_t nodeNum) override
    {
        _return.code = MP_SUCCESS;
    }
    
    void AsyncExecuteLivemountSubJob(ActionResult& _return, const LivemountJob& job, const SubJob& subJob) override
    {
        _return.code = MP_SUCCESS;
    }

    void AsyncCancelLivemountGenerateSubJob(ActionResult& _return, const CancelLivemountJob& job, const int32_t  nodeNum) override
    {
        _return.code = MP_SUCCESS;
    }

   void AsyncExecuteCancelLivemountSubJob(ActionResult& _return, const CancelLivemountJob& job, const SubJob& subJob )
   {
        _return.code = MP_SUCCESS;
    }
    
    void AsyncInstantRestorePrerequisite(ActionResult& _return, const RestoreJob& job) override
    {
        _return.code = MP_SUCCESS;
    }
    
    void AsyncInstantRestoreGenerateSubJob(ActionResult& _return, const RestoreJob& job, const int32_t nodeNum) override
    {
        _return.code = MP_SUCCESS;
    }
    
    void AsyncExecuteInstantRestoreSubJob(ActionResult& _return, const RestoreJob& job, const SubJob& subJob) override
    {
        _return.code = MP_SUCCESS;
    }
    
    void AsyncInstantRestorePostJob(ActionResult& _return, const RestoreJob& job, const SubJob& subJob, const JobResult::type restoreJobResult) override
    {
        _return.code = MP_SUCCESS;
    }
    
    void AsyncBuildIndexGenerateSubJob(ActionResult& _return, const BuildIndexJob& job, const int32_t nodeNum) override
    {
        _return.code = MP_SUCCESS;
    }

    void AsyncBuildIndexSubJob(ActionResult& _return, const BuildIndexJob& job, const SubJob& subJob) override
    {
        _return.code = MP_SUCCESS;
    }

    void AsyncDelCopyGenerateSubJob(ActionResult& _return, const DelCopyJob& job, const int32_t nodeNum) override
    {
        _return.code = MP_SUCCESS;
    }

    void AsyncDelCopySubJob(ActionResult& _return, const DelCopyJob& job, const SubJob& subJob) override
    {
        _return.code = MP_SUCCESS;
    }

    void AsyncCheckCopyGenerateSubJob(ActionResult& _return, const CheckCopyJob& job, const int32_t nodeNum) override
    {
        _return.code = MP_SUCCESS;
    }

    void AsyncCheckCopySubJob(ActionResult& _return, const CheckCopyJob& job, const SubJob& subJob) override
    {
        _return.code = MP_SUCCESS;
    }

    void PauseJob(ActionResult& _return, const std::string& jobId, const std::string& subJobId, const std::string& appType) override
    {
        _return.code = MP_SUCCESS;
    }
};

class ExternalJobTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void ExternalJobTest::SetUp() {}

void ExternalJobTest::TearDown() {}

void ExternalJobTest::SetUpTestCase() {}

void ExternalJobTest::TearDownTestCase() {}

#endif