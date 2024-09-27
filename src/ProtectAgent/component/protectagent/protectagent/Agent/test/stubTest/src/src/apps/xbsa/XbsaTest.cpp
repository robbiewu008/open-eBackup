#include "apps/xbsa/XbsaTest.h"
#include "common/ConfigXmlParse.h"

namespace {
mp_void LogReturn(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...)
{
    return;
}

int32_t BSABeginTxnMgrFailed(long bsaHandle)
{
    return BSA_RC_ABORT_SYSTEM_ERROR;
}

int32_t BSABeginTxnMgrSuccess(long bsaHandle)
{
    return BSA_RC_SUCCESS;
}

int32_t BSADeleteObjectMgrFailed(long bsaHandle, BSA_UInt64 copyId)
{
    return BSA_RC_ABORT_SYSTEM_ERROR;
}

int32_t BSADeleteObjectMgrSuccess(long bsaHandle, BSA_UInt64 copyId)
{
    return BSA_RC_SUCCESS;
}

int32_t BSAEndDataMgrFailed(long bsaHandle)
{
    return BSA_RC_ABORT_SYSTEM_ERROR;
}

int32_t BSAEndDataMgrSuccess(long bsaHandle)
{
    return BSA_RC_SUCCESS;
}

int32_t BSAEndTxnMgrFailed(long bsaHandle, BSA_Vote vote)
{
    return BSA_RC_ABORT_SYSTEM_ERROR;
}

int32_t BSAEndTxnMgrSuccess(long bsaHandle, BSA_Vote vote)
{
    return BSA_RC_SUCCESS;
}

int32_t BSAGetDataMgrFailed(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    return BSA_RC_ABORT_SYSTEM_ERROR;
}

int32_t BSAGetDataMgrSuccess(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    return BSA_RC_SUCCESS;
}

int32_t BSAGetLastErrorMgrFailed(BSA_UInt32 *sizePtr, char *errorCodePtr)
{
    return BSA_RC_ABORT_SYSTEM_ERROR;
}

int32_t BSAGetLastErrorMgrSuccess(BSA_UInt32 *sizePtr, char *errorCodePtr)
{
    return BSA_RC_SUCCESS;
}

int32_t BSAGetNextQueryObjectMgrFailed(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr)
{
    return BSA_RC_ABORT_SYSTEM_ERROR;
}

int32_t BSAGetNextQueryObjectMgrSuccess(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr)
{
    return BSA_RC_SUCCESS;
}

int32_t BSAGetObjectMgrFailed(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr, BSA_DataBlock32 *dataBlockPtr)
{
    return BSA_RC_ABORT_SYSTEM_ERROR;
}

int32_t BSAGetObjectMgrSuccess(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr, BSA_DataBlock32 *dataBlockPtr)
{
    return BSA_RC_SUCCESS;
}

int32_t BSAInitMgrFailed(long *bsaHandlePtr, BSA_SecurityToken *tokenPtr, BSA_ObjectOwner *objectOwnerPtr, char **environmentPtr)
{
    return BSA_RC_ABORT_SYSTEM_ERROR;
}

int32_t BSAInitMgrSuccess(long *bsaHandlePtr, BSA_SecurityToken *tokenPtr, BSA_ObjectOwner *objectOwnerPtr, char **environmentPtr)
{
    return BSA_RC_SUCCESS;
}

int32_t BSAQueryApiVersionMgrFailed(BSA_ApiVersion *apiVersionPtr)
{
    return BSA_RC_ABORT_SYSTEM_ERROR;
}

int32_t BSAQueryApiVersionMgrSuccess(BSA_ApiVersion *apiVersionPtr)
{
    return BSA_RC_SUCCESS;
}

int32_t BSAQueryObjectMgrFailed(long bsaHandle, BSA_QueryDescriptor *queryDescriptorPtr, BSA_ObjectDescriptor *objectDescriptorPtr)
{
    return BSA_RC_ABORT_SYSTEM_ERROR;
}

int32_t BSAQueryObjectMgrSuccess(long bsaHandle, BSA_QueryDescriptor *queryDescriptorPtr, BSA_ObjectDescriptor *objectDescriptorPtr)
{
    return BSA_RC_SUCCESS;
}

int32_t BSATerminateMgrFailed(long bsaHandle)
{
    return BSA_RC_ABORT_SYSTEM_ERROR;
}

int32_t BSATerminateMgrSuccess(long bsaHandle)
{
    return BSA_RC_SUCCESS;
}

int32_t BSASendDataMgrFailed(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    return BSA_RC_ABORT_SYSTEM_ERROR;
}

int32_t BSASendDataMgrSuccess(long bsaHandle, BSA_DataBlock32 *dataBlockPtr)
{
    return BSA_RC_SUCCESS;
}

int32_t BSAQueryServiceProviderMgrFailed(BSA_UInt32 *sizePtr, char *delimiter, char *providerPtr)
{
    return BSA_RC_ABORT_SYSTEM_ERROR;
}

int32_t BSAQueryServiceProviderMgrSuccess(BSA_UInt32 *sizePtr, char *delimiter, char *providerPtr)
{
    return BSA_RC_SUCCESS;
}

mp_void CLogger_Log_Stub(mp_void* pthis)
{
    return;
}

mp_int32 StubBSAMgr()
{
    return BSA_RC_SUCCESS;
}

mp_int32 StubBSABSABeginTxn(mp_void* pthis, long bsaHandle)
{
    return BSA_RC_SUCCESS;
}


mp_int32 StubBSACreateObjectMgr(mp_void* pthis)
{
    return BSA_RC_SUCCESS;
}

mp_int32 CConfigXmlParser_Init_Stub(mp_string strCfgFilePath)
{
    return MP_SUCCESS;
}
mp_int32 CConfigXmlParser_GetValueBool_Stub(const mp_string& strSection, const mp_string& strKey, mp_bool& bValue)
{
    return MP_SUCCESS;
}
int32_t ThriftClientMgr_InitModule_Stub()
{
    return MP_SUCCESS;
}
std::shared_ptr<apache::thrift::transport::TSocket> Stub_CreateSock()
{
    const std::string DEFAULT_IP = "127.0.0.1";
    const int DEFAULT_HOSTPORT = 59560;
    return std::make_shared<apache::thrift::transport::TSocket>(DEFAULT_IP, DEFAULT_HOSTPORT);
}
}


TEST_F(XbsaTest, BSABeginTxn) {
    long bsaHandle = 0;
    Stub stub;

    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);
    
    stub.set(ADDR(ThriftClientMgr, BSABeginTxnMgr), StubBSABSABeginTxn);
    EXPECT_EQ(BSA_RC_INVALID_HANDLE, BSABeginTxn(bsaHandle));
    
    bsaHandle = 1;
    stub.set(ADDR(ThriftClientMgr, BSABeginTxnMgr), StubBSABSABeginTxn);
    EXPECT_EQ(BSA_RC_SUCCESS, BSABeginTxn(bsaHandle));
}

TEST_F(XbsaTest, BSACreateObject) {
    long bsaHandle = 0;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);
    BSA_ObjectDescriptor objectDescriptorPtrs;
    BSA_DataBlock32 dataBlockPtrs;

    // objectDescriptorPtr == Null
    // BSA_ObjectDescriptor objectDescriptorPtr = NULL;
    // BSA_DataBlock32 dataBlockPtr = NULL;
    // EXPECT_EQ(BSA_RC_NULL_ARGUMENT, BSACreateObject(bsaHandle, &objectDescriptorPtr, &dataBlockPtr));
    // EXPECT_EQ(BSA_RC_NULL_ARGUMENT, BSACreateObject(bsaHandle, &objectDescriptorPtrs, &dataBlockPtr));
    // bsaHandle <= 0
    EXPECT_EQ(BSA_RC_INVALID_HANDLE, BSACreateObject(bsaHandle, &objectDescriptorPtrs, &dataBlockPtrs));
    // copyType == BSA_CopyType_ANY
    bsaHandle = 1;
    objectDescriptorPtrs.copyType = BSA_CopyType_ANY;
    EXPECT_EQ(BSA_RC_INVALID_OBJECTDESCRIPTOR, BSACreateObject(bsaHandle, &objectDescriptorPtrs, &dataBlockPtrs));
    // copyType == BSA_ObjectType_ANY
    bsaHandle = 1;
    objectDescriptorPtrs.objectType = BSA_ObjectType_ANY;
    EXPECT_EQ(BSA_RC_INVALID_OBJECTDESCRIPTOR, BSACreateObject(bsaHandle, &objectDescriptorPtrs, &dataBlockPtrs));
    // strlen <= 0
    char pathName[1024] = "";
    memcpy(objectDescriptorPtrs.objectName.pathName, pathName, sizeof(pathName));
    objectDescriptorPtrs.objectType = BSA_ObjectType_FILE;
    objectDescriptorPtrs.copyType = BSA_CopyType_ARCHIVE;
    EXPECT_EQ(BSA_RC_INVALID_OBJECTDESCRIPTOR, BSACreateObject(bsaHandle, &objectDescriptorPtrs, &dataBlockPtrs));

    char pathName1[1024] = "test";
    memcpy(objectDescriptorPtrs.objectName.pathName, pathName1, sizeof(pathName1));
    stub.set(ADDR(ThriftClientMgr, BSACreateObjectMgr), StubBSACreateObjectMgr);
    EXPECT_EQ(BSA_RC_SUCCESS, BSACreateObject(bsaHandle, &objectDescriptorPtrs, &dataBlockPtrs));
}

TEST_F(XbsaTest, BSADeleteObject) {
    long bsaHandle = 0;
    BSA_UInt64 copyId;
    Stub stub;
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    
    EXPECT_EQ(BSA_RC_INVALID_HANDLE, BSADeleteObject(bsaHandle, copyId));
    
    bsaHandle = 1;
    stub.set(ADDR(ThriftClientMgr, BSADeleteObjectMgr), StubBSAMgr);
    EXPECT_EQ(BSA_RC_SUCCESS, BSADeleteObject(bsaHandle, copyId));
}

TEST_F(XbsaTest, BSAEndData) {
    long bsaHandle = 0;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);
    
    EXPECT_EQ(BSA_RC_INVALID_HANDLE, BSAEndData(bsaHandle));
    
    bsaHandle = 1;
    stub.set(ADDR(ThriftClientMgr, BSAEndDataMgr), StubBSAMgr);
    EXPECT_EQ(BSA_RC_SUCCESS, BSAEndData(bsaHandle));
}

TEST_F(XbsaTest, BSAEndTxn) {
    long bsaHandle = 0;
    BSA_Vote vote;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);
    
    EXPECT_EQ(BSA_RC_INVALID_HANDLE, BSAEndTxn(bsaHandle, vote));
    
    bsaHandle = 1;
    stub.set(ADDR(ThriftClientMgr, BSAEndTxnMgr), StubBSAMgr);
    EXPECT_EQ(BSA_RC_SUCCESS, BSAEndTxn(bsaHandle, vote));
}

TEST_F(XbsaTest, BSAGetData) {
    long bsaHandle = 0;
    BSA_DataBlock32 dataBlockPtr;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);
    
    EXPECT_EQ(BSA_RC_INVALID_HANDLE, BSAGetData(bsaHandle, &dataBlockPtr));
    
    bsaHandle = 1;
    stub.set(ADDR(ThriftClientMgr, BSAGetDataMgr), StubBSAMgr);
    EXPECT_EQ(BSA_RC_SUCCESS, BSAGetData(bsaHandle, &dataBlockPtr));
}

TEST_F(XbsaTest, BSAGetEnvironment) {
    long bsaHandle = 0;
    BSA_ObjectOwner objectOwner;
    char ptr;
    char* Pprt = &ptr;
    EXPECT_EQ(BSA_RC_SUCCESS, BSAGetEnvironment(bsaHandle, &objectOwner, &Pprt));
}

TEST_F(XbsaTest, BSAGetLastError) {
    BSA_UInt32 sizePtr;
    char errorCodePtr;
    Stub stub;
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    stub.set(ADDR(ThriftClientMgr, BSAGetLastErrorMgr), StubBSAMgr);
    EXPECT_EQ(BSA_RC_SUCCESS, BSAGetLastError(&sizePtr, &errorCodePtr));
}

TEST_F(XbsaTest, BSAGetNextQueryObject) {
    long bsaHandle = 0;
    BSA_ObjectDescriptor objectDescriptorPtr;
    Stub stub;
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);
    EXPECT_EQ(BSA_RC_INVALID_HANDLE, BSAGetNextQueryObject(bsaHandle, &objectDescriptorPtr));

    bsaHandle = 1;
    stub.set(ADDR(ThriftClientMgr, BSAGetNextQueryObjectMgr), StubBSAMgr);
    EXPECT_EQ(BSA_RC_SUCCESS, BSAGetNextQueryObject(bsaHandle, &objectDescriptorPtr));
}

TEST_F(XbsaTest, BSAGetObject) {
    long bsaHandle = 0;
    BSA_ObjectDescriptor objectDescriptorPtr;
    BSA_DataBlock32 dataBlockPtr;
    Stub stub;
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    stub.set(ADDR(ThriftClientMgr, BSAGetObjectMgr), StubBSAMgr);
    EXPECT_EQ(BSA_RC_INVALID_HANDLE, BSAGetObject(bsaHandle, &objectDescriptorPtr, &dataBlockPtr));

    bsaHandle = 1;
    EXPECT_EQ(BSA_RC_SUCCESS, BSAGetObject(bsaHandle, &objectDescriptorPtr, &dataBlockPtr));
}

TEST_F(XbsaTest, BSAInit) {
    Stub stub;
    long bsaHandlePtr = 0;
    BSA_SecurityToken tokenPtr;
    BSA_ObjectOwner objectOwnerPtr;
    char environmentPtr;
    char* environmentPtrP = &environmentPtr;
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    stub.set(ADDR(ThriftClientMgr, BSAInitMgr), StubBSAMgr);
    EXPECT_EQ(BSA_RC_SUCCESS, BSAInit(&bsaHandlePtr, &tokenPtr, &objectOwnerPtr, &environmentPtrP));
}

TEST_F(XbsaTest, BSAQueryApiVersion) {
    Stub stub;
    BSA_ApiVersion apiVersionPtr;
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    stub.set(ADDR(ThriftClientMgr, BSAQueryApiVersionMgr), StubBSAMgr);
    EXPECT_EQ(BSA_RC_SUCCESS, BSAQueryApiVersion(&apiVersionPtr));
}

TEST_F(XbsaTest, BSAQueryObject) {
    Stub stub;
    long bsaHandlePtr = 0;
    BSA_QueryDescriptor queryDescriptorPtr;
    BSA_ObjectDescriptor objectDescriptorPtr;
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    stub.set(ADDR(ThriftClientMgr, BSAQueryObjectMgr), StubBSAMgr);
    EXPECT_EQ(BSA_RC_INVALID_HANDLE, BSAQueryObject(bsaHandlePtr ,&queryDescriptorPtr, &objectDescriptorPtr));

    bsaHandlePtr = 1;
    EXPECT_EQ(BSA_RC_SUCCESS, BSAQueryObject(bsaHandlePtr ,&queryDescriptorPtr, &objectDescriptorPtr));
}

TEST_F(XbsaTest, BSAQueryServiceProvider) {
    Stub stub;
    BSA_UInt32 sizePtr;
    char delimiter;
    char providerPtr;
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);

    stub.set(ADDR(ThriftClientMgr, BSAQueryServiceProviderMgr), StubBSAMgr);
    EXPECT_EQ(BSA_RC_SUCCESS, BSAQueryServiceProvider(&sizePtr, &delimiter, &providerPtr));
}

TEST_F(XbsaTest, BSASendData) {
    long bsaHandle = 0;
    BSA_DataBlock32 dataBlockPtr;
    Stub stub;
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);
    stub.set(ADDR(ThriftClientMgr, BSASendDataMgr), StubBSAMgr);
    EXPECT_EQ(BSA_RC_SUCCESS, BSASendData(bsaHandle, &dataBlockPtr));
}

TEST_F(XbsaTest, BSATerminate) {
    long bsaHandle = 0;
    Stub stub;
    stub.set(ADDR(CConfigXmlParser, Init), CConfigXmlParser_Init_Stub);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_bool&))ADDR(CConfigXmlParser,GetValueBool),
        CConfigXmlParser_GetValueBool_Stub);
    stub.set(ADDR(ThriftClientMgr, InitModule), ThriftClientMgr_InitModule_Stub);
    stub.set(ADDR(ThriftClientMgr, CreateSock), Stub_CreateSock);
    EXPECT_EQ(BSA_RC_INVALID_HANDLE, BSATerminate(bsaHandle));

    bsaHandle = 1;
    stub.set(ADDR(ThriftClientMgr, BSATerminateMgr), StubBSAMgr);
    EXPECT_EQ(BSA_RC_SUCCESS, BSATerminate(bsaHandle));
}

TEST_F(XbsaTest, NBBSAGetErrorString) {
    int errCode;
    BSA_UInt32 sizePtr;
    char errorCodePtr;
    EXPECT_EQ(BSA_RC_SUCCESS, NBBSAGetErrorString(errCode, &sizePtr, &errorCodePtr));
}

TEST_F(XbsaTest, NBBSAGetServerError) {
    long bsaHandle = 0;
    int serverStatus;
    BSA_UInt32 sizePtr;
    char serverStatusStr;
    EXPECT_EQ(BSA_RC_SUCCESS, NBBSAGetServerError(bsaHandle, &serverStatus, sizePtr, &serverStatusStr));
}

TEST_F(XbsaTest, NBBSASetEnv) {
    long bsaHandle = 0;
    char envKey;
    char envVal;
    EXPECT_EQ(BSA_RC_SUCCESS, NBBSASetEnv(bsaHandle, &envKey, &envVal));
}