#define private public
#include "xbsaclientcomm/ThriftClientMgr.h"
#include "securecom/SecureUtils.h"
#include "securecom/CryptAlg.h"
#include "common/Path.h"
#include "common/ConfigXmlParse.h"
#include "common/StackTracer.h"
#include "common/JsonHelper.h"
#include "common/JsonUtils.h"
#include "xbsaclientcomm/DataConversion.h"
#include "gtest/gtest.h"
#include "stub.h"
#include "common/Log.h"
#include <vector>


using namespace apache::thrift;
using namespace apache::thrift::async;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace std;

class ThriftClientTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

};

void ThriftClientTest::SetUp() {}
void ThriftClientTest::TearDown() {}
void ThriftClientTest::SetUpTestCase() {}
void ThriftClientTest::TearDownTestCase() {}

mp_void CLogger_Log_Stub(mp_void* pthis)
{
    return;
}

mp_int32 CConfigXmlParser_Load_Stub()
{
    return MP_SUCCESS;
}

mp_int32 CConfigXmlParser_Load_Stub_Failed()
{
    return MP_FAILED;
}

mp_int32 CConfigXmlParser_Init_Stub(mp_string strCfgFilePath)
{
    return MP_SUCCESS;
}

void ThriftClientMgr_Stub()
{}

mp_int32 CConfigXmlParser_GetValueBool_Stub(const mp_string& strSection, const mp_string& strKey, mp_bool& bValue)
{
    return MP_SUCCESS;
}

mp_int32 CConfigXmlParser_GetValueString_Stub(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_SUCCESS;
}

int32_t ThriftClientMgr_InitModule_Stub()
{
    return MP_SUCCESS;
}

int32_t ThriftClientMgr_Init_Stub(bool ssl)
{
    return MP_SUCCESS;
}

mp_int32 InitCrypt_Stub(const mp_uint32 roleType)
{
    return MP_SUCCESS;
}

mp_void GetHostFromCert_Stub(mp_void* pThis, const mp_string& certPath, mp_string &hostName)
{

}

void TFramedTransport_open_Stub()
{}

void TSSLSocketFactory_ciphers_Stub(const std::string& enable)
{}

void TSSLSocketFactory_loadCertificate_Stub(const char* path, const char* format = "PEM")
{}

void TSSLSocketFactory_overrideDefaultPasswordCallback_Stub()
{}

void TSSLSocketFactory_loadPrivateKey_Stub(const char* path, const char* format = "PEM")
{}

void TSSLSocketFactory_loadTrustedCertificates_Stub(const char* path, const char* capath = nullptr)
{}

void TSSLSocketFactory_authenticate_Stub(bool required)
{}

mp_string StubGetLogPath()
{
    return "";
}

mp_string StubGetConfFilePath()
{
    return "";
}

mp_int32 StubFailed()
{
    return MP_FAILED;
}

mp_int32 StubSuccess()
{
    return MP_SUCCESS;
}

mp_bool StubFalse()
{
    return MP_FALSE;
}

mp_bool StubTrue()
{
    return MP_TRUE;
}

mp_int32 StubReturnInt60()
{
    return 60;
}

mp_int32 StubReturnInt0()
{
    return 0;
}

mp_int32 StubReturnInt31()
{
    return 31;
}

mp_int32 StubGetValueString(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    strValue = "";
    return MP_SUCCESS;
}

mp_string StubCreateSockGetConfFilePath(mp_string strFileName)
{
    return "";
}

mp_void StubGetHostFromCert(mp_void* pThis, const mp_string& certPath, mp_string &hostName)
{
    hostName = "test";
}

std::shared_ptr<apache::thrift::transport::TSocket> StubCreateSockNull(bool ssl)
{
    return nullptr;
}

std::shared_ptr<apache::thrift::transport::TSocket> StubCreateSockNotNull(bool ssl)
{
    std::shared_ptr<TSocket> socket;
    return socket;
}

mp_int32 StubReadFileSucc(const mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    vecOutput.push_back("xxx");
    return MP_SUCCESS;
}

TEST_F(ThriftClientTest, InitOk) {
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(InitCrypt, InitCrypt_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString),
        CConfigXmlParser_GetValueString_Stub);
    typedef void (*fptr1)(TSSLSocketFactory*, const mp_string&);
    fptr1 fptr_ciphers = (fptr1)(&TSSLSocketFactory::ciphers);
    stub.set(fptr_ciphers, TSSLSocketFactory_ciphers_Stub);
    typedef void (*fptr2)(TSSLSocketFactory*, const char*, const char*);
    fptr2 fptr_loadCertificate = (fptr2)(&TSSLSocketFactory::loadCertificate);
    stub.set(fptr_loadCertificate, TSSLSocketFactory_loadCertificate_Stub);
    typedef void (*fptr3)(TSSLSocketFactory*);
    fptr3 fptr_overrideDefaultPasswordCallback = (fptr3)(&TSSLSocketFactory::overrideDefaultPasswordCallback);
    stub.set(fptr_overrideDefaultPasswordCallback, TSSLSocketFactory_overrideDefaultPasswordCallback_Stub);
    typedef void (*fptr4)(TSSLSocketFactory*, const char*, const char*);
    fptr4 fptr_loadPrivateKey = (fptr4)(&TSSLSocketFactory::loadPrivateKey);
    stub.set(fptr_loadPrivateKey, TSSLSocketFactory_loadPrivateKey_Stub);
    typedef void (*fptr5)(TSSLSocketFactory*,const char*, const char*);
    fptr5 fptr_loadTrustedCertificates = (fptr5)(&TSSLSocketFactory::loadTrustedCertificates);
    stub.set(fptr_loadTrustedCertificates, TSSLSocketFactory_loadTrustedCertificates_Stub);
    typedef void (*fptr6)(TSSLSocketFactory*, bool);
    fptr6 fptr_authenticate = (fptr6)(&TSSLSocketFactory::authenticate);
    stub.set(fptr_authenticate, TSSLSocketFactory_authenticate_Stub);
    stub.set(SecureCom::GetHostFromCert, GetHostFromCert_Stub);
    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);
    EXPECT_EQ(MP_SUCCESS, ThriftClientMgr::GetInstance().Init(true));
    ThriftClientMgr::GetInstance().SetThriftRetryInterval(5);
    ThriftClientMgr::GetInstance().SetThriftRetryTimes(1);
}

TEST_F(ThriftClientTest, InitFailed) {
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(InitCrypt, InitCrypt_Stub);
    stub.set(SecureCom::GetHostFromCert, GetHostFromCert_Stub);
    typedef void (*fptr)(TFramedTransport*);
    fptr TFramedTransport_open = (fptr)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);
    EXPECT_EQ(MP_SUCCESS, ThriftClientMgr::GetInstance().Init(false));
}

void BSAServiceClient_BSABeginTxn_Stub(void *obj, CallResult &_return, const int64_t handle)
{
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceClient_BSABeginTxn_Failed_Stub(void *obj, CallResult &_return, const int64_t handle)
{
    _return.response = BSA_RC_ACCESS_FAILURE;
}

void BSAServiceClient_BSABeginTxn_throw1_Stub(CallResult &_return, const int64_t handle)
{
    throw TTransportException(TTransportException::NOT_OPEN, "test");
}

void BSAServiceClient_BSABeginTxn_throw2_Stub(CallResult &_return, const int64_t handle)
{
    throw std::runtime_error("test");
}

TEST_F(ThriftClientTest, Test_CreateSock)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);

    EXPECT_NE(ThriftClientMgr::GetInstance().CreateSock(false), nullptr);
    EXPECT_NE(ThriftClientMgr::GetInstance().CreateSock(true), nullptr);
}

std::shared_ptr<apache::thrift::transport::TSocket> Stub_CreateSock()
{
    const std::string DEFAULT_IP = "127.0.0.1";
    const int DEFAULT_HOSTPORT = 59560;
    return std::make_shared<TSocket>(DEFAULT_IP, DEFAULT_HOSTPORT);
}

TEST_F(ThriftClientTest, BSABeginTxnMgr) {
    long bsaHandle = 1;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);

    typedef int32_t (*fptr)(BSAServiceClient*,const int64_t);
    fptr BSAServiceClient_BSABeginTxn = (fptr)(&BSAServiceClient::BSABeginTxn);
    stub.set(BSAServiceClient_BSABeginTxn, BSAServiceClient_BSABeginTxn_Stub);
    EXPECT_EQ(BSA_RC_SUCCESS, ThriftClientMgr::GetInstance().BSABeginTxnMgr(bsaHandle));
    stub.set(BSAServiceClient_BSABeginTxn, BSAServiceClient_BSABeginTxn_Failed_Stub);
    EXPECT_EQ(BSA_RC_ACCESS_FAILURE, ThriftClientMgr::GetInstance().BSABeginTxnMgr(bsaHandle));
    stub.set(BSAServiceClient_BSABeginTxn, BSAServiceClient_BSABeginTxn_throw1_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSABeginTxnMgr(bsaHandle));
    stub.set(BSAServiceClient_BSABeginTxn, BSAServiceClient_BSABeginTxn_throw2_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSABeginTxnMgr(bsaHandle));
}

void BSAServiceClient_BSACreateObject_Stub(void *obj, CreateObjectResult& _return,
    const int64_t handle, const BsaObjectDescriptor& objectDescriptor)
{
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceClient_BSACreateObject_throw1_Stub(CreateObjectResult& _return,
    const int64_t handle, const BsaObjectDescriptor& objectDescriptor)
{
    throw TTransportException(TTransportException::NOT_OPEN, "test");
}

void BSAServiceClient_BSACreateObject_throw2_Stub(CreateObjectResult& _return,
    const int64_t handle, const BsaObjectDescriptor& objectDescriptor)
{
    throw std::runtime_error("test");
}

TEST_F(ThriftClientTest, BSACreateObjectMgr) {
    long bsaHandle = 1;
    BSA_ObjectDescriptor objectDescriptorPtr;
    BSA_DataBlock32 dataBlockPtr;
    objectDescriptorPtr.copyType = BSA_CopyType_BACKUP;
    objectDescriptorPtr.objectType = BSA_ObjectType_FILE;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);

    typedef void (*fptr)(BSAServiceClient*,CreateObjectResult&, const int64_t, const BsaObjectDescriptor&);
    fptr BSAServiceClient_BSABeginTxn = (fptr)(&BSAServiceClient::BSACreateObject);
    stub.set(BSAServiceClient_BSABeginTxn, BSAServiceClient_BSACreateObject_Stub);
    EXPECT_EQ(MP_SUCCESS, 
        ThriftClientMgr::GetInstance().BSACreateObjectMgr(bsaHandle, &objectDescriptorPtr, &dataBlockPtr));
    stub.set(BSAServiceClient_BSABeginTxn, BSAServiceClient_BSACreateObject_throw1_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR,
        ThriftClientMgr::GetInstance().BSACreateObjectMgr(bsaHandle, &objectDescriptorPtr, &dataBlockPtr));
    stub.set(BSAServiceClient_BSABeginTxn, BSAServiceClient_BSACreateObject_throw2_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR,
        ThriftClientMgr::GetInstance().BSACreateObjectMgr(bsaHandle, &objectDescriptorPtr, &dataBlockPtr));
}

void BSAServiceClient_BSADeleteObject_Stub(void *obj, CallResult &_return, const int64_t handle, const BsaUInt64& copyId)
{
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceClient_BSADeleteObject_throw1_Stub(CallResult &_return, const int64_t handle, const BsaUInt64& copyId)
{
    throw TTransportException(TTransportException::NOT_OPEN, "test");
}

void BSAServiceClient_BSADeleteObject_throw2_Stub(CallResult &_return, const int64_t handle, const BsaUInt64& copyId)
{
    throw std::runtime_error("test");
}

TEST_F(ThriftClientTest, BSADeleteObjectMgr) {
    long bsaHandle = 1;
    BSA_UInt64 copyId;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);

    typedef int32_t (*fptr)(BSAServiceClient*, const int64_t , const BsaUInt64& );
    fptr BSAServiceClient_BSADeleteObject = (fptr)(&BSAServiceClient::BSADeleteObject);
    stub.set(BSAServiceClient_BSADeleteObject, BSAServiceClient_BSADeleteObject_Stub);
    EXPECT_EQ(BSA_RC_SUCCESS, ThriftClientMgr::GetInstance().BSADeleteObjectMgr(bsaHandle, copyId));
    stub.set(BSAServiceClient_BSADeleteObject, BSAServiceClient_BSADeleteObject_throw1_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSADeleteObjectMgr(bsaHandle, copyId));
    stub.set(BSAServiceClient_BSADeleteObject, BSAServiceClient_BSADeleteObject_throw2_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSADeleteObjectMgr(bsaHandle, copyId));
}

void BSAServiceClient_BSAEndData_Stub(void *obj, CallResult &_return, const int64_t handle,
    const BsaUInt64 &estimatedSize, const int64_t size)
{
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceClient_BSAEndData_throw1_Stub(CallResult &_return, const int64_t handle,
    const BsaUInt64 &estimatedSize, const int64_t size)
{
    throw TTransportException(TTransportException::NOT_OPEN, "test");
}

void BSAServiceClient_BSAEndData_throw2_Stub(CallResult &_return, const int64_t handle,
    const BsaUInt64 &estimatedSize, const int64_t size)
{
    throw std::runtime_error("test");
}

/*
* 测试用例：GetEstimatedSize接口测试
* 前置条件：无
* CHECK点：1.m_readMap为空时返回失败，2.m_readMap有对应的handle上下文时返回成功
*/
TEST_F(ThriftClientTest, GetEstimatedSize_Test)
{
    long bsaHandle = 1;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);

    BsaUInt64 estimatedSize;
    ThriftClientMgr::GetInstance().m_writeMap.clear();
    EXPECT_EQ(ThriftClientMgr::GetInstance().GetEstimatedSize(bsaHandle, estimatedSize), false);

    BsaHandleContext context;
    context.workingObj.storgePath = "/tmp/";
    ThriftClientMgr::GetInstance().m_writeMap[bsaHandle] = context;
    EXPECT_EQ(ThriftClientMgr::GetInstance().GetEstimatedSize(bsaHandle, estimatedSize), true);
}

TEST_F(ThriftClientTest, GetEstimatedSizeTwo_Test)
{
    long bsaHandle = 1;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CMpFile, FileSize), StubFailed);

    BsaUInt64 estimatedSize;
    BsaHandleContext context;
    context.workingObj.storgePath = "/tmp/";
    ThriftClientMgr::GetInstance().m_writeMap[bsaHandle] = context;
    EXPECT_EQ(ThriftClientMgr::GetInstance().GetEstimatedSize(bsaHandle, estimatedSize), MP_FALSE);
}

TEST_F(ThriftClientTest, BSAEndDataMgr) {
    long bsaHandle = 1;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);

    typedef int32_t (*fptr)(BSAServiceClient*, const int64_t);
    fptr BSAServiceClient_BSAEndData = (fptr)(&BSAServiceClient::BSAEndData);
    stub.set(BSAServiceClient_BSAEndData, BSAServiceClient_BSAEndData_Stub);

    ThriftClientMgr::GetInstance().m_writeMap.clear();
    ThriftClientMgr::GetInstance().m_writer.SetWriteStatus(FileIoStatus::OPEN);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAEndDataMgr(bsaHandle));
    ThriftClientMgr::GetInstance().m_writer.SetWriteStatus(FileIoStatus::CLOSE);
    EXPECT_EQ(BSA_RC_SUCCESS, ThriftClientMgr::GetInstance().BSAEndDataMgr(bsaHandle));
    stub.set(BSAServiceClient_BSAEndData, BSAServiceClient_BSAEndData_throw1_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAEndDataMgr(bsaHandle));
    stub.set(BSAServiceClient_BSAEndData, BSAServiceClient_BSAEndData_throw2_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAEndDataMgr(bsaHandle));

    ThriftClientMgr::GetInstance().m_writer.SetWriteStatus(FileIoStatus::OPEN);
    stub.set(&ThriftClientMgr::GetEstimatedSize, StubTrue);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAEndDataMgr(bsaHandle));
}

void BSAServiceClient_BSAEndTxn_Stub(void *obj, CallResult &_return, const int64_t handle, const int32_t vote)
{
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceClient_BSAEndTxn_throw1_Stub(CallResult &_return, const int64_t handle, const int32_t vote)
{
    throw TTransportException(TTransportException::NOT_OPEN, "test");
}

void BSAServiceClient_BSAEndTxn_throw2_Stub(CallResult &_return, const int64_t handle, const int32_t vote)
{
    throw std::runtime_error("test");
}

TEST_F(ThriftClientTest, BSAEndTxnMgr) {
    long bsaHandle = 1;
    BSA_Vote vote = BSA_Vote::BSA_Vote_ABORT;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);

    typedef int32_t (*fptr)(BSAServiceClient*, const int64_t, const int32_t);
    fptr BSAServiceClient_BSAEndTxn = (fptr)(&BSAServiceClient::BSAEndTxn);
    stub.set(BSAServiceClient_BSAEndTxn, BSAServiceClient_BSAEndTxn_Stub);
    EXPECT_EQ(BSA_RC_SUCCESS, ThriftClientMgr::GetInstance().BSAEndTxnMgr(bsaHandle, vote));
    stub.set(BSAServiceClient_BSAEndTxn, BSAServiceClient_BSAEndTxn_throw1_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAEndTxnMgr(bsaHandle, vote));
    stub.set(BSAServiceClient_BSAEndTxn, BSAServiceClient_BSAEndTxn_throw2_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAEndTxnMgr(bsaHandle, vote));
}

TEST_F(ThriftClientTest, BSAEndTxnMgrTwo) {
    long bsaHandle = 1;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);

    BSA_Vote vote;
    typedef int32_t (*fptr)(BSAServiceClient*, const int64_t, const int32_t);
    fptr BSAServiceClient_BSAEndTxn = (fptr)(&BSAServiceClient::BSAEndTxn);
    stub.set(BSAServiceClient_BSAEndTxn, BSAServiceClient_BSAEndTxn_Stub);
    EXPECT_EQ(BSA_RC_INVALID_VOTE, ThriftClientMgr::GetInstance().BSAEndTxnMgr(bsaHandle, vote));

    vote = BSA_Vote::BSA_Vote_COMMIT;
    EXPECT_EQ(BSA_RC_SUCCESS, ThriftClientMgr::GetInstance().BSAEndTxnMgr(bsaHandle, vote));
}

TEST_F(ThriftClientTest, BSAGetEnvironmentMgr) {
    long bsaHandle;
    BSA_ObjectOwner * objectOwner;
    char ** ptr;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);

    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);

    EXPECT_EQ(BSA_RC_SUCCESS, ThriftClientMgr::GetInstance().BSAGetEnvironmentMgr(bsaHandle, objectOwner, ptr));
}

void BSAServiceClient_BSAGetLastError_Stub(void *obj, GetLastErrorResult& _return, const int64_t bufferSize)
{
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceClient_BSAGetLastError2_Stub(void *obj, GetLastErrorResult& _return, const int64_t bufferSize)
{
    _return.bufferSize = 2;
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceClient_BSAGetLastError_throw1_Stub(GetLastErrorResult& _return, const int64_t bufferSize)
{
    throw TTransportException(TTransportException::NOT_OPEN, "test");
}

void BSAServiceClient_BSAGetLastError_throw2_Stub(GetLastErrorResult& _return, const int64_t bufferSize)
{
    throw std::runtime_error("test");
}

TEST_F(ThriftClientTest, BSAGetLastErrorMgr) {
    BSA_UInt32 sizePtr;
    char *errorCodePtr = new (std::nothrow) char[10];;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);

    typedef int32_t (*fptr)(BSAServiceClient*, GetLastErrorResult& , const int64_t);
    fptr BSAServiceClient_BSAGetLastError = (fptr)(&BSAServiceClient::BSAGetLastError);
    stub.set(BSAServiceClient_BSAGetLastError, BSAServiceClient_BSAGetLastError_Stub);
    EXPECT_EQ(BSA_RC_SUCCESS, ThriftClientMgr::GetInstance().BSAGetLastErrorMgr(&sizePtr, errorCodePtr));
    stub.set(BSAServiceClient_BSAGetLastError, BSAServiceClient_BSAGetLastError_throw1_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAGetLastErrorMgr(&sizePtr, errorCodePtr));
    stub.set(BSAServiceClient_BSAGetLastError, BSAServiceClient_BSAGetLastError_throw2_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAGetLastErrorMgr(&sizePtr, errorCodePtr));
    stub.set(BSAServiceClient_BSAGetLastError, BSAServiceClient_BSAGetLastError2_Stub);
    EXPECT_EQ(BSA_RC_BUFFER_TOO_SMALL, ThriftClientMgr::GetInstance().BSAGetLastErrorMgr(&sizePtr, errorCodePtr));
}

void BSAServiceClient_BSAGetNextQueryObject_Stub(void *obj, GetNextQueryObjectResult& _return, const int64_t handle)
{
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceClient_BSAGetNextQueryObject_throw1_Stub(GetNextQueryObjectResult& _return, const int64_t handle)
{
    throw TTransportException(TTransportException::NOT_OPEN, "test");
}

void BSAServiceClient_BSAGetNextQueryObject_throw2_Stub(GetNextQueryObjectResult& _return, const int64_t handle)
{
    throw std::runtime_error("test");
}

TEST_F(ThriftClientTest, BSAGetNextQueryObjectMgr) {
    long bsaHandle = 1;
    BSA_ObjectDescriptor objectDescriptorPtr;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);

    typedef void (*fptr)(BSAServiceClient*, GetNextQueryObjectResult& , const int64_t);
    fptr BSAServiceClient_BSAGetNextQueryObject = (fptr)(&BSAServiceClient::BSAGetNextQueryObject);
    stub.set(BSAServiceClient_BSAGetNextQueryObject, BSAServiceClient_BSAGetNextQueryObject_Stub);
    EXPECT_EQ(BSA_RC_SUCCESS, ThriftClientMgr::GetInstance().BSAGetNextQueryObjectMgr(bsaHandle, &objectDescriptorPtr));
    stub.set(BSAServiceClient_BSAGetNextQueryObject, BSAServiceClient_BSAGetNextQueryObject_throw1_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAGetNextQueryObjectMgr(bsaHandle, &objectDescriptorPtr));
    stub.set(BSAServiceClient_BSAGetNextQueryObject, BSAServiceClient_BSAGetNextQueryObject_throw2_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAGetNextQueryObjectMgr(bsaHandle, &objectDescriptorPtr));

    stub.set(BSAServiceClient_BSAGetNextQueryObject, BSAServiceClient_BSAGetNextQueryObject_Stub);
    stub.set(&DataConversion::ConvertObjectDescriptorOut, StubFalse);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAGetNextQueryObjectMgr(bsaHandle, &objectDescriptorPtr));

}

void BSAServiceClient_BSAGetObject_Stub(void *obj, GetObjectResult& _return, const int64_t handle,const BsaObjectDescriptor& objectDesc)
{
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceClient_BSAGetObject_throw1_Stub(GetObjectResult& _return, const int64_t handle, const BsaObjectDescriptor& objectDesc)
{
    throw TTransportException(TTransportException::NOT_OPEN, "test");
}

void BSAServiceClient_BSAGetObject_throw2_Stub(GetObjectResult& _return, const int64_t handle, const BsaObjectDescriptor& objectDesc)
{
    throw std::runtime_error("test");
}

TEST_F(ThriftClientTest, BSAGetObjectMgr) {
    long bsaHandle = 1;
    BSA_ObjectDescriptor objectDescriptorPtr;
    BSA_DataBlock32 dataBlockPtr;
    objectDescriptorPtr.copyType = BSA_CopyType_BACKUP;
    objectDescriptorPtr.objectType = BSA_ObjectType_FILE;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);

    typedef void (*fptr)(BSAServiceClient*, GetObjectResult& , const int64_t , const BsaObjectDescriptor&);
    fptr BSAServiceClient_BSAGetObject = (fptr)(&BSAServiceClient::BSAGetObject);

    stub.set(BSAServiceClient_BSAGetObject, BSAServiceClient_BSAGetObject_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAGetObjectMgr(bsaHandle, &objectDescriptorPtr, &dataBlockPtr));
    stub.set(BSAServiceClient_BSAGetObject, BSAServiceClient_BSAGetObject_throw1_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAGetObjectMgr(bsaHandle, &objectDescriptorPtr, &dataBlockPtr));
    stub.set(BSAServiceClient_BSAGetObject, BSAServiceClient_BSAGetObject_throw2_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAGetObjectMgr(bsaHandle, &objectDescriptorPtr, &dataBlockPtr));

    stub.set(BSAServiceClient_BSAGetObject, BSAServiceClient_BSAGetObject_Stub);
    stub.set(&ThriftClientMgr::SetWorkingObj, StubTrue);
    EXPECT_EQ(BSA_RC_SUCCESS, ThriftClientMgr::GetInstance().BSAGetObjectMgr(bsaHandle, &objectDescriptorPtr, &dataBlockPtr));
}

void BSAServiceClient_BSAInit_Stub(void *obj, BSAInitResult& _return, const std::string& tokenPtr,
    const BsaObjectOwner& objectOwner, const std::string& envPtr)
{
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceClient_BSAInit_throw1_Stub(BSAInitResult& _return, const std::string& tokenPtr,
    const BsaObjectOwner& objectOwner, const std::string& envPtr)
{
    throw TTransportException(TTransportException::NOT_OPEN, "test");
}

void BSAServiceClient_BSAInit_throw2_Stub(BSAInitResult& _return, const std::string& tokenPtr,
    const BsaObjectOwner& objectOwner, const std::string& envPtr)
{
    throw std::runtime_error("test");
}

void BSAServiceClient_BSAQueryApiVersion_Stub(void *obj, QueryApiVersionResult& _return)
{
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceClient_BSAQueryApiVersion_throw1_Stub(QueryApiVersionResult& _return)
{
    throw TTransportException(TTransportException::NOT_OPEN, "test");
}

void BSAServiceClient_BSAQueryApiVersion_throw2_Stub(QueryApiVersionResult& _return)
{
    throw std::runtime_error("test");
}

TEST_F(ThriftClientTest, BSAQueryApiVersionMgr) {
    BSA_ApiVersion apiVersionPtr;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);

    typedef void (*fptr)(BSAServiceClient*, QueryApiVersionResult&);
    fptr BSAServiceClient_BSAQueryApiVersion = (fptr)(&BSAServiceClient::BSAQueryApiVersion);
    stub.set(BSAServiceClient_BSAQueryApiVersion, BSAServiceClient_BSAQueryApiVersion_Stub);
    EXPECT_EQ(MP_SUCCESS, ThriftClientMgr::GetInstance().BSAQueryApiVersionMgr(&apiVersionPtr));
    stub.set(BSAServiceClient_BSAQueryApiVersion, BSAServiceClient_BSAQueryApiVersion_throw1_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAQueryApiVersionMgr(&apiVersionPtr));
    stub.set(BSAServiceClient_BSAQueryApiVersion, BSAServiceClient_BSAQueryApiVersion_throw2_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAQueryApiVersionMgr(&apiVersionPtr));
}

void BSAServiceClient_BSAQueryObject_Stub(void *obj, QueryObjectResult& _return, const int64_t handle, const BsaQueryDescriptor& queryDesc)
{
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceClient_BSAQueryObject_throw1_Stub(QueryObjectResult& _return, const int64_t handle, const BsaQueryDescriptor& queryDesc)
{
    throw TTransportException(TTransportException::NOT_OPEN, "test");
}

void BSAServiceClient_BSAQueryObject_throw2_Stub(QueryObjectResult& _return, const int64_t handle, const BsaQueryDescriptor& queryDesc)
{
    throw std::runtime_error("test");
}

TEST_F(ThriftClientTest, BSAQueryObjectMgr) {
    long bsaHandle = 1;
    BSA_QueryDescriptor queryDescriptorPtr;
    BSA_ObjectDescriptor objectDescriptorPtr;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);

    typedef void (*fptr)(BSAServiceClient*, QueryObjectResult& , const int64_t , const BsaQueryDescriptor& );
    fptr BSAServiceClient_BSAQueryObject = (fptr)(&BSAServiceClient::BSAQueryObject);
    stub.set(BSAServiceClient_BSAQueryObject, BSAServiceClient_BSAQueryObject_Stub);
    EXPECT_EQ(MP_SUCCESS, ThriftClientMgr::GetInstance().BSAQueryObjectMgr(bsaHandle, &queryDescriptorPtr, &objectDescriptorPtr));
    stub.set(BSAServiceClient_BSAQueryObject, BSAServiceClient_BSAQueryObject_throw1_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAQueryObjectMgr(bsaHandle, &queryDescriptorPtr, &objectDescriptorPtr));
    stub.set(BSAServiceClient_BSAQueryObject, BSAServiceClient_BSAQueryObject_throw2_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAQueryObjectMgr(bsaHandle, &queryDescriptorPtr, &objectDescriptorPtr));

    stub.set(BSAServiceClient_BSAQueryObject, BSAServiceClient_BSAQueryObject_Stub);
    stub.set(ADDR(DataConversion, ConvertObjectDescriptorOut), StubFailed);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAQueryObjectMgr(bsaHandle, &queryDescriptorPtr, &objectDescriptorPtr));
}

void BSAServiceClient_BSAQueryServiceProvider_Stub(void *obj, QueryServiceProviderResult& _return, const int64_t retSize)
{
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceClient_BSAQueryServiceProvider2_Stub(void *obj, QueryServiceProviderResult& _return, const int64_t retSize)
{
    _return.retSize = 2;
    _return.response = BSA_RC_SUCCESS;
}


void BSAServiceClient_BSAQueryServiceProvider_throw1_Stub(QueryServiceProviderResult& _return, const int64_t retSize)
{
    throw TTransportException(TTransportException::NOT_OPEN, "test");
}

void BSAServiceClient_BSAQueryServiceProvider_throw2_Stub(QueryServiceProviderResult& _return, const int64_t retSize)
{
    throw std::runtime_error("test");
}

TEST_F(ThriftClientTest, BSAQueryServiceProviderMgr) {
    BSA_UInt32 sizePtr = 1;
    char *delimiter = new (std::nothrow) char[10];
    char *providerPtr = new (std::nothrow) char[10];
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);

    typedef void (*fptr)(BSAServiceClient*, QueryServiceProviderResult& , const int64_t);
    fptr BSAServiceClient_BSAQueryServiceProvider = (fptr)(&BSAServiceClient::BSAQueryServiceProvider);
    stub.set(BSAServiceClient_BSAQueryServiceProvider, BSAServiceClient_BSAQueryServiceProvider_Stub);
    EXPECT_EQ(MP_SUCCESS, ThriftClientMgr::GetInstance().BSAQueryServiceProviderMgr(&sizePtr, delimiter, providerPtr));
    stub.set(BSAServiceClient_BSAQueryServiceProvider, BSAServiceClient_BSAQueryServiceProvider_throw1_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAQueryServiceProviderMgr(&sizePtr, delimiter, providerPtr));
    stub.set(BSAServiceClient_BSAQueryServiceProvider, BSAServiceClient_BSAQueryServiceProvider_throw2_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAQueryServiceProviderMgr(&sizePtr, delimiter, providerPtr));

    stub.set(BSAServiceClient_BSAQueryServiceProvider, BSAServiceClient_BSAQueryServiceProvider2_Stub);
    EXPECT_EQ(BSA_RC_BUFFER_TOO_SMALL, ThriftClientMgr::GetInstance().BSAQueryServiceProviderMgr(&sizePtr, delimiter, providerPtr));
}

void BSAServiceClient_BSATerminate_Stub(void *obj, CallResult &_return, const int64_t handle)
{
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceClient_BSATerminate_throw1_Stub(CallResult &_return, const int64_t handle)
{
    throw TTransportException(TTransportException::NOT_OPEN, "test");
}

void BSAServiceClient_BSATerminate_throw2_Stub(CallResult &_return, const int64_t handle)
{
    throw std::runtime_error("test");
}

TEST_F(ThriftClientTest, BSATerminateMgr) {
    long bsaHandle = 1;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);

    typedef int32_t (*fptr)(BSAServiceClient*, const int64_t);
    fptr BSAServiceClient_BSATerminate = (fptr)(&BSAServiceClient::BSATerminate);
    stub.set(BSAServiceClient_BSATerminate, BSAServiceClient_BSATerminate_Stub);

    BsaHandleContext context;
    ThriftClientMgr::GetInstance().m_readMap.clear();
    ThriftClientMgr::GetInstance().m_readMap[bsaHandle] = context;
    EXPECT_EQ(BSA_RC_SUCCESS, ThriftClientMgr::GetInstance().BSATerminateMgr(bsaHandle));
    EXPECT_EQ(ThriftClientMgr::GetInstance().m_readMap.empty(), true);

    stub.set(BSAServiceClient_BSATerminate, BSAServiceClient_BSATerminate_throw1_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSATerminateMgr(bsaHandle));

    stub.set(BSAServiceClient_BSATerminate, BSAServiceClient_BSATerminate_throw2_Stub);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSATerminateMgr(bsaHandle));

    ThriftClientMgr::GetInstance().m_readMap.clear();
    stub.set(BSAServiceClient_BSATerminate, BSAServiceClient_BSATerminate_Stub);
    EXPECT_EQ(BSA_RC_SUCCESS, ThriftClientMgr::GetInstance().BSATerminateMgr(bsaHandle));
}

void BSAServiceClient_BSASendData_Stub(void *obj, CallResult &_return, const int64_t handle, const BsaDataBlock32 &dataBlock)
{
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceClient_BSASendData_throw1_Stub(CallResult &_return, const int64_t handle, const BsaDataBlock32 &dataBlock)
{
    throw TTransportException(TTransportException::NOT_OPEN, "test");
}

void BSAServiceClient_BSASendData_throw2_Stub(CallResult &_return, const int64_t handle, const BsaDataBlock32 &dataBlock)
{
    throw std::runtime_error("test");
}

int32_t ThriftClientMgr_SendData_Stub(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    return MP_SUCCESS;
}

TEST_F(ThriftClientTest, BSASendDataMgr) {
    long bsaHandle = 1;
    BSA_DataBlock32 dataBlockPtr;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, SendData), ThriftClientMgr_SendData_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);

    typedef int32_t (*fptr)(BSAServiceClient*, const int64_t , const BsaDataBlock32& );
    fptr BSAServiceClient_BSASendData = (fptr)(&BSAServiceClient::BSASendData);

    stub.set(BSAServiceClient_BSASendData, BSAServiceClient_BSASendData_Stub);
    ThriftClientMgr::GetInstance().m_writer.m_writeStatus = FileIoStatus::CLOSE;
    EXPECT_EQ(BSA_RC_SUCCESS, ThriftClientMgr::GetInstance().BSASendDataMgr(bsaHandle, &dataBlockPtr));
    
    stub.set(BSAServiceClient_BSASendData, BSAServiceClient_BSASendData_throw1_Stub);
    ThriftClientMgr::GetInstance().m_writer.m_writeStatus = FileIoStatus::CLOSE;
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSASendDataMgr(bsaHandle, &dataBlockPtr));

    stub.set(BSAServiceClient_BSASendData, BSAServiceClient_BSASendData_throw2_Stub);
    ThriftClientMgr::GetInstance().m_writer.m_writeStatus = FileIoStatus::CLOSE;
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSASendDataMgr(bsaHandle, &dataBlockPtr));

    ThriftClientMgr::GetInstance().m_writer.SetWriteStatus(FileIoStatus::OPEN);
    stub.set(ADDR(File, GetNowTime), StubReturnInt60);
    stub.set(ADDR(File, GetLastTime), StubReturnInt0);
    stub.set(BSAServiceClient_BSASendData, BSAServiceClient_BSASendData_Stub);
    EXPECT_EQ(BSA_RC_SUCCESS, ThriftClientMgr::GetInstance().BSASendDataMgr(bsaHandle, &dataBlockPtr));

    ThriftClientMgr::GetInstance().m_writer.SetWriteStatus(FileIoStatus::OPEN);
    stub.set(ADDR(File, GetNowTime), StubReturnInt60);
    stub.set(ADDR(File, GetLastTime), StubReturnInt31);
    stub.set(ADDR(ThriftClientMgr, SendData), StubFailed);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSASendDataMgr(bsaHandle, &dataBlockPtr));
}

void BSAServiceClient_BSAGetData_Stub(void *obj, GetDataResult& _return, const int64_t handle, const BsaDataBlock32& dataBlock)
{
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceClient_BSAGetData_throw1_Stub(GetDataResult& _return, const int64_t handle, const BsaDataBlock32& dataBlock)
{
    throw TTransportException(TTransportException::NOT_OPEN, "test");
}

void BSAServiceClient_BSAGetData_throw2_Stub(GetDataResult& _return, const int64_t handle, const BsaDataBlock32& dataBlock)
{
    throw std::runtime_error("test");
}

int32_t ThriftClientMgr_GetData_Stub(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    return MP_SUCCESS;
}

void DataConversion_convertdataBlockIn_Stub(BSA_DataBlock32 *src, BsaDataBlock32 &dst)
{}

TEST_F(ThriftClientTest, BSAGetDataMgr) {
    long bsaHandle = 1;
    BSA_DataBlock32 dataBlockPtr;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, GetData), ThriftClientMgr_GetData_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);

    typedef void (*fptr)(BSAServiceClient*, GetDataResult& , const int64_t , const BsaDataBlock32&);
    fptr BSAServiceClient_BSAGetData = (fptr)(&BSAServiceClient::BSAGetData);
    stub.set(BSAServiceClient_BSAGetData, BSAServiceClient_BSAGetData_Stub);
    ThriftClientMgr::GetInstance().m_reader.m_readStatus = FileIoStatus::CLOSE;
    EXPECT_EQ(BSA_RC_NO_MORE_DATA, ThriftClientMgr::GetInstance().BSAGetDataMgr(bsaHandle, &dataBlockPtr));

    stub.set(BSAServiceClient_BSAGetData, BSAServiceClient_BSAGetData_throw1_Stub);
    ThriftClientMgr::GetInstance().m_reader.m_readStatus = FileIoStatus::CLOSE;
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAGetDataMgr(bsaHandle, &dataBlockPtr));

    stub.set(BSAServiceClient_BSAGetData, BSAServiceClient_BSAGetData_throw2_Stub);
    ThriftClientMgr::GetInstance().m_reader.m_readStatus = FileIoStatus::CLOSE;
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAGetDataMgr(bsaHandle, &dataBlockPtr));

    ThriftClientMgr::GetInstance().m_reader.m_readStatus = FileIoStatus::OPEN;
    stub.set(ADDR(File, GetNowTime), StubReturnInt60);
    stub.set(ADDR(File, GetLastTime), StubReturnInt0);
    stub.set(BSAServiceClient_BSAGetData, BSAServiceClient_BSAGetData_Stub);
    EXPECT_EQ(BSA_RC_NO_MORE_DATA, ThriftClientMgr::GetInstance().BSAGetDataMgr(bsaHandle, &dataBlockPtr));

    ThriftClientMgr::GetInstance().m_reader.m_readStatus = FileIoStatus::OPEN;
    stub.set(ADDR(File, GetNowTime), StubReturnInt60);
    stub.set(ADDR(File, GetLastTime), StubReturnInt31);
    stub.set(ADDR(ThriftClientMgr, GetData), StubFailed);
    EXPECT_EQ(BSA_RC_ABORT_SYSTEM_ERROR, ThriftClientMgr::GetInstance().BSAGetDataMgr(bsaHandle, &dataBlockPtr));
}

int FileIO_Open_Stub(long bsaHandle, int protectionType)
{
    return MP_SUCCESS;
}

int FileIO_OpenFailed_Stub(long bsaHandle, int protectionType)
{
    return MP_FAILED;
}

int FileIO_Write_Stub(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    return MP_SUCCESS;
}

int FileIO_WriteFailed_Stub(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    return MP_FAILED;
}

TEST_F(ThriftClientTest, SendData) {
    long bsaHandle = 1;
    BSA_DataBlock32 dataBlockPtr;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(File, OpenForWrite), FileIO_Open_Stub);
    stub.set(ADDR(File, Write), FileIO_Write_Stub);

    ThriftClientMgr::GetInstance().m_writeMap.clear();
    EXPECT_EQ(MP_FAILED, ThriftClientMgr::GetInstance().SendData(bsaHandle, &dataBlockPtr));

    BsaHandleContext context;
    ThriftClientMgr::GetInstance().m_writeMap[bsaHandle] = context;
    EXPECT_EQ(MP_SUCCESS, ThriftClientMgr::GetInstance().SendData(bsaHandle, &dataBlockPtr));

    stub.set(ADDR(File, OpenForWrite), FileIO_OpenFailed_Stub);
    ThriftClientMgr::GetInstance().m_writeMap[bsaHandle] = context;
    EXPECT_EQ(MP_FAILED, ThriftClientMgr::GetInstance().SendData(bsaHandle, &dataBlockPtr));

    stub.set(ADDR(File, OpenForWrite), FileIO_Open_Stub);
    stub.set(ADDR(File, Write), FileIO_WriteFailed_Stub);
    ThriftClientMgr::GetInstance().m_writeMap[bsaHandle] = context;
    EXPECT_EQ(MP_FAILED, ThriftClientMgr::GetInstance().SendData(bsaHandle, &dataBlockPtr));
}

int FileIO_Read_Stub(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    return MP_SUCCESS;
}

int FileIO_ReadFailed_Stub(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    return MP_FAILED;
}

int FileIO_ConnectArchiveFailed_Stub(long bsaHandle, BsaHandleContext &context)
{
    return MP_FAILED;
}

int FileIO_ConnectArchive_Stub(long bsaHandle, BsaHandleContext &context)
{
    return MP_SUCCESS;
}

int FileIO_ReadFromArchiveFailed_Stub(long bsaHandle, BSA_DataBlock32 *dataBlockPtr, BsaHandleContext &context)
{
    return MP_FAILED;
}

int FileIO_ReadFromArchive_Stub(long bsaHandle, BSA_DataBlock32 *dataBlockPtr, BsaHandleContext &context)
{
    return MP_SUCCESS;
}


TEST_F(ThriftClientTest, GetData) {
    long bsaHandle = 1;
    BSA_DataBlock32 dataBlockPtr;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(File, OpenForRead), FileIO_Open_Stub);
    stub.set(ADDR(File, Read), FileIO_Read_Stub);

    ThriftClientMgr::GetInstance().m_readMap.clear();
    EXPECT_EQ(MP_FAILED, ThriftClientMgr::GetInstance().GetData(bsaHandle, &dataBlockPtr));

    BsaHandleContext context;
    ThriftClientMgr::GetInstance().m_readMap[bsaHandle] = context;
    EXPECT_EQ(MP_SUCCESS, ThriftClientMgr::GetInstance().GetData(bsaHandle, &dataBlockPtr));

    // DataType == BSA_GET_DATA_FROM_NAS情况
    context.workingObj.getDataType = BSA_GET_DATA_FROM_NAS;
    ThriftClientMgr::GetInstance().m_readMap.clear();
    ThriftClientMgr::GetInstance().m_readMap[bsaHandle] = context;
    stub.set(ADDR(File, OpenForRead), FileIO_OpenFailed_Stub);
    EXPECT_EQ(MP_FAILED, ThriftClientMgr::GetInstance().GetData(bsaHandle, &dataBlockPtr));

    stub.set(ADDR(File, OpenForRead), FileIO_Open_Stub);
    stub.set(ADDR(File, Read), FileIO_ReadFailed_Stub);
    EXPECT_EQ(MP_FAILED, ThriftClientMgr::GetInstance().GetData(bsaHandle, &dataBlockPtr));

    // DataType == BSA_GET_DATA_FROM_ARCHIVE情况
    context.workingObj.getDataType = BSA_GET_DATA_FROM_ARCHIVE;
    ThriftClientMgr::GetInstance().m_readMap.clear();
    ThriftClientMgr::GetInstance().m_readMap[bsaHandle] = context;
    stub.set(ADDR(File, ConnectArchive), FileIO_ConnectArchiveFailed_Stub);
    EXPECT_EQ(MP_FAILED, ThriftClientMgr::GetInstance().GetData(bsaHandle, &dataBlockPtr));

    stub.set(ADDR(File, ConnectArchive), FileIO_ConnectArchive_Stub);
    stub.set(ADDR(File, ReadFromArchive), FileIO_ReadFromArchiveFailed_Stub);
    EXPECT_EQ(MP_SUCCESS, ThriftClientMgr::GetInstance().GetData(bsaHandle, &dataBlockPtr));

    stub.set(ADDR(File, ReadFromArchive), FileIO_ReadFromArchive_Stub);
    EXPECT_EQ(MP_SUCCESS, ThriftClientMgr::GetInstance().GetData(bsaHandle, &dataBlockPtr));
}

/*
* 测试用例：SaveObjIntoReadMap
* 前置条件：无
* CHECK点：1.先bsaHandle插入m_readMap，再根据对象名称插入queryMap
*/
TEST_F(ThriftClientTest, SaveObjIntoReadMap_Test)
{
    long bsaHandle = 2;
    QueryObjectResult objectDescriptor;
    objectDescriptor.objectDesc.objectName.pathName = "/roach/file1";

    ThriftClientMgr::GetInstance().m_readMap.clear();
    ThriftClientMgr::GetInstance().SaveObjIntoReadMap(bsaHandle, objectDescriptor);
    EXPECT_EQ(ThriftClientMgr::GetInstance().m_readMap.size(), 1);
    EXPECT_EQ(ThriftClientMgr::GetInstance().m_readMap[bsaHandle].queryMap.size(), 0);

    objectDescriptor.storePath = "/tmp/roach/file1";
    ThriftClientMgr::GetInstance().SaveObjIntoReadMap(bsaHandle, objectDescriptor);
    EXPECT_EQ(ThriftClientMgr::GetInstance().m_readMap.size(), 1);
    EXPECT_EQ(ThriftClientMgr::GetInstance().m_readMap[bsaHandle].queryMap.size(), 1);

    objectDescriptor.objectDesc.objectName.pathName = "/roach/file2";
    objectDescriptor.storePath = "/tmp/roach/file2";
    ThriftClientMgr::GetInstance().SaveObjIntoReadMap(bsaHandle, objectDescriptor);
    EXPECT_EQ(ThriftClientMgr::GetInstance().m_readMap.size(), 1);
    EXPECT_EQ(ThriftClientMgr::GetInstance().m_readMap[bsaHandle].queryMap.size(), 2);

    ThriftClientMgr::GetInstance().m_readMap.clear();
    objectDescriptor.storePath = "/tmp/roach/file1";
    objectDescriptor.objectDesc.objectName.pathName = "/roach/file2";
    ThriftClientMgr::GetInstance().SaveObjIntoReadMap(bsaHandle, objectDescriptor);
    EXPECT_EQ(ThriftClientMgr::GetInstance().m_readMap.size(), 1);
    EXPECT_EQ(ThriftClientMgr::GetInstance().m_readMap[bsaHandle].queryMap.size(), 1);
}

TEST_F(ThriftClientTest, InitModule_Test)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CLogger, Init), CLogger_Log_Stub);
    stub.set(ADDR(CPath, GetLogPath), StubGetLogPath);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Load_Stub_Failed);
    stub.set(ADDR(CPath, GetConfFilePath), StubGetConfFilePath);
    EXPECT_EQ(MP_FAILED, ThriftClientMgr::GetInstance().InitModule());
}

TEST_F(ThriftClientTest, InitNoParam_Test)
{
    mp_int32 entryCount = 0;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), StubFailed);
    ThriftClientMgr::GetInstance().Init();
    EXPECT_EQ(0, entryCount);
}

TEST_F(ThriftClientTest, Init_Test)
{
    Stub stub;
    bool ssl = true;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    stub.set(&AGENT_API::InitCrypt, StubFailed);
    mp_int32 iRet = ThriftClientMgr::GetInstance().Init(ssl);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(&AGENT_API::InitCrypt, StubSuccess);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubGetValueString);
    iRet = ThriftClientMgr::GetInstance().Init(ssl);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(ThriftClientTest, CreateSock_Test)
{
    Stub stub;
    bool ssl = true;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    std::shared_ptr<apache::thrift::transport::TSocket> Ret = ThriftClientMgr::GetInstance().CreateSock(ssl);
    EXPECT_NE(Ret, nullptr);

    stub.set(&CPath::GetConfFilePath, StubCreateSockGetConfFilePath);
    stub.set(ADDR(SecureCom, GetHostFromCert), GetHostFromCert_Stub);
    Ret = ThriftClientMgr::GetInstance().CreateSock(ssl);
    EXPECT_NE(Ret, nullptr);

    stub.set(&CPath::GetConfFilePath, StubCreateSockGetConfFilePath);
    stub.set(ADDR(SecureCom, GetHostFromCert), StubGetHostFromCert);
    Ret = ThriftClientMgr::GetInstance().CreateSock(ssl);
    EXPECT_NE(Ret, nullptr);
}

TEST_F(ThriftClientTest, GetClient_Test)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    stub.set(&ThriftClientMgr::CreateSock, StubCreateSockNull);
    std::shared_ptr<BSAServiceClient> Ret = ThriftClientMgr::GetInstance().GetClient();
    EXPECT_EQ(Ret, nullptr);

    stub.set(&ThriftClientMgr::CreateSock, StubCreateSockNotNull);
    Ret = ThriftClientMgr::GetInstance().GetClient();
    EXPECT_EQ(Ret, nullptr);
}

TEST_F(ThriftClientTest, BSACreateObjectMgr_Test)
{
    Stub stub;
    long bsaHandle;
    BSA_ObjectDescriptor objectDescriptorPtr;
    BSA_DataBlock32 dataBlockPtr;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);
    typedef void (*fptr)(BSAServiceClient*,CreateObjectResult&, const int64_t, const BsaObjectDescriptor&);
    fptr BSAServiceClient_BSABeginTxn = (fptr)(&BSAServiceClient::BSACreateObject);
    stub.set(BSAServiceClient_BSABeginTxn, BSAServiceClient_BSACreateObject_Stub);
    stub.set(&DataConversion::ConvertObjectDescriptorOut, StubFalse);
    int32_t iRet = ThriftClientMgr::GetInstance().BSACreateObjectMgr(bsaHandle, &objectDescriptorPtr, &dataBlockPtr);
    EXPECT_EQ(iRet, BSA_RC_ABORT_SYSTEM_ERROR);
}


TEST_F(ThriftClientTest, SetWorkingObj_Test)
{
    Stub stub;
    long bsaHandle = 1;
    BSA_ObjectDescriptor objectDescriptorPtr;
    BSA_ObjectName objectName;
    char pathName[1024] = "test1";
    memcpy(objectName.pathName, pathName, sizeof(pathName));
    objectDescriptorPtr.objectName = objectName;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    typedef void (*fptr)(BSAServiceClient*,CreateObjectResult&, const int64_t, const BsaObjectDescriptor&);
    fptr BSAServiceClient_BSABeginTxn = (fptr)(&BSAServiceClient::BSACreateObject);

    BsaHandleContext context;
    std::map<std::string, BsaObjContext> queryMap;
    BsaObjContext bsaObjContext;
    queryMap["test1"] = bsaObjContext;
    queryMap["test2"] = bsaObjContext;

    context.queryMap = queryMap;
    ThriftClientMgr::GetInstance().m_readMap.clear();
    mp_bool bRet = ThriftClientMgr::GetInstance().SetWorkingObj(bsaHandle, &objectDescriptorPtr);
    EXPECT_EQ(bRet, MP_FALSE);

    ThriftClientMgr::GetInstance().m_readMap[bsaHandle] = context;
    bRet = ThriftClientMgr::GetInstance().SetWorkingObj(bsaHandle, &objectDescriptorPtr);
    EXPECT_EQ(bRet, MP_TRUE);
    ThriftClientMgr::GetInstance().m_readMap.clear();
}

void BSAServiceClient_BSAInitMgr_Stub(void *obj, BSAInitResult& _return, const BsaObjectOwner& objectOwner, const std::string& envPtr)
{
    _return.response = BSA_RC_SUCCESS;
}

void BSAServiceClient_BSAInitMgr_throw1_Stub(BSAInitResult& _return, const BsaObjectOwner& objectOwner, const std::string& envPtr)
{
    throw TTransportException(TTransportException::NOT_OPEN, "test");
}

TEST_F(ThriftClientTest, BSAInitMgr_Test)
{
    Stub stub;
    long bsaHandlePtr;
    BSA_SecurityToken tokenPtr;
    BSA_ObjectOwner objectOwnerPtr;
    char environmentPtr;
    char* enenvironmentPtrP = &environmentPtr;

    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    typedef void (*fptr7)(TFramedTransport*);
    fptr7 TFramedTransport_open = (fptr7)(&TFramedTransport::open);
    stub.set(TFramedTransport_open, TFramedTransport_open_Stub);

    typedef void (*fptr)(BSAServiceClient*,BSAInitResult&, const BsaObjectOwner&, const std::string&);
    fptr BSAServiceClient_BSAInitMgr = (fptr)(&BSAServiceClient::BSAInit);
    stub.set(BSAServiceClient_BSAInitMgr, BSAServiceClient_BSAInitMgr_Stub);
    mp_int32 iRet = ThriftClientMgr::GetInstance().BSAInitMgr(&bsaHandlePtr, &tokenPtr, &objectOwnerPtr,
                                                              &enenvironmentPtrP, BSA_AppType::BSA_DWS);
    EXPECT_EQ(iRet, BSA_RC_SUCCESS);

    stub.set(BSAServiceClient_BSAInitMgr, BSAServiceClient_BSAInitMgr_throw1_Stub);
    iRet = ThriftClientMgr::GetInstance().BSAInitMgr(&bsaHandlePtr, &tokenPtr, &objectOwnerPtr, &enenvironmentPtrP,
                                                     BSA_AppType::BSA_DWS);
    EXPECT_EQ(iRet, BSA_RC_ABORT_SYSTEM_ERROR);
}

TEST_F(ThriftClientTest, GetInformixVersion_TEST)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CPath, GetConfFilePath), StubGetConfFilePath);
    stub.set(ADDR(CMpFile, ReadFile), StubFailed);

    mp_int32 iRet = ThriftClientMgr::GetInstance().GetInformixVersion();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CMpFile, ReadFile), StubReadFileSucc);
    stub.set(ADDR(JsonHelper, JsonStringToJsonValue), StubFalse);
    iRet = ThriftClientMgr::GetInstance().GetInformixVersion();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(JsonHelper, JsonStringToJsonValue), StubTrue);
    stub.set(ADDR(CJsonUtils, GetJsonString), StubFailed);
    iRet = ThriftClientMgr::GetInstance().GetInformixVersion();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CJsonUtils, GetJsonString), StubSuccess);
    iRet = ThriftClientMgr::GetInstance().GetInformixVersion();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

