#include "apps/dws/XBSAServer/BsaSessionManagerTest.h"
#include "common/ConfigXmlParse.h"
#include "common/Path.h"
#include "apps/dws/XBSAServer/BsaTransManager.h"
#include "apps/dws/XBSAServer/BsaDb.h"
#include "apps/appprotect/plugininterface/ApplicationProtectBaseDataType_types.h"
#include "apps/dws/XBSAServer/BsaIntfAdaptor.h"

namespace {
mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)

mp_int32 StubFailed()
{
    return MP_FAILED;
}

mp_int32 StubSuccess()
{
    return MP_SUCCESS;
}

mp_bool StubTrue()
{
    return MP_TRUE;
}

mp_bool StubFalse()
{
    return MP_FALSE;
}

mp_string StubGetMountPathEmpty()
{
    return "";
}

mp_string StubGetMountPath()
{
    return "test";
}

mp_void StubVoid()
{
    return;
}

mp_int32 StubQueryDwsHosts(const BsaQueryPageInfo &pageInfo, std::vector<DwsHostInfo> &hostList)
{
    DwsHostInfo info;
    info.fsId = "fsId";
    info.fsName = "fsName";
    info.fsDeviceId = "fsDeviceId";
    info.hostname = "hostname";
    hostList.push_back(info);
    return MP_SUCCESS;
}

mp_int32 StubParseFsRelation(DwsFsRelation &relation)
{
    FsRelation fsR;
    fsR.oldFsId = "";
    fsR.oldFsName = "";
    fsR.oldEsn = "";
    fsR.newFsId = "";
    fsR.newFsName = "";
    fsR.newEsn = "";
    relation.relations.push_back(fsR);
    return MP_FALSE;
}

BsaSession *StubGetSessionNull(mp_long bsaHandle)
{
    return nullptr;
}

BsaTransaction *GetTransTestSub(mp_long sessionId)
{
    mp_long session;
    mp_long transId;
    BsaTransaction *newBsaTransaction = new BsaTransaction(session, transId);
    return newBsaTransaction;
}

static mp_int32 StubGetValueString2(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    static mp_int32 iCounter = 0;

    if (iCounter++ > 0)
    {
        return MP_SUCCESS;
    }
    else
    {
        return MP_FAILED;
    }
}

static mp_string Stub_GetMountPath_Succ(const mp_string &deviceSN, const mp_string &fsName)
{
    return "/tmp";
}
}

TEST_F(BsaSessionManagerTest, NewSessionTest)
{
    DoGetJsonStringTest();
    BSAInitResult rsp;
    BsaObjectOwner objectOwner;
    mp_string env;
    BsaSessionManager::GetInstance().NewSession(rsp, objectOwner, env, BSA_AppType::BSA_DWS);

    stub.set(&BsaSessionManager::CheckNewSessionParam, StubSuccess);
    BsaSessionManager::GetInstance().NewSession(rsp, objectOwner, env, BSA_AppType::BSA_DWS);

    BsaSessionManager::GetInstance().NewSession(rsp, objectOwner, env, BSA_AppType::BSA_DWS);

    stub.set(&BsaSession::Init, StubFailed);
    BsaSessionManager::GetInstance().NewSession(rsp, objectOwner, env, BSA_AppType::BSA_DWS);

    stub.set(&BsaSession::Init, StubSuccess);
    BsaSessionManager::GetInstance().NewSession(rsp, objectOwner, env, BSA_AppType::BSA_DWS);
}

TEST_F(BsaSessionManagerTest, CloseSessionTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    BsaSessionManager::GetInstance().CloseSession(sessionId);
}

TEST_F(BsaSessionManagerTest, BeginTxnTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    BsaSessionManager::GetInstance().BeginTxn(sessionId);
}

TEST_F(BsaSessionManagerTest, EndTxnTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_int32 vote;
    BsaSessionManager::GetInstance().EndTxn(sessionId, vote);

    stub.set(&BsaIntfAdaptor::VoteValid, StubTrue);
    stub.set(&BsaSessionManager::GetSession, StubGetSessionNull);
    BsaSessionManager::GetInstance().EndTxn(sessionId, vote);
}

TEST_F(BsaSessionManagerTest, CreateObjectTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    BsaObjectDescriptor objDesc;
    CreateObjectResult rsp;
    BsaSessionManager::GetInstance().CreateObject(sessionId, objDesc, rsp);
    stub.set(ADDR(BsaSessionManager, GetTrans), GetTransTestSub);
    BsaSessionManager::GetInstance().CreateObject(sessionId, objDesc, rsp);

    stub.set(ADDR(BsaSessionManager, CheckCreateObjParam), StubSuccess);
    stub.set(ADDR(BsaIntfAdaptor, ConvertCreateReqObj), StubFailed);
    BsaSessionManager::GetInstance().CreateObject(sessionId, objDesc, rsp);

    stub.set(ADDR(BsaIntfAdaptor, ConvertCreateReqObj), StubSuccess);
    stub.set(ADDR(BsaTransaction, CreateObj), StubSuccess);
    BsaSessionManager::GetInstance().CreateObject(sessionId, objDesc, rsp);

    stub.set(ADDR(BsaIntfAdaptor, ConvertCreateRspObj), StubVoid);
    BsaSessionManager::GetInstance().CreateObject(sessionId, objDesc, rsp);

    stub.reset(ADDR(BsaSessionManager, GetTrans));
    stub.reset(ADDR(BsaSessionManager, CheckCreateObjParam));
    stub.reset(ADDR(BsaIntfAdaptor, ConvertCreateReqObj));
    stub.reset(ADDR(BsaIntfAdaptor, ConvertCreateRspObj));
}

TEST_F(BsaSessionManagerTest, QueryObjectTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    BsaQueryDescriptor objDesc;
    QueryObjectResult rsp;
    BsaSessionManager::GetInstance().QueryObject(sessionId, objDesc, rsp);
    stub.set(ADDR(BsaSessionManager, GetTrans), GetTransTestSub);
    BsaSessionManager::GetInstance().QueryObject(sessionId, objDesc, rsp);

    stub.set(ADDR(BsaSessionManager, CheckQueryObjParam), StubSuccess);
    stub.set(ADDR(BsaIntfAdaptor, ConvertQueryReqObj), StubSuccess);
    stub.set(ADDR(BsaTransaction, QueryObj), StubFailed);
    BsaSessionManager::GetInstance().QueryObject(sessionId, objDesc, rsp);
}

TEST_F(BsaSessionManagerTest, GetObjectTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    BsaObjectDescriptor objDesc;
    GetObjectResult rsp;
    BsaSessionManager::GetInstance().GetObject(sessionId, objDesc, rsp);
}

BsaTransaction* BsaTransactionptr(){
    mp_long bsaHandle = 655350;
    mp_long transId = 5643212;
    BsaTransaction* res = new BsaTransaction(bsaHandle, transId);
    return res;
}

TEST_F(BsaSessionManagerTest, GetNextObjTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    GetNextQueryObjectResult rsp;
    BsaSessionManager::GetInstance().GetNextObj(sessionId, rsp);
    stub.set(&BsaSessionManager::GetTrans, BsaTransactionptr);
    BsaSessionManager::GetInstance().GetNextObj(sessionId, rsp);
}

TEST_F(BsaSessionManagerTest, DeleteObjectTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    BsaUInt64 copyId;
    BsaSessionManager::GetInstance().DeleteObject(sessionId, copyId);
    stub.set(&BsaSessionManager::GetTrans, BsaTransactionptr);
    BsaSessionManager::GetInstance().DeleteObject(sessionId, copyId);
}

TEST_F(BsaSessionManagerTest, SendDataTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    BsaDataBlock32 dataBlock;
    BsaSessionManager::GetInstance().SendData(sessionId, dataBlock);
    stub.set(&BsaSessionManager::GetTrans, BsaTransactionptr);
    BsaSessionManager::GetInstance().SendData(sessionId, dataBlock);
}

TEST_F(BsaSessionManagerTest, GetDataTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    BsaDataBlock32 dataBlock;
    BsaSessionManager::GetInstance().GetData(sessionId, dataBlock);
    stub.set(&BsaSessionManager::GetTrans, BsaTransactionptr);
    BsaSessionManager::GetInstance().GetData(sessionId, dataBlock);
}

TEST_F(BsaSessionManagerTest, EndDataTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    BsaDataBlock32 dataBlock;
    BsaUInt64 estimatedSize;
    int64_t size;
    BsaSessionManager::GetInstance().EndData(sessionId, estimatedSize, size);
    stub.set(&BsaSessionManager::GetTrans, BsaTransactionptr);
    BsaSessionManager::GetInstance().EndData(sessionId, estimatedSize, size);
}

TEST_F(BsaSessionManagerTest, GetLastErrTest)
{
    DoGetJsonStringTest();
    BsaSessionManager::GetInstance().GetLastErr();
}

TEST_F(BsaSessionManagerTest, GetProviderTest)
{
    DoGetJsonStringTest();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubGetValueString2);
    BsaSessionManager::GetInstance().GetProvider();
}


TEST_F(BsaSessionManagerTest, SetLastErrTest)
{
    DoGetJsonStringTest();
    mp_int32 errNo;
    BsaSessionManager::GetInstance().SetLastErr(errNo);
}

TEST_F(BsaSessionManagerTest, NewTransIdTest)
{
    DoGetJsonStringTest();
    BsaSessionManager::GetInstance().NewTransId();
}

TEST_F(BsaSessionManagerTest, GenFullStorePathTest)
{
    DoGetJsonStringTest();
    mp_string mountPath;
    mp_string storePath;
    BsaSessionManager::GetInstance().GenFullStorePath(mountPath, storePath);
}

TEST_F(BsaSessionManagerTest, GetSessionTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    BsaSessionManager::GetInstance().GetSession(sessionId);
}

TEST_F(BsaSessionManagerTest, GetTransTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    BsaSessionManager::GetInstance().GetTrans(sessionId);
}

TEST_F(BsaSessionManagerTest, CheckNewSessionParamTest)
{
    DoGetJsonStringTest();
    BsaObjectOwner objectOwner;
    mp_string env[BSA_ENV_BUTT];
    mp_int32 envSize;
    BsaSessionManager::GetInstance().CheckNewSessionParam(objectOwner, env, envSize);

    stub.set(BsaIntfAdaptor::BsaObjectOwnerValid, StubTrue);
    BsaSessionManager::GetInstance().CheckNewSessionParam(objectOwner, env, envSize);
    stub.reset(BsaIntfAdaptor::BsaObjectOwnerValid);
}

TEST_F(BsaSessionManagerTest, CheckCreateObjParamTest)
{
    DoGetJsonStringTest();
    BsaObjectDescriptor objDesc;
    BsaSessionManager::GetInstance().CheckCreateObjParam(objDesc);

    stub.set(BsaIntfAdaptor::CopyTypeValid, StubTrue);
    stub.set(BsaIntfAdaptor::ObjectTypeValid, StubTrue);
    stub.set(BsaIntfAdaptor::BsaObjectOwnerValid, StubTrue);
    stub.set(BsaIntfAdaptor::AppObjectOwnerValid, StubFalse);
    BsaSessionManager::GetInstance().CheckCreateObjParam(objDesc);

    stub.set(BsaIntfAdaptor::AppObjectOwnerValid, StubTrue);
    stub.set(BsaIntfAdaptor::ObjectSpaceNameValid, StubTrue);
    stub.set(BsaIntfAdaptor::PathNameValid, StubFalse);
    BsaSessionManager::GetInstance().CheckCreateObjParam(objDesc);

    stub.set(BsaIntfAdaptor::PathNameValid, StubTrue);
    stub.set(BsaIntfAdaptor::ResourceTypeValid, StubTrue);
    stub.set(BsaIntfAdaptor::ObjectDescriptionValid, StubTrue);
    stub.set(BsaIntfAdaptor::ObjectInfoValid, StubFalse);
    BsaSessionManager::GetInstance().CheckCreateObjParam(objDesc);

    stub.set(BsaIntfAdaptor::ObjectInfoValid, StubTrue);
    EXPECT_EQ(BsaSessionManager::GetInstance().CheckCreateObjParam(objDesc), BSA_RC_SUCCESS);
}

TEST_F(BsaSessionManagerTest, CheckQueryObjParamTest)
{
    DoGetJsonStringTest();
    BsaQueryDescriptor queryDesc;
    BsaSessionManager::GetInstance().CheckQueryObjParam(queryDesc);

    stub.set(BsaIntfAdaptor::CopyTypeValid, StubTrue);
    stub.set(BsaIntfAdaptor::ObjectTypeValid, StubTrue);
    stub.set(BsaIntfAdaptor::ObjectStatusValid, StubTrue);
    stub.set(BsaIntfAdaptor::BsaObjectOwnerValid, StubTrue);
    stub.set(BsaIntfAdaptor::AppObjectOwnerValid, StubFalse);
    BsaSessionManager::GetInstance().CheckQueryObjParam(queryDesc);

    stub.set(BsaIntfAdaptor::AppObjectOwnerValid, StubTrue);
    stub.set(BsaIntfAdaptor::ObjectSpaceNameValid, StubTrue);
    stub.set(BsaIntfAdaptor::PathNameValid, StubFalse);
    BsaSessionManager::GetInstance().CheckQueryObjParam(queryDesc);

    stub.set(BsaIntfAdaptor::PathNameValid, StubTrue);
    BsaSessionManager::GetInstance().CheckQueryObjParam(queryDesc);
}

TEST_F(BsaSessionManagerTest, ParseEnvTest)
{
    DoGetJsonStringTest();
    mp_string in;
    mp_string out[BSA_ENV_BUTT];
    mp_int32 outSize;
    BsaSessionManager::GetInstance().ParseEnv(in, out, outSize);

    in = "BSA_API_VERSION=test;test=test";
    BsaSessionManager::GetInstance().ParseEnv(in, out, outSize);
}

TEST_F(BsaSessionManagerTest, InitIdTest)
{
    DoGetJsonStringTest();
    mp_long defaultVal = 20;
    mp_long maxVal = 100;
    BsaSessionManager::GetInstance().InitId(defaultVal, maxVal);
}

TEST_F(BsaSessionManagerTest, NewIdTest)
{
    DoGetJsonStringTest();
    mp_long last;
    BsaSessionManager::GetInstance().NewId(last);
}

TEST_F(BsaSessionManagerTest, NewSessionIdTest)
{
    DoGetJsonStringTest();
    BsaSessionManager::GetInstance().NewSessionId();
}