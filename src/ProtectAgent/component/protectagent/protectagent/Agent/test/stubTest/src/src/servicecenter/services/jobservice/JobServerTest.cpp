#define private public
#define protected public

#include <services/jobservice/JobServerTest.h>
#include <servicecenter/services/jobservice/detail/JobServer.h>
#include <servicecenter/services/jobservice/include/IJobServer.h>
#include <servicecenter/servicefactory/include/ServiceFactory.h>
#include <pluginfx/ExternalPluginManager.h>
#include <servicecenter/messageservice/detail/Subject.h>
#include <servicecenter/services/jobservice/detail/JobServiceFactory.h>
#include <servicecenter/services/jobservice/detail/JobServerRpcPublishObserver.h>
#include <apps/appprotect/plugininterface/JobServiceHandler.h>
#include <taskmanager/externaljob/AppProtectJobHandler.h>
#include "thriftservice/detail/ThriftServer.h"

using namespace jobservice;
using namespace servicecenter;
using namespace messageservice;
using namespace messageservice::detail;

namespace {
mp_void LogTest()
{}
#define DO_LOG_TEST()                                                                                                    \
    do {                                                                                                               \
        stub11.set(ADDR(CLogger, Log), LogTest);                                                                       \
    } while (0)

static mp_int32 StubGetValueStringSec(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_SUCCESS;
}

static mp_int32 StubGetValueStringParentSec(
    mp_void* pthis, const mp_string& strParentSection, const mp_string& strChildSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_SUCCESS;
}

static mp_int32 StubGetValueInt32(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

static mp_int32 StubStartGettingJobSrv()
{
    return MP_SUCCESS;
}

static mp_uint32 StubGetHttpStatusCode()
{
    return SC_BAD_REQUEST;
}

mp_void StubVoid()
{
    return;
}

mp_bool StubFalse()
{
    return MP_FALSE;
}

std::shared_ptr<JobServiceFactory> StubJobServiceFactoryGetInstance()
{
    std::shared_ptr<JobServiceFactory> instance;
    return instance;
}

std::shared_ptr<messageservice::RpcPublishObserver> StubCreateObserver()
{
    std::shared_ptr<messageservice::RpcPublishObserver> observer;
    return observer;
}
}  // namespace

static std::shared_ptr<Subject> g_subject = std::make_shared<Subject>();
static bool g_registered = false;

bool RegisterObserverStub()
{
    g_registered = true;
    return true;
}

static bool g_funcCalled = false;

mp_int32 AddNewJobStub(mp_void* pThis, const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse)
{
    httpResponse.statusCode = SC_OK;
    g_funcCalled = true;
    return MP_SUCCESS;
}

mp_int32 ReportJobDetailsStub(AppProtect::ActionResult& _return, const AppProtect::SubJobDetails& jobInfo)
{
    g_funcCalled = true;
    return MP_SUCCESS;
}

mp_int32 ReportCopyAdditionalInfoStub(const DmeRestClient::HttpReqParam& httpParam, mp_string& responseBody)
{
    g_funcCalled = true;
    return MP_SUCCESS;
}

mp_int32 StubSendRequestSuccess(mp_void* pThis, const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse)
{
    g_funcCalled = true;
    Json::Value value;
    value["errorCode"] = "0";
    httpResponse.body = value.toStyledString();
    return MP_SUCCESS;
}

mp_int32 StubGetUbcIpsByMainJobId(mp_void* pThis, const mp_string mainJobId, std::vector<mp_string>& ubcIps)
{
    return MP_SUCCESS;
}
/*
 * 用例名称：JobServer服务初始化注册RPC事件
 * 前置条件：1、JobServer已注册服务中心，2、实例化JobServer对象
 * check点：1、检查注册rpc事件是否触发
 */
TEST_F(JobServerTest, JobServer_initailize_test_rpc_observer_register_test_true)
{
    typedef bool (*pJobServer)(ExternalPluginManager*);
    pJobServer pClientAddr = (pJobServer)(&JobServer::RegisterRpcObserver);
    stub11.set(pClientAddr, RegisterObserverStub);

    auto ret = ServiceFactory::GetInstance()->Register<JobServer>("IJobServer");
    auto jobservice = ServiceFactory::GetInstance()->GetService<IJobServer>("IJobServer");
    EXPECT_EQ(g_registered, true);
}

/*
 * 用例名称：RpcObserver对象初始化后TPorcessor已设置OK
 * 前置条件：1、JobServiceFactory获取JobServerRpcPublishObserver对象
 * check点：1、检查JobServerRpcPublishObserver内部状态已设置OK
 */
TEST_F(JobServerTest, JobServerRpcPublishObserver_initailize_tprocessor_is_setted_test_not_null)
{

    auto observer = JobServiceFactory::GetInstance()->CreateObserver();
    auto rpcObserve = std::dynamic_pointer_cast<JobServerRpcPublishObserver>(observer);
    EXPECT_NE(rpcObserve, nullptr);
    EXPECT_NE(rpcObserve->m_processor, nullptr);
}

/*
 * 用例名称：JobServiceHandler对象转发插件PRC请求到DmeRestClient
 * 前置条件：1、获取JobServiceHandler对象
 * check点：1、检查AddNewJobStub是否转发到了DmeRestClient::AddNewJobStub接口
 */
TEST_F(JobServerTest, JobServiceHandler_call_RollbackDataAfterBackupFailed_dispatchor_to_AddNewJob)
{
    DO_LOG_TEST();
    typedef bool (*pRestClient)(DmeRestClient*, const DmeRestClient::HttpReqParam&, mp_string&);
    pRestClient pClientAddr = (pRestClient)(&DmeRestClient::SendRequest);
    stub11.set(pClientAddr, AddNewJobStub);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringSec);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, const mp_string&, mp_string&))ADDR( CConfigXmlParser, GetValueString),
        &StubGetValueStringParentSec);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        &StubGetValueInt32);
    stub11.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);

    AppProtect::ActionResult result;
    std::vector<AppProtect::SubJob> subjobs;
    AppProtect::SubJob job;
    job.jobId = "123456";
    job.jobName = "test";
    job.policy = ExecutePolicy::ANY_NODE;
    job.jobInfo = "test params";
    subjobs.push_back(job);
    g_funcCalled = false;
    auto handler = std::make_shared<JobServiceHandler>();
    handler->AddNewJob(result, subjobs);
    EXPECT_EQ(g_funcCalled, true);
}

/*
 * 用例名称：JobServiceHandler对象转发插件PRC请求到DmeRestClient
 * 前置条件：1、获取JobServiceHandler对象
 * check点：1、检查ReportJobDetails是否转发到了DmeRestClient::ReportJobDetails接口
 */
TEST_F(JobServerTest, JobServiceHandler_call_RollbackDataAfterBackupFailed_dispatchor_to_ReportJobDetails)
{
    DO_LOG_TEST();
    typedef bool (*pRestClient)(
        AppProtect::AppProtectJobHandler*, AppProtect::ActionResult&, const AppProtect::SubJobDetails&);
    pRestClient pClientAddr = (pRestClient)(&AppProtect::AppProtectJobHandler::ReportJobDetails);
    stub11.set(pClientAddr, ReportJobDetailsStub);
    stub11.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, StartGettingJobSrv),
        StubStartGettingJobSrv);

    AppProtect::ActionResult result;
    const AppProtect::SubJobDetails obdetails;
    g_funcCalled = false;
    auto handler = std::make_shared<JobServiceHandler>();
    handler->ReportJobDetails(result, obdetails);
    EXPECT_EQ(g_funcCalled, true);
}

static mp_void SleepStub(mp_void* pthis, mp_uint32 ms)
{
    return;
}

/*
 * 用例名称：JobServiceHandler对象转发插件PRC请求到DmeRestClient
 * 前置条件：1、获取JobServiceHandler对象
 * check点：1、检查ReportExternalImage是否转发到了DmeRestClient::ReportExternalImage接口
 */
TEST_F(JobServerTest, JobServiceHandler_call_RollbackDataAfterBackupFailed_dispatchor_to_ReportExternalCopy)
{
    DO_LOG_TEST();
    typedef bool (*pRestClient)(DmeRestClient*, AppProtect::ActionResult&, const AppProtect::Copy&);

    stub11.set(ADDR(DmeRestClient, SendRequest), StubSendRequestSuccess);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringSec);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringParentSec);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        &StubGetValueInt32);
    stub11.set(ADDR(CMpTime, DoSleep), SleepStub);
    stub11.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);

    AppProtect::ActionResult result;
    const AppProtect::Copy image;
    std::string jobId;
    g_funcCalled = false;
    auto handler = std::make_shared<JobServiceHandler>();
    handler->ReportCopyAdditionalInfo(result, jobId, image);
    EXPECT_EQ(g_funcCalled, true);
}

/*
* 用例名称：JobServiceHandler根据一致性哈希环分配文件到文件系统
* 前置条件：1、获取JobServiceHandler对象
* check点：1、执行2次ComputerFileLocationInMultiFileSystem，在输入一致的情况下，结果一致
*/
TEST_F(JobServerTest, JobServiceHandler_ComputerFileLocationInMultiFileSystem)
{
    DO_LOG_TEST();
    std::vector<std::string> vecFile = { "/tmp/file1", "/tmp/file2", "/tmp/file3" };
    std::vector<std::string> vecFileSystem = { "/share1", "/share2", "/share3", "/share4" };
    auto handler = std::make_shared<JobServiceHandler>();
    std::map<std::string, std::string> mapRet1, mapRet2;
    handler->ComputerFileLocationInMultiFileSystem(mapRet1, vecFile, vecFileSystem);
    EXPECT_EQ(vecFile.size(), mapRet1.size());
    handler->ComputerFileLocationInMultiFileSystem(mapRet2, vecFile, vecFileSystem);
    EXPECT_EQ(vecFile.size(), mapRet2.size());

    for (auto strFile : vecFile) {
        EXPECT_EQ(true, mapRet1[strFile] == mapRet2[strFile]);
    }
}

mp_int32 StubSendRequestFailed(mp_void* pThis, const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse)
{
    httpResponse.statusCode = 400;
    g_funcCalled = true;
    Json::Value value;
    value["errorCode"] = "1000";
    value["errorMessage"] = "xxxx";
    httpResponse.body = value.toStyledString();
    return MP_SUCCESS;
}

/*
 * 用例名称：测试获取上一个copy的信息
 * 前置条件：1、mock log 2、mock CConfigXmlParser函数
 * check点：1、检查QueryPreviousCopy异常分支后的异常信息
 */
TEST_F(JobServerTest, JobServiceHandler_call_RollbackDataAfterBackupFailed_dispatchor_to_QueryPreviousCopy_FAILED)
{
    DO_LOG_TEST();
    stub11.set(ADDR(DmeRestClient, SendRequest), StubSendRequestFailed);
    stub11.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringSec);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringParentSec);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        &StubGetValueInt32);

    AppProtect::Copy image;
    AppProtect::Application application;

    mp_int32 errorCode = 0;
    std::set<CopyDataType::type> types = { CopyDataType::INCREMENT_COPY };
    auto handler = std::make_shared<JobServiceHandler>();
    try {
        handler->QueryPreviousCopy(image, application, types, "", "mainId");
    } catch (const AppProtect::AppProtectFrameworkException& e) {
        errorCode = e.code;
    }

    EXPECT_EQ(1000, errorCode);
}

/*
 * 用例名称：测试获取上一个copy的信息
 * 前置条件：1、mock log 2、mock CConfigXmlParser函数
 * check点：1、检查QueryPreviousCopy的正常分支
 */
TEST_F(JobServerTest, JobServiceHandler_call_RollbackDataAfterBackupFailed_dispatchor_to_QueryPreviousCopy_OK)
{
    DO_LOG_TEST();
    stub11.set(ADDR(DmeRestClient, SendRequest), StubSendRequestFailed);
    stub11.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringSec);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringParentSec);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        &StubGetValueInt32);

    AppProtect::Copy image;
    AppProtect::Application application;

    mp_int32 errorCode = 0;
    std::set<CopyDataType::type> types = { CopyDataType::INCREMENT_COPY };
    auto handler = std::make_shared<JobServiceHandler>();
    try {
        handler->QueryPreviousCopy(image, application, types, "", "mainId");
    } catch (const AppProtect::AppProtectFrameworkException& e) {
        errorCode = e.code;
    }

    EXPECT_EQ(errorCode, 1000);
}

/*
 * 用例名称：测试处理Rest请求响应的状态码
 * 前置条件：1、mock log 2、mock CConfigXmlParser函数
 * check点：1、检查返回值是否为响应的状态码
 */

TEST_F(JobServerTest, JobServiceHandler_test_rest_status_code_OK)
{
    DO_LOG_TEST();
    stub11.set(ADDR(DmeRestClient, SendRequest), StubSendRequestFailed);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringSec);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        &StubGetValueStringParentSec);
    stub11.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        &StubGetValueInt32);
    stub11.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobId);

    AppProtect::ActionResult result;
    std::vector<AppProtect::SubJob> subjobs;
    AppProtect::SubJob job;
    job.jobId = "123456";
    job.jobName = "test";
    job.policy = ExecutePolicy::ANY_NODE;
    job.jobInfo = "test params";
    subjobs.push_back(job);
    auto handler = std::make_shared<JobServiceHandler>();
    handler->AddNewJob(result, subjobs);

    EXPECT_EQ(200, result.code);
}

TEST_F(JobServerTest, JobServer_RegisterRpcObserver)
{
    DO_LOG_TEST();
    JobServer jobServer;
    stub11.set(&JobServiceFactory::GetInstance, StubJobServiceFactoryGetInstance);
    stub11.set(&JobServiceFactory::CreateObserver, StubCreateObserver);
    stub11.set(&ExternalPluginManager::RegisterObserver, StubVoid);
    jobServer.RegisterRpcObserver();
}

TEST_F(JobServerTest, JobServer_Uninitailize)
{
    DO_LOG_TEST();
    JobServer jobServer;
    bool bRet = jobServer.Uninitailize();
    EXPECT_EQ(true, bRet);
}