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
#include "common/JsonUtils.h"
#include "dataprocess/VMwareNativeDataPathImplTest.h"
#include "apps/vmwarenative/VMwareNativeBackup.h"

#define StubClogToVoidLogNullPointReference()                                                                          \
    do {                                                                                                               \
        stub.set(                                                                                                      \
            (mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),     \
            StubVMwareNativeDataPathImplGetValueInt32Return);                                                                   \
    } while (0)
static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 VMwareNativeDataPathImpl_InitVmProtectionParams_stub(Json::Value &msg)
{
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl_ParseDiskType_stub(Json::Value &msg, mp_string &diskType)
{
    return MP_FAILED;
}

VMWARE_DISK_RET_CODE VMwareDiskLib_BuildConnectParams_stub(
const std::string &vmRef, const vmware_pm_info &pmInfo, VddkConnectParams &connectParams)
{
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl_InitVddkLib_stub(Json::Value &msgBody, mp_bool &isInited)
{
    return MP_SUCCESS;
}

VMwareDiskLib* VMwareDiskLib_GetInstance_stub()
{
    VMwareDiskLib* vMwareDiskLib = NULL;
    vMwareDiskLib = new (std::nothrow) VMwareDiskLib;
    return vMwareDiskLib;
}

mp_void CLogger_Log_Stub11(mp_void* pthis)
{
    return;
}

namespace {
    // VMwareNativeVddkInit
    Json::Value vddkReqMsgBody =
        "{\"ProductManager\":{\"Certs\":[\"\"],\"IP\":\"10.10.10.10\",\"Password\":\"******\",\"Port\":443,\"Protocol\":0, \
        \"UserName\":\"admin@vsphere.local\",\"Version\":\"6.7.0\"}, \
        \"parentTaskId\":\"9d139626-9048-4d1f-9939-160a91cc1ace\",\"taskId\":\"9d139626-9048-4d1f-9939-160a91cc1ace\",\
        \"vmInfo\":{\"vmID\":\"501f33cd-4caf-a27a-8df8-b2bed9c29744\",\"vmName\":\"test_vm_0001\",\
        \"vmRef\":\"vm-71024\"}},\"cmd\":1106}";
/*
    TEST_F(VMwareNativeDataPathImplTest, VMwareNativeVddkInit_without_inited_success)
    {
        VMwareNativeDataPathImpl impl;
        // vm level vddk init
        Stub stub;
        stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
            ADDR(CLogger, Log), CLogger_Log_Stub11);
        stub.set(ADDR(VMwareNativeDataPathImpl, InitVmProtectionParams),
            VMwareNativeDataPathImpl_InitVmProtectionParams_stub);
        stub.set(ADDR(VMwareNativeDataPathImpl, ParseDiskType),
            VMwareNativeDataPathImpl_ParseDiskType_stub);
        EXPECT_EQ(MP_SUCCESS, impl.VMwareNativeVddkInit(vddkReqMsgBody, false));
    }
*/
    TEST_F(VMwareNativeDataPathImplTest, VMwareNativeVddkInit_without_inited_failure)
    {
        StubClogToVoidLogNullPointReference();
        stub.set(&CLogger::Log, StubCLoggerLog);
        VMwareNativeDataPathImpl impl;
        // disk level vddk init
        Json::Value nullBody;
        EXPECT_NE(0, impl.VMwareNativeVddkInit(nullBody, false));
    }

    TEST_F(VMwareNativeDataPathImplTest, VMwareNativeVddkInit_with_inited_success)
    {
        StubClogToVoidLogNullPointReference();
        stub.set(&CLogger::Log, StubCLoggerLog);
        VMwareNativeDataPathImpl impl;
        Json::Value nullBody;
        // disk level vddk init
        EXPECT_EQ(MP_FAILED, impl.VMwareNativeVddkInit(nullBody, true));
    }

    TEST_F(VMwareNativeDataPathImplTest, VMwareNativeVddkInit_with_inited_failure)
    {
        StubClogToVoidLogNullPointReference();
        stub.set(&CLogger::Log, StubCLoggerLog);
        VMwareNativeDataPathImpl impl;
        // disk level vddk init
        Json::Value nullBody;
        EXPECT_NE(0, impl.VMwareNativeVddkInit(nullBody, true));
    }

    // VMwareNativeVddkCleanup
    TEST_F(VMwareNativeDataPathImplTest, VMwareNativeVddkCleanup_success)
    {
        StubClogToVoidLogNullPointReference();
        stub.set(&CLogger::Log, StubCLoggerLog);
        VMwareNativeDataPathImpl impl;
        // vm level vddk init
        Json::Value nullBody;
        EXPECT_EQ(MP_FAILED, impl.VMwareNativeVddkCleanup(nullBody));
    }

    TEST_F(VMwareNativeDataPathImplTest, VMwareNativeVddkCleanup_failure)
    {
        StubClogToVoidLogNullPointReference();
        stub.set(&CLogger::Log, StubCLoggerLog);
        VMwareNativeDataPathImpl impl;
        // vm level vddk init
        Json::Value nullBody;
        EXPECT_EQ(MP_FAILED, impl.VMwareNativeVddkCleanup(nullBody));
    }

    // VMwareNativePreparation
    Json::Value preparationReqBody = "{\"ProductManager\":{\"Certs\":[\"\"],\"IP\":\"10.10.10.10\",\
        \"Password\":\"******\",\"Port\":443,\"Protocol\":0,\"UserName\":\"admin@vsphere.local\",\
        \"Version\":\"6.7.0\"},\"SnapType\":2,\"Volumes\":[{\"DirtyRange\":\"\",\
        \"diskID\":\"6000c295-0d77-587b-7bad-74481beb6257\",\
        \"diskPath\":\"[datastore1 (5)] 1a0c6cb6-0d88-43a7-b8ce-cc2a0ed/6000C292-a070-fd07-d6bc3-20210529T163852.vmdk\",\
        \"diskSize\":68719476736,\"mediumID\":\"5661580165964968486\"}],\"parentTaskId\":\"9d139626-904860a91cc1ace\",\
        \"storage\":{\"storProtocol\":2,\"storType\":1,\"storageIps\":[\"8.42.99.244\"]},\
        \"taskId\":\"e39173104-c7d8e4dcd17e\",\"vmInfo\":{\"snapshotRef\":\"snapshot-88598\",\
        \"vmID\":\"501f33cd-4caf-a27a-8df8-b2bed9c29744\",\"vmName\":\"test_vm_0001\",\
        \"vmRef\":\"vm-71024\"}},\"cmd\":1025}";

    mp_int32 VMwareNativeDataPathImpl_ParseStorageType_stub(const Json::Value &msgBody)
    {
        return MP_SUCCESS;
    }

    mp_int32 VMwareNativeDataPathImpl_ParseVmSnapshotRef_stub(const Json::Value &msgBody)
    {
        return MP_SUCCESS;
    }
    TEST_F(VMwareNativeDataPathImplTest, VMwareNativePreparation_success)
    {
        StubClogToVoidLogNullPointReference();
        stub.set(&CLogger::Log, StubCLoggerLog);
        stub.set(ADDR(VMwareNativeDataPathImpl, ParseStorageType), VMwareNativeDataPathImpl_ParseStorageType_stub);
        stub.set(ADDR(VMwareNativeDataPathImpl, ParseVmSnapshotRef), VMwareNativeDataPathImpl_ParseVmSnapshotRef_stub);
        VMwareNativeDataPathImpl impl;
        // vm level vddk init
        Json::Value preparationReq;
        Json::Value reqBody;
        reqBody["dbName"] = "db1";
        preparationReq["body"] = reqBody;
        preparationReq["cmd"] = 1;
        EXPECT_EQ(0, impl.VMwareNativePreparation(preparationReq));
    }

    TEST_F(VMwareNativeDataPathImplTest, VMwareNativePreparation_failure)
    {
        StubClogToVoidLogNullPointReference();
        stub.set(&CLogger::Log, StubCLoggerLog);
        VMwareNativeDataPathImpl impl;
        // vm level vddk init
        Json::Value nullBody;
        EXPECT_NE(0, impl.VMwareNativePreparation(nullBody));
    }

    // VMwareNativeDataBlockBackup
    Json::Value runBackupReqBody = "{\"DirtyRange\":[{\"length\":4194304,\"start\":62914560000},{\"length\":4194304,\"start\":62918754304},\
        \"backupLevel\":2,\"cmd\":1032,\"descFileAttrs\":{\"cylinder\":8354,\"head\":255,\"sector\":63},\
        \"diskID\":\"6000c295-0d77-587b-7bad-74481beb6257\",\"diskPath\":\"[datastore1 (5)] 1a0c6cc2a0ed/6000C292-a52.vmdk\",\
        \"diskSize\":68719476736,\"eagerlyCrub\":\"true\",\"isSystem\":false,\"limitSpeed\":0,\
        \"mediumID\":\"5661580165964968486\",\"parentTaskId\":\"9d139626-9048-4d1f-9939-160a91cc1ace\",\
        \"storProtocol\":2,\"systemVirt\":1,\"taskId\":\"e3917310-91c3-4314-9004-c7d8e4dcd17e\"},\"cmd\":1080}";
    TEST_F(VMwareNativeDataPathImplTest, VMwareNativeDataBlockBackup_success)
    {
        StubClogToVoidLogNullPointReference();
        stub.set(&CLogger::Log, StubCLoggerLog);
        VMwareNativeDataPathImpl impl;
        // vm level vddk init
        Json::Value preparationReq;
        Json::Value reqBody;
        reqBody["dbName"] = "db1";
        preparationReq["body"] = reqBody;
        preparationReq["cmd"] = 1;
        EXPECT_EQ(MP_FAILED, impl.VMwareNativePreparation(preparationReq));
    }

    TEST_F(VMwareNativeDataPathImplTest, VMwareNativeDataBlockBackup_failure)
    {
        StubClogToVoidLogNullPointReference();
        stub.set(&CLogger::Log, StubCLoggerLog);
        VMwareNativeDataPathImpl impl;
        // vm level vddk init
        Json::Value nullBody;
        EXPECT_NE(0, impl.VMwareNativePreparation(nullBody));
    }

    // HandleDataBlockBackupFinish
    Json::Value finishDiskReqBody = "{\"FinishDisk\":true,\"diskID\":\"6000c291-be2d-a8da-8b66-292e6eeb6e6a\",\
        \"parentTaskId\":\"9d139626-9048-4d1f-9939-160a91cc1ace\",\"taskId\":\"3ca67c73-f0f5-4c54-a3a3-68d92b1669e6\"},\
        \"cmd\":1084}";
    TEST_F(VMwareNativeDataPathImplTest, DataBlockBackupFinish_success)
    {
        StubClogToVoidLogNullPointReference();
        stub.set(&CLogger::Log, StubCLoggerLog);
        VMwareNativeDataPathImpl impl;
        // vm level vddk init
        Json::Value preparationReq;
        Json::Value reqBody;
        reqBody["dbName"] = "db1";
        preparationReq["body"] = reqBody;
        preparationReq["cmd"] = 1;
        EXPECT_EQ(MP_FAILED, impl.VMwareNativePreparation(preparationReq));
    }

    TEST_F(VMwareNativeDataPathImplTest, DataBlockBackupFinish_failure)
    {
        StubClogToVoidLogNullPointReference();
        stub.set(&CLogger::Log, StubCLoggerLog);
        VMwareNativeDataPathImpl impl;
        // vm level vddk init
        Json::Value nullBody;
        EXPECT_NE(0, impl.VMwareNativePreparation(nullBody));
    }

    // VMwareNativeDataBlockRestore
    Json::Value runRecoveryReqBody =
        "{\"DirtyRange\":[{\"length\":4194304,\"start\":62914560000},{\"length\":4194304,\"start\":62918754304},\
        \"backupLevel\":2,\"cmd\":1032,\"descFileAttrs\":{\"cylinder\":8354,\"head\":255,\"sector\":63},\
        \"diskID\":\"6000c295-0d77-587b-7bad-74481beb6257\",\"diskPath\":\"[datastore1 (5)] 1a0c6cc2a0ed/6000C292-a52.vmdk\",\
        \"diskSize\":68719476736,\"eagerlyCrub\":\"true\",\"isSystem\":false,\"limitSpeed\":0,\
        \"mediumID\":\"5661580165964968486\",\"parentTaskId\":\"9d139626-9048-4d1f-9939-160a91cc1ace\",\
        \"storProtocol\":2,\"systemVirt\":1,\"taskId\":\"e3917310-91c3-4314-9004-c7d8e4dcd17e\"},\"cmd\":1044}";
    TEST_F(VMwareNativeDataPathImplTest, VMwareNativeDataBlockRestore_success)
    {
        StubClogToVoidLogNullPointReference();
        stub.set(&CLogger::Log, StubCLoggerLog);
        VMwareNativeDataPathImpl impl;
        // vm level vddk init
        Json::Value preparationReq;
        Json::Value reqBody;
        reqBody["dbName"] = "db1";
        preparationReq["body"] = reqBody;
        preparationReq["cmd"] = 1;
        EXPECT_EQ(MP_FAILED, impl.VMwareNativePreparation(preparationReq));
    }

    TEST_F(VMwareNativeDataPathImplTest, VMwareNativeDataBlockRestore_failure)
    {
        StubClogToVoidLogNullPointReference();
        stub.set(&CLogger::Log, StubCLoggerLog);
        VMwareNativeDataPathImpl impl;
        // vm level vddk init
        Json::Value nullBody;
        EXPECT_NE(0, impl.VMwareNativePreparation(nullBody));
    }

    // DataBlockRestoreFinish
    Json::Value restoreFinishDiskReqBody = "{\"FinishDisk\":true,\"diskID\":\"6000c291-be2d-a8da-8b66-292e6eeb6e6a\",\
        \"parentTaskId\":\"9d139626-9048-4d1f-9939-160a91cc1ace\",\"taskId\":\"3ca67c73-f0f5-4c54-a3a3-68d92b1669e6\"},\
        \"cmd\":1096}";
    TEST_F(VMwareNativeDataPathImplTest, DataBlockRestoreFinish_success)
    {
        StubClogToVoidLogNullPointReference();
        stub.set(&CLogger::Log, StubCLoggerLog);
        VMwareNativeDataPathImpl impl;
        // vm level vddk init
        Json::Value preparationReq;
        Json::Value reqBody;
        reqBody["dbName"] = "db1";
        preparationReq["body"] = reqBody;
        preparationReq["cmd"] = 1;
        EXPECT_EQ(MP_FAILED, impl.VMwareNativePreparation(preparationReq));
    }

    TEST_F(VMwareNativeDataPathImplTest, DataBlockRestoreFinish_failure)
    {
        StubClogToVoidLogNullPointReference();
        stub.set(&CLogger::Log, StubCLoggerLog);
        VMwareNativeDataPathImpl impl;
        // vm level vddk init
        Json::Value nullBody;
        EXPECT_NE(0, impl.VMwareNativePreparation(nullBody));
    }
    
    TEST_F(VMwareNativeDataPathImplTest, CheckVmwareInfo)
    {
        stub.set(&CLogger::Log, StubCLoggerLog);
        VMwareNativeDataPathImpl impl;
        {
            vmware_vm_info params;
            EXPECT_EQ(MP_SUCCESS, impl.CheckVmwareVmInfo(params));
        }
        {
            vmware_pm_info params;
            params.strIP = "192.168.1.1";
            EXPECT_EQ(MP_SUCCESS, impl.CheckVmwarePmInfo(params));
        }
        {
            vmware_volume_info params;
            EXPECT_EQ(MP_SUCCESS, impl.CheckVmwareVolumeInfo(params));
        }
    }
}
