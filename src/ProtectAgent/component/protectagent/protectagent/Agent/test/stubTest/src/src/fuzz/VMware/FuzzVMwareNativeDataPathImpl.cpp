#include "fuzz/VMware/FuzzVMwareNativeDataPathImpl.h"
#include "secodeFuzz.h"
#include "Cmd.h"
#include "dataprocess/datapath/VMwareNativeDataPathImpl.h"
#include "apps/vmwarenative/VMwareNativeBackup.h"
#include "common/Types.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "securecom/RootCaller.h"
using namespace std;

mp_int32 StubVMwareNativeDataPathImplGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

#define StubClogToVoidLogNullPointReference()                                                                          \
    do {                                                                                                               \
        m_stub.set(                                                                                                      \
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

mp_int32 VMwareNativeDataPathImpl_ParseStorageType_stub(const Json::Value &msgBody)
{
    return MP_SUCCESS;
}

mp_int32 VMwareNativeDataPathImpl_ParseVmSnapshotRef_stub(const Json::Value &msgBody)
{
    return MP_SUCCESS;
}

namespace {
    mp_char STRINIT[] = "12345";
    const mp_int32 UTILS_NUM_1000 = 1000;
    const mp_uchar UTILS_NUM_255  = 255;
    const mp_uchar UTILS_NUM_254  = 254;
    const mp_uchar UTILS_NUM_233  = 233;
    const mp_uchar UTILS_NUM_4    = 4;
    

    const mp_uint32 UTILS_NUM_0 = 0;
    const mp_uint32 UTILS_NUM_STORPROTOCOL_ISCSI = 1;
    const mp_uint32 UTILS_NUM_STORPROTOCOL_NAS = 2;
    const mp_uint32 UTILS_NUM_VMNAME_MAX_LEN = 128;
    const mp_uint32 UTILS_NUM_ERRDETAIL_MAX_LEN = 1024;
    const mp_uint32 UTILS_NUM_PORTS_MIN = 1;
    const mp_uint32 UTILS_NUM_PORTS_MAX = 65535;
    const mp_uint32 UTILS_NUM_TASKID_MAX_LEN = 60;
    const mp_uint32 UTILS_NUM_FOTMAT_STRING_MAX_LEN = 512;
    const mp_uint32 UTILS_NUM_UUID_MAX_LEN = 36;
    void initBodyParams(Json::Value &bmgBody)
    {
        Json::Value jsonReq;
        jsonReq["Port"] = DT_SetGetNumberRange(&g_Element[0],443,1,UTILS_NUM_PORTS_MAX);
        jsonReq["Password"] = DT_SetGetString(&g_Element[1],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        jsonReq["IP"] = DT_SetGetString(&g_Element[2], 6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        jsonReq["Protocol"] = 0;
        jsonReq["UserName"] = DT_SetGetString(&g_Element[3],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        jsonReq["Version"] = DT_SetGetString(&g_Element[4],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        Json::Value vmInfo;
        vmInfo["vmID"] = DT_SetGetString(&g_Element[5],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        vmInfo["vmName"] = DT_SetGetString(&g_Element[6],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        vmInfo["vmRef"] = DT_SetGetString(&g_Element[7],6,UTILS_NUM_VMNAME_MAX_LEN,STRINIT);
        bmgBody["vmInfo"] = vmInfo;
        bmgBody["ProductManager"] = jsonReq;
        bmgBody["parentTaskId"] = DT_SetGetString(&g_Element[8],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        bmgBody["taskId"] = DT_SetGetString(&g_Element[9],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        bmgBody["cmd"] = DT_SetGetString(&g_Element[10],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
    }
}

//ParsePreparationRequesetParms  used in  vddkinit and vddkcleanup
TEST_F(FuzzVMwareNativeDataPathIml, ParsePreparationRequsetParams)
{
    StubClogToVoidLogNullPointReference();
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    VMwareNativeDataPathImpl impl;
    DT_Enable_Leak_Check(0,0);
    DT_FUZZ_START1("ParsePreparationRequsetParams")
    {
        Json::Value bmgBody;
        initBodyParams(bmgBody);
        Json::Value reqBodyParams;
        reqBodyParams[MANAGECMD_KEY_BODY] =bmgBody;
        vmware_vm_info vmInfoInner;
        vmware_pm_info pmInfoInner;
        mp_uint64 snapType = 2;
        EXPECT_EQ(MP_SUCCESS, impl.ParsePreparationRequsetParams(reqBodyParams,snapType,vmInfoInner,pmInfoInner));
    }
    DT_FUZZ_END()
}

//VddKInit  VMwareDiskLib::GetInstance()->BuildConnectParams
TEST_F(FuzzVMwareNativeDataPathIml, VddKInit)
{
    StubClogToVoidLogNullPointReference();
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    VMwareNativeDataPathImpl impl;
    DT_Enable_Leak_Check(0,0);
    DT_FUZZ_START1("VddKInit")
    {
        Json::Value bmgBody;
        initBodyParams(bmgBody);
        Json::Value reqBodyParams;
        reqBodyParams[MANAGECMD_KEY_BODY] =bmgBody;
        EXPECT_EQ(MP_FAILED, impl.VMwareNativeVddkInit(reqBodyParams, false));
    }
    DT_FUZZ_END()
}

// VMwareNativeVddkCleanup
TEST_F(FuzzVMwareNativeDataPathIml, VddKCleanUp)
{
    StubClogToVoidLogNullPointReference();
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    VMwareNativeDataPathImpl impl;
    // vm level vddk init
    DT_Enable_Leak_Check(0,0);
    DT_FUZZ_START1("VddKCleanup")
    {
        Json::Value bmgBody;
        initBodyParams(bmgBody);
        Json::Value reqBodyParams;
        reqBodyParams[MANAGECMD_KEY_BODY] =bmgBody;
        EXPECT_EQ(MP_FAILED, impl.VMwareNativeVddkCleanup(reqBodyParams));
    }
    DT_FUZZ_END()
}

// VMwareNativePreparation
TEST_F(FuzzVMwareNativeDataPathIml, Preparation)
{
    StubClogToVoidLogNullPointReference();
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(VMwareNativeDataPathImpl, ParseStorageType), VMwareNativeDataPathImpl_ParseStorageType_stub);
    m_stub.set(ADDR(VMwareNativeDataPathImpl, ParseVmSnapshotRef), VMwareNativeDataPathImpl_ParseVmSnapshotRef_stub);
    VMwareNativeDataPathImpl impl;
    DT_FUZZ_START1("Preparation")
    {
        Json::Value bmgBody;
        initBodyParams(bmgBody);
        Json::Value reqBodyParams;
        reqBodyParams[MANAGECMD_KEY_BODY] =bmgBody;
        EXPECT_EQ(MP_SUCCESS, impl.VMwareNativePreparation(reqBodyParams));;
    }
    DT_FUZZ_END()
}

//TargetLunPrepare check the ParseStorageLunmounted 
TEST_F(FuzzVMwareNativeDataPathIml, TargetLunPrepare)
{
    StubClogToVoidLogNullPointReference();
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    VMwareNativeDataPathImpl impl;
    DT_FUZZ_START1("TargetLunPrepare")
    {
        Json::Value bmgBody;
        initBodyParams(bmgBody);
        Json::Value reqBodyParams;
        reqBodyParams[MANAGECMD_KEY_BODY] =bmgBody;
        EXPECT_EQ(MP_FAILED, impl.TargetLunPrepare(reqBodyParams));
    }
    DT_FUZZ_END()
}

// VMwareNativeDataBlockBackup check the PrepareProcess
TEST_F(FuzzVMwareNativeDataPathIml, DataBlockBackup)
{
    StubClogToVoidLogNullPointReference();
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    VMwareNativeDataPathImpl impl;
    // vm level vddk init
    DT_FUZZ_START1("DataBlockBackup")
    {
        Json::Value bmgBody;
        initBodyParams(bmgBody);
        bmgBody["dbName"] = DT_SetGetString(&g_Element[0],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        Json::Value reqBodyParams;
        reqBodyParams[MANAGECMD_KEY_BODY] =bmgBody;
        mp_string strErr = DT_SetGetString(&g_Element[1],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        EXPECT_EQ(MP_FAILED, impl.PrepareProcess(reqBodyParams,strErr));
    }
    DT_FUZZ_END()
}

//BackupOpenDisk check ParseVolumeParams
TEST_F(FuzzVMwareNativeDataPathIml, BackupOpenDisk)
{
    StubClogToVoidLogNullPointReference();
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    VMwareNativeDataPathImpl impl;
    // vm level vddk init
    DT_FUZZ_START1("BackupOpenDisk")
    {
        Json::Value bmgBody;
        bmgBody[PARAM_KEY_TASKID] = DT_SetGetString(&g_Element[0],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        bmgBody[PARAM_KEY_PARENT_TASKID] = DT_SetGetString(&g_Element[10],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        bmgBody[PARAM_KEY_VOLUME_MEDIUMID]=DT_SetGetString(&g_Element[11],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        bmgBody[PARAM_KEY_VOLUME_DISKID]=DT_SetGetString(&g_Element[1],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        bmgBody[PARAM_KEY_VOLUME_DISKPATH]=DT_SetGetString(&g_Element[2],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        bmgBody[PARAM_KEY_VOLUME_DISKSIZE]=DT_SetGetString(&g_Element[3],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        bmgBody[PARAM_KEY_VOLUME_BACKUPED_DISKID]=DT_SetGetString(&g_Element[4],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        bmgBody[PARAM_KEY_HOSTAGENT_SYSTEM_VIRT]=DT_SetGetString(&g_Element[5],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        bmgBody[PARAM_KEY_VOLUME_SUPPORT_SAN_TRANSPORTMODE]=DT_SetGetString(&g_Element[6],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        bmgBody[PARAM_KEY_DESCFILE_ATTRS_SECTOR]=DT_SetGetString(&g_Element[12],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        bmgBody[PARAM_KEY_VOLUME_BACKUPLEVEL]=DT_SetGetString(&g_Element[13],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        bmgBody[PARAM_KEY_VOLUME_EAGERLY_CRUB]=DT_SetGetString(&g_Element[14],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        Json::Value attr;
        attr[PARAM_KEY_DESCFILE_ATTRS_CYLINDER] = DT_SetGetString(&g_Element[7],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        attr[PARAM_KEY_DESCFILE_ATTRS_HEAD] = DT_SetGetString(&g_Element[8],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        attr[PARAM_KEY_DESCFILE_ATTRS_SECTOR] = DT_SetGetString(&g_Element[9],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        bmgBody[PARAM_KEY_DESCFILE_ATTRS] = attr;
        Json::Value reqBodyParams;
        reqBodyParams[MANAGECMD_KEY_BODY] =bmgBody;
        EXPECT_EQ(MP_FAILED, impl.VMwareNativeBackupOpenDisk(reqBodyParams));
    }
    DT_FUZZ_END()
}

// DataBlockRestore check PrepareProcess
TEST_F(FuzzVMwareNativeDataPathIml, DataBlockRestore)
{
    StubClogToVoidLogNullPointReference();
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    VMwareNativeDataPathImpl impl;
    // vm level vddk init
    DT_FUZZ_START1("DataBlockRestore")
    {
        Json::Value bmgBody;
        initBodyParams(bmgBody);
        bmgBody["dbName"] = DT_SetGetString(&g_Element[0],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        Json::Value reqBodyParams;
        reqBodyParams[MANAGECMD_KEY_BODY] =bmgBody;
        mp_string strErr = DT_SetGetString(&g_Element[1],6,UTILS_NUM_FOTMAT_STRING_MAX_LEN,STRINIT);
        EXPECT_EQ(MP_FAILED, impl.PrepareProcess(reqBodyParams,strErr));
    }
    DT_FUZZ_END()
}