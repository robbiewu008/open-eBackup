#include "taskmanager/AppProtectJobHandlerTest.h"
#include "apps/appprotect/plugininterface/ApplicationProtectBaseDataType_types.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "servicecenter/servicefactory/include/ServiceFactory.h"
#include "thriftservice/include/IThriftService.h"
#include "apps/appprotect/CommonDef.h"
#include "taskmanager/externaljob/PluginMainJob.h"
#include "taskmanager/externaljob/PluginJobFactory.h"
#include "taskmanager/externaljob/JobStateDB.h"
#include "taskmanager/externaljob/Job.h"
#include "message/curlclient/DmeRestClient.h"
#include "taskmanager/externaljob/PluginSubGeneJob.h"
#include "taskmanager/externaljob/PluginSubPrepJob.h"
#include "apps/appprotect/plugininterface/ApplicationProtectPlugin_types.h"
#include "apps/appprotect/plugininterface/ProtectService.h"
#include "taskmanager/externaljob/ReportJobDetailFactory.h"
#include "taskmanager/externaljob/PluginSubBusiJob.h"
#include "pluginfx/ExternalPluginParse.h"
#include "pluginfx/ExternalPluginManager.h"
#include "servicecenter/thriftservice/detail/ThriftClient.h"
#include "message/tcp/CSocket.h"
#include "common/Ip.h"
#include "common/ConfigXmlParse.h"
#include "securecom/RootCaller.h"
#include "taskmanager/externaljob/ClearMountPointsJob.h"
#include "alarm/AlarmMgr.h"
#include "taskmanager/externaljob/JobTimer.h"
#include "host/host.h"

using namespace servicecenter;
using namespace thriftservice;
using namespace AppProtect;
namespace {
mp_void LogTest()
{}
#define DoLogTest()                                                                                                    \
    do {                                                                                                               \
        stub.set(ADDR(CLogger, Log), LogTest);                                                                         \
    } while (0)
    
mp_int32 StubGetHostSNSuccess(void *obj, mp_string& strSN)
{
    mp_string strTmp = {"123456"};
    strSN = strTmp;
    return MP_SUCCESS;
}

mp_int32 UpdateRoleSuccess(const mp_string &nodeId, mp_int32 &role)
{
    role = 1;
    return MP_SUCCESS;
}

mp_int32 ExecAllowBackupInLocalNodeSuccess(const BackupLimit::type policy)
{
    return MP_SUCCESS;
}


void StubApplicationServiceNormalCall(void (ApplicationServiceIf::*DiscoverHostCluster)(ApplicationEnvironment&, ApplicationEnvironment),ApplicationEnvironment &ret, ApplicationEnvironment in)
{
    ApplicationEnvironment node;
    node.id = "123456";
    node.name = "123456";
    node.extendInfo = "{\"role\":1}";
    ret.nodes.push_back(node);
}

static mp_string g_subTaskIdStub = mp_string("sub-123456789");
static mp_string g_mainTaskIdStub = mp_string("main-123456789");
static mp_string g_mainJobParamsStub =  mp_string("main-ParamStub");
static mp_int32 g_subJobTypeStub = (mp_int32)SubJobType::type::PRE_SUB_JOB;
static mp_int32 g_mainJobTypeStub = (mp_int32)AppProtect::MainJobType::BACKUP_JOB;
static mp_string g_appInfoType = mp_string("InfoType");
static std::vector<mp_string> g_ipList = { "127.0.0.1, 127.0.0.2" };
class MockJob : public AppProtect::Job {
public:
    MockJob(const PluginJobData& data) : Job(data) {}

    void NotifyJobDetail(const AppProtect::SubJobDetails& jobDetails) override
    {
    }
    bool IsCompleted() override
    {
        return true;
    }

    mp_int32 Initialize() override
    {
        return MP_SUCCESS;
    }

    mp_int32 CanbeRunInLocalNode() override
    {
        return MP_FAILED;
    }

    bool IsFailed() override
    {
        return false;
    }
};

std::shared_ptr<Job> CreatePluginJobStub(void* obj, const PluginJobData& data)
{
    auto pJob = std::make_shared<MockJob>(data);
    return pJob;
}

mp_int32 DeleteRecordStub(void* obj, const mp_string& mainId, const mp_string& subId)
{
    return MP_SUCCESS;
}

mp_int32 SuccStub(void* obj)
{
    return MP_SUCCESS;
}

mp_int32 StubParseRspDoradoIP(void* obj, const Json::Value &value, std::multimap<mp_string, std::vector<mp_string>>& doradoIp)
{
    std::string remotePath = "/KubernetesCommon_e1f54a04-0eb6-3e0b-990b-7a24fdf59ca4/source_policy_e1f54a04-0eb6-3e0b-990b-7a24fdf59ca4_Context_Global_MD";
    std::vector<mp_string> hostList;
    hostList.push_back("192.168.100.100");
    doradoIp.insert(make_pair(remotePath, hostList));
    return MP_SUCCESS;
}

mp_int32 StubFailed(void* obj)
{
    return MP_FAILED;
}

mp_void SuccClearMountPointsStub()
{
    return;
}

bool ReturnTrueStub(void* obj)
{
    return true;
}

bool ReturnFalseStub(void* obj)
{
    return false;
}

mp_int32 QueryRecordStub(void* obj, std::vector<PluginJobData>& result)
{
    PluginJobData main;
    result.push_back(main);
    return MP_SUCCESS;
}

mp_int32 InsertRecordStub(void* obj, const AppProtect::PluginJobData& mainId)
{
    return MP_SUCCESS;
}

mp_int32 UpdateGenerateTimeStub(void* obj, const mp_string& mainId, const mp_int64 time)
{
    return MP_SUCCESS;
} 

mp_int32 CheckTaskOfPluginSucStub(void* obj, const std::shared_ptr<Job>& job, const MainJobInfoPtr& mainJobInfo)
{
    return MP_SUCCESS;
}

mp_int32 SubcribeSucStub(void* obj, const mp_string& jobID, const mp_string& nodeID, const std::vector<mp_string>& agentIpLists)
{
    return MP_SUCCESS;
}

mp_int32 AcquireJobSucStub(void* obj, const MainJobInfoPtr& mainJob, const mp_string& nodeID, std::vector<std::shared_ptr<Job>>& jobs)
{
    PluginJobData data;
    data.mainID = g_mainTaskIdStub;
    std::shared_ptr<Job> job = std::make_shared<MockJob>(data);
    jobs.push_back(job); 
    return MP_SUCCESS;
}

mp_int32 ParseRspDoradoIPStubSucc(void* obj, const Json::Value& value, std::vector<mp_string>& doradoIp)
{
    return MP_SUCCESS;
}


mp_int32 GetRotationTimeStub(void* obj)
{
    return 0;
}

mp_int32 DMEClientRestUpdateSecureInfoStubSuc(void* obj)
{
    return MP_SUCCESS;
}

mp_int32 CheckIpConnectionSucStub(void* obj, const mp_string& inIplist, std::vector<mp_string>& outLists)
{
    return MP_SUCCESS;
}

static bool g_reported = false;

mp_int32 ReportStub(mp_void* pThis, const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse)
{
    g_reported = true;
    httpResponse.statusCode = SC_OK;
    return MP_SUCCESS;
}

mp_int32 StubGetUbcIpsByMainJobId(mp_void* pThis, const mp_string mainJobId, std::vector<mp_string>& ubcIps)
{
    return MP_SUCCESS;
}

static mp_int32 g_reportTime = 0;
mp_int32 SendRequetStub(mp_void* pThis, const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse)
{
    ++g_reportTime;
    httpResponse.statusCode = SC_OK;
    return MP_SUCCESS;
}

mp_int32 StubSendRequetError(mp_void* pThis, const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse)
{
    httpResponse.body = "{ \"errorCode\" : \"10086\", \"errorMessage\" : \"errorMessage\" }";
    httpResponse.statusCode = 400;
    return MP_SUCCESS;
}

mp_int32 StubStatisticInfo(mp_void* pThis, const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse)
{
    httpResponse.body = "[{\"id\":\"id1\",\"ip\":\"ip1\",\"tasks\":{\"main\":0,\"sub\":0}},{\"id\":\"id2\",\"ip\":\"ip2\",\"tasks\":{\"main\":0,\"sub\":0}}]";
    httpResponse.statusCode = SC_OK;
    return MP_SUCCESS;
}

bool MainJobTimeoutStub()
{
    return true;
}

mp_void UpdateDmeAddrStub(const std::vector<mp_string> &dmeIpList, mp_uint32 dmePort)
{
    return;
}

mp_int32 MountNasStub(void* obj)
{
    return MP_SUCCESS;
}

mp_int32 SplitRepositoriesStub(void* obj)
{
    return MP_SUCCESS;
}

mp_int32 QueryJobStub(void* obj, const mp_string& mainId, const mp_string& subId, PluginJobData& jobData)
{
    return MP_SUCCESS;
}

mp_uint32 StopPluginStub()
{
    return MP_SUCCESS;
}

mp_void StubDoSleep(mp_uint32 ms)
{
}

static bool g_pushJop = false;
mp_void StubPushJob(void* obj, std::shared_ptr<Job> pJob)
{
    g_pushJop = true;
}

std::vector<std::shared_ptr<Job>> StubAcquireRunningJob(void* obj, const mp_string& mainID)
{
    std::vector<std::shared_ptr<Job>> jobs;
    PluginJobData jobData = {"pluginName", mainID, "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> job = std::make_shared<AppProtect::PluginMainJob>(jobData);
    jobs.push_back(job);
    return jobs;
}

void StubAddAcquireInfo(void* obj) 
{
}

mp_int32 StubParseRsp(mp_void* pThist, const mp_string& respParam, std::vector<std::shared_ptr<Job>>& jobs)
{
    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    jobs.push_back(pJob);
    return MP_SUCCESS;
}

mp_int32 StubQueryAllJob(mp_void* pThist, std::vector<PluginJobData>& result)
{
    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    result.push_back(jobData);
    return MP_SUCCESS;
}

mp_int32 StubGetValidIpList(std::vector<mp_string>& ipv4List, std::vector<mp_string>& ipv6List)
{
    ipv4List = {"8.40.1.1", "8.40.1.2", "8.40.1.3"};
    ipv6List = {};
    return MP_SUCCESS;
}

mp_int32 StubCheckHostLinkStatus(const mp_string& strSrcIp, const mp_string& strHostIp, mp_uint16 uiPort, mp_int32 timeout=300)
{
    return strSrcIp == "8.40.1.2" ? MP_SUCCESS : MP_FAILED;
}

mp_int32 StubGetIpList(void* obj, const std::set<mp_string>& doradoIps,
    std::vector<mp_string>& validLocalIps, std::vector<mp_string>& validDoradoIps)
{
    validLocalIps.push_back("8.40.1.2");
    validDoradoIps.push_back("8.40.1.2");
    return MP_SUCCESS;
}

mp_int32 StubGetIpListSuc(void* obj, const std::set<mp_string>& doradoIps,
    std::vector<mp_string>& validLocalIps, std::vector<mp_string>& validDoradoIps)
{
    validLocalIps.push_back("8.40.1.2");
    validDoradoIps.push_back("8.40.1.2");
    validDoradoIps.push_back("8.40.1.3");
    return MP_SUCCESS;
}

mp_int32 StubGetIpListFail(void* obj, const std::set<mp_string>& doradoIps,
    std::vector<mp_string>& validLocalIps, std::vector<mp_string>& validDoradoIps)
{
    validDoradoIps.clear();
    validLocalIps.push_back("8.40.1.2");
    return MP_SUCCESS;
}

mp_void StubSendEvent(const mp_string &eventId, bool isSuccess, const mp_string &param)
{
    return;
}

mp_int32 StubJudgeMainJobCnt()
{
    return MP_SUCCESS;
}

mp_int32 StubGetHostSN(void* obj, mp_string& strSN)
{
    strSN = "123";
    return MP_SUCCESS;
}

mp_int32 StubGetValueInt32(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    if (strSection == CFG_FRAME_THRIFT_SECTION) {
        if (strKey == MAIN_JOB_CNT_MAX_INNER) {
            iValue = 1;
        } else if (strKey == MAIN_JOB_CNT_MAX) {
            iValue = 1;
        } else if (strKey == SUB_JOB_CNT_MAX_INNER) {
            iValue = 1;
        } else if (strKey == SUB_JOB_CNT_MAX) {
            iValue = 1;
        }
    } 
    return MP_SUCCESS;
}

mp_int32 ExecTesta(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    if (iCommandID == (mp_int32)ROOT_COMMAND_CAT) {
        if (strParam == "/etc/hosts") {
            pvecResult->push_back("172.18.128.1 nas.storage.protectengine_a.host.1");
        }
    }
    return MP_SUCCESS;
}

}  // namespace

void AppProtectJobHandlerTest::SetUp()
{
    stub.set(ADDR(AppProtect::AppProtectJobHandler, GetRotationTime), GetRotationTimeStub);
    stub.set((mp_int32(JobStateDB::*)(const mp_string&))ADDR(AppProtect::JobStateDB, DeleteRecord), DeleteRecordStub);
    stub.set(ADDR(AppProtect::JobStateDB, InsertRecord), InsertRecordStub);

    // constrcut acquire body rsp
    {
        Json::Value JobItem;
        {
            // construct mainJob;
            JobItem[TASKID] = g_mainTaskIdStub;
            JobItem[TASKTYPE] = g_mainJobTypeStub;
            JobItem[SUBTASKID] = "";
            JobItem[SUBTASKTYPE] = g_subJobTypeStub;
            // construct app info
            Application appInfo;
            appInfo.type = g_appInfoType;
            Json::Value appInfoJsonValue;
            StructToJson(appInfo, appInfoJsonValue);
            JobItem[APPINFO] = appInfoJsonValue;
            Copy copy;
            Json::Value copyJsonValue;
            StructToJson(copy, copyJsonValue);
            JobItem[COPYINFO] = copyJsonValue;
        }
        m_root.append(JobItem);
    }
    {
        Json::Value JobItem;
        {
            // construct subJob;
            JobItem[TASKID] = g_mainTaskIdStub;
            JobItem[TASKTYPE] = g_mainJobTypeStub;
            JobItem[SUBTASKID] = g_subTaskIdStub;
            JobItem[SUBTASKTYPE] = g_subJobTypeStub;
            // construct app info
            Application appInfo;
            appInfo.type = g_appInfoType;
            Json::Value appInfoJsonValue;
            StructToJson(appInfo, appInfoJsonValue);
            JobItem[APPINFO] = appInfoJsonValue;
            Copy copy;
            Json::Value copyJsonValue;
            StructToJson(copy, copyJsonValue);
            JobItem[COPYINFO] = copyJsonValue;
        }
        m_root.append(JobItem);
    }
}

void AppProtectJobHandlerTest::TearDown()
{
    stub.reset((mp_int32(JobStateDB::*)(const mp_string&))ADDR(AppProtect::JobStateDB, DeleteRecord));
    stub.reset(ADDR(AppProtect::JobStateDB, InsertRecord));
}

class  SendRequestMock {
public:
    mp_int32 SendRequestSucStub(const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse)
    {
        httpResponse.statusCode = SC_OK;
        Json::Value root;
        // constrcut acquire body rsp
        {
            Json::Value JobItem;
            JobItem[TASKID] = g_mainTaskIdStub;
            JobItem[TASKTYPE] = g_mainJobTypeStub;
            JobItem[SUBTASKID] = "";
            JobItem[SUBTASKTYPE] = -1;
            JobItem[TASKTYPE] = mp_int32(MainJobType::BACKUP_JOB);
            {
                Application appInfo;
                appInfo.type = g_appInfoType;
                Json::Value appInfoJsonValue;
                StructToJson(appInfo, appInfoJsonValue);
                JobItem[APPINFO] = appInfoJsonValue;
            }
            {
                Json::Value repJosnValueArray;
                Json::Value repJosnValue;
                StorageRepository res;
                HostAddress host;
                host.ip = std::string("127.0.0.1");
                res.remoteHost.push_back(host);
                StructToJson(res, repJosnValue);
                repJosnValueArray.append(repJosnValue);
                JobItem[REPOSITORIES] = repJosnValueArray;
            }
            root.append(JobItem);
        }
        httpResponse.body = root.toStyledString();
        return MP_SUCCESS;
    }
    mp_uint32 GetHttpStatusCode()
    {
        return 200;
    }
};

void AppProtectJobHandlerTest::SetUpTestCase()
{

}

void AppProtectJobHandlerTest::TearDownTestCase()
{

}

mp_int32 DmeClientUpdateSecureInfoSucStub()
{
    return MP_SUCCESS;
}

static int32_t g_execCount = 0;
class ProtectServiceClientSub : public ProtectServiceClient {
public:
    ProtectServiceClientSub(std::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
        ProtectServiceClient(prot) {}

    void AllowBackupInLocalNode(ActionResult& _return, const BackupJob& job, const BackupLimit::type limit) override
    {
        _return.code = MP_SUCCESS;
    }

    void PauseJob(ActionResult& _return, const std::string& jobId, const std::string& subJobId, const std::string& appType) override
    {
        ++g_execCount;
        _return.code = MP_SUCCESS;
    }

    void CheckBackupJobType(ActionResult& _return, const BackupJob& job) override
    {
        _return.__set_code(MP_FAILED);
        _return.__set_bodyErr(ERR_INC_TO_FULL);
    }

    void AsyncBackupPrerequisite(ActionResult& _return, const BackupJob& job) override
    {
        _return.code = MP_SUCCESS;
    }

    void AsyncBackupGenerateSubJob(ActionResult& _return, const BackupJob& job, const int32_t nodeNum) override
    {
        _return.code = MP_SUCCESS;
    }
};

std::shared_ptr<thriftservice::IThriftClient> GetThriftClientStub()
{
    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    thriftservice::ClientSocketOpt opt = { "127.0.0.1", 59610 };
    std::shared_ptr<thriftservice::IThriftClient> thriftclient = thriftservice->RegisterClient(opt);
    return thriftclient;
}

static std::shared_ptr<ProtectServiceIf> g_ProtectServiceClient;
static std::shared_ptr<ProtectServiceIf> StubAllowGetThriftClient(mp_void* pThis, std::shared_ptr<thriftservice::IThriftClient> pThriftClient)
{
    if (g_ProtectServiceClient.get() == nullptr) {
        g_ProtectServiceClient = std::make_shared<ProtectServiceClientSub>(nullptr);
    }
    return g_ProtectServiceClient;
}

class ProtectServiceClientCallFailedTestStub : public ProtectServiceClient {
public:
    ProtectServiceClientCallFailedTestStub(std::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
        ProtectServiceClient(prot) {}

    void CheckBackupJobType(ActionResult& _return, const BackupJob& job) override
    {
        _return.code = MP_FAILED;
        _return.bodyErr = ERR_INC_TO_FULL;
    }
};

static std::shared_ptr<ProtectServiceClient> g_ProtectServiceFailedClient;
static std::shared_ptr<ProtectServiceClient> StubAllowGetThriftClientAndPluginReturnFailed()
{
    if (g_ProtectServiceFailedClient.get() == nullptr) {
        g_ProtectServiceFailedClient = std::make_shared<ProtectServiceClientCallFailedTestStub>(nullptr);
    }
    return g_ProtectServiceFailedClient;
}

mp_int32 StubRestSendResponseFailed(mp_void* pThis, const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse)
{
    httpResponse.statusCode = SC_OK;
    Json::Value rspValue;
    rspValue["errorCode"] = "1";
    httpResponse.body = rspValue.toStyledString();
    return MP_SUCCESS;
}

static bool g_call = false;
mp_int32 StubRestSendResponse(mp_void* pThis, const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse)
{
    httpResponse.statusCode = SC_OK;
    Json::Value rspValue;
    rspValue["errorCode"] = "0";
    httpResponse.body = rspValue.toStyledString();
    g_call = true;
    return MP_SUCCESS;
}

mp_int32 StubCanbeRunInLocalNode()
{
    return MP_SUCCESS;
}

mp_int32 StubCheckIsDoradoEnvironmentSuccess(void *obj, mp_bool& isDorado)
{
    isDorado = true;
    return MP_SUCCESS;
}

mp_int32 StubTestCheckIsDoradoEnvironmentSuccess(mp_bool& isDorado)
{
    isDorado = true;
    return MP_SUCCESS;
}

mp_int32 StubGetDoraDoLanNet(void *obj, mp_string& net)
{
    return MP_SUCCESS;
}

/*
 * 用例名称：初始化AppProtectJobHandler
 * 前置条件：NA
 * check点：初始化AppProtectJobHandler成功
 */
TEST_F(AppProtectJobHandlerTest, InitializeTest)
{
    DoLogTest();
    stub.set(ADDR(DmeRestClient, UpdateSecureInfo), DmeClientUpdateSecureInfoSucStub);
    stub.set(&CIP::CheckIsDoradoEnvironment, StubTestCheckIsDoradoEnvironmentSuccess);
    stub.set(ADDR(AppProtectJobHandler, GetDoraDoLanNet), StubGetDoraDoLanNet);
    stub.set(ADDR(CHost, GetContainerIPList), SuccStub);
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    EXPECT_NE(appProtectJobHandler, nullptr);
}

/*
 * 用例名称：AppProtectJobHandler获取任务服务
 * 前置条件：NA
 * check点：初始化AppProtectJobHandler 停止获取任-返回成功
 */
TEST_F(AppProtectJobHandlerTest, GetJobSrv_Stop_SUC)
{
    DoLogTest();
    stub.set(ADDR(AppProtectJobHandler, StopGettingJobSrv), SuccStub);
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->StopGettingJobSrv();
}

/*
 * 用例名称：AppProtectJobHandler 任务接口---添加任务
 * 前置条件：NA
 * check点：任务接口---1、任务缓存队列不为空
 *                      2、缓存中的主任务和插入的主任务相等
 */
TEST_F(AppProtectJobHandlerTest, Job_InterFace_TEST)
{
    DoLogTest();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();

    PluginJobData jobData = {"pluginName", "mainID", "subID", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    if (pJob) {
        appProtectJobHandler->m_runJobs.push_back(pJob);
        auto jobs = appProtectJobHandler->GetRunJobs();
        EXPECT_EQ(jobs.size(), 1);
        auto job = appProtectJobHandler->GetRunJobById("mainID", "subID");
        EXPECT_EQ("mainID", job->GetData().mainID);
        appProtectJobHandler->m_runJobs.clear();
    }
}

/*
 * 用例名称：获取后置任务
 * 前置条件：NA
 * check点：任务获取成功，任务信息不为空
 */
TEST_F(AppProtectJobHandlerTest, GetPostJob_TEST)
{
    DoLogTest();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();

    PluginJobData jobData = {"pluginName", "mainID", "subID", Json::Value(), AppProtect::MainJobType::BACKUP_JOB,
        SubJobType::type::POST_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    if (pJob) {
        appProtectJobHandler->m_runJobs.push_back(pJob);
        auto jobs = appProtectJobHandler->GetRunJobs();
        EXPECT_EQ(jobs.size(), 1);
        auto job = appProtectJobHandler->GetPostJobByMainId("mainID");
        EXPECT_EQ("mainID", job->GetData().mainID);
        appProtectJobHandler->m_runJobs.clear();
    }
}

/*
 * 用例名称：AppProtectJobHandler 解析存储IP列表
 * 前置条件：NA
 * check点：1、解析的IP列表和设置的相等
 */
TEST_F(AppProtectJobHandlerTest, ParseRspDoradoIP_From_Rep_TEST)
{
    DoLogTest();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();

    Json::Value JobItem;
    {
        // construct mainJob;
        JobItem[TASKID] = g_mainTaskIdStub;
        JobItem[TASKTYPE] = g_mainJobTypeStub;
        JobItem[SUBTASKID] = "";
        JobItem[SUBTASKTYPE] = -1;
        // construct app info
        Application appInfo;
        appInfo.type = g_appInfoType;
        Json::Value appInfoJsonValue;
        StructToJson(appInfo, appInfoJsonValue);
        JobItem[APPINFO] = appInfoJsonValue;
        // construct StorageRepository
        Json::Value repJosnValueArray;
        {
            Json::Value repJosnValue;
            {
                StorageRepository res;
                HostAddress host;
                host.ip = std::string("127.0.0.1");
                res.remoteHost.push_back(host);
                StructToJson(res, repJosnValue);
            }
            repJosnValueArray.append(repJosnValue);
        }

        JobItem[REPOSITORIES] = repJosnValueArray;
    }
    std::multimap<mp_string, std::vector<mp_string>> lists;
    appProtectJobHandler->ParseRspDoradoIP(JobItem, lists);
    EXPECT_EQ(lists.size(), 1);
}

/*
 * 用例名称：AppProtectJobHandler 解析存储IP列表-copyInfo 中获取
 * 前置条件：NA
 * check点：1、解析的IP列表和设置的相等
 */
TEST_F(AppProtectJobHandlerTest, ParseRspDoradoIP_From_CopyInfo_TEST)
{
    DoLogTest();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();

    Json::Value JobItem;
    JobItem[TASKID] = g_mainTaskIdStub;
    JobItem[TASKTYPE] = g_mainJobTypeStub;
    JobItem[SUBTASKID] = "";
    JobItem[SUBTASKTYPE] = -1;
    JobItem[TASKTYPE] = mp_int32(MainJobType::BACKUP_JOB);
    {
        Json::Value repJosnValueArray;
        Json::Value repJosnValue;
        StorageRepository res;
        HostAddress host;
        host.ip = std::string("127.0.0.1");
        res.remoteHost.push_back(host);
        StructToJson(res, repJosnValue);
        repJosnValueArray.append(repJosnValue);
        JobItem[REPOSITORIES] = repJosnValueArray;
    }
    std::multimap<mp_string, std::vector<mp_string>> lists;
    appProtectJobHandler->ParseRspDoradoIP(JobItem, lists);
    EXPECT_EQ(1, lists.size());
}

/*
 * 用例名称：AppProtectJobHandler 解析返回 获取任务
 * 前置条件：NA
 * check点：1、解析的任务参数和设置的相等
 */
TEST_F(AppProtectJobHandlerTest, ParseRsp_SUC)
{
    stub.set(ADDR(PluginJobFactory, CreatePluginJob), CreatePluginJobStub);
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();

    std::vector<std::shared_ptr<AppProtect::Job>> jobs;
    EXPECT_EQ(MP_SUCCESS, appProtectJobHandler->ParseRsp(m_root.toStyledString(), jobs));
    EXPECT_EQ(2, jobs.size());
    stub.reset(ADDR(PluginJobFactory, CreatePluginJob));
}

/*
 * 用例名称：AppProtectJobHandler获取任务-解析rsp失败
 * 前置条件：获取任务服务启动
 * check点：查看返回值失败，或者任务为空
 */
TEST_F(AppProtectJobHandlerTest, ParseRsp_Fail)
{
    DoLogTest();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    std::vector<std::shared_ptr<AppProtect::Job>> jobs;
    std::string emptyRsp;
    mp_int32 iRet = appProtectJobHandler->ParseRsp(emptyRsp, jobs);
    EXPECT_TRUE(jobs.empty());
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：AppProtectJobHandler获取任务-唤醒成功
 * 前置条件：获取任务服务启动
 * check点：查看任务缓存中 任务信息和打桩信息相等
 */
TEST_F(AppProtectJobHandlerTest, WakeUpJob_SUCC)
{
    DoLogTest();
    stub.set(ADDR(DmeRestClient, UpdateSecureInfo), DmeClientUpdateSecureInfoSucStub);
    stub.set(ADDR(DmeRestClient, UpdateDmeAddr), UpdateDmeAddrStub);

    mp_string taskid("test");
    mp_string ip1("127.0.0.1");
    mp_string ip2("127.0.0.2");
    Json::Value notifyInfo;
    {
        // construct taskid;
        // construct notifyInfo;
        notifyInfo[NOTIFY_APPTYPE] = "";
        Json::Value dmeIpList;
        dmeIpList.append(Json::Value(ip1));
        dmeIpList.append(Json::Value(ip2));
        notifyInfo[NOTIFY_DMEIPLISTS] = dmeIpList;
    }

    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    mp_int32 iRet = appProtectJobHandler->WakeUpJob(mp_string("test"), notifyInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);

    auto allJobInfo = appProtectJobHandler->GetAllAcquireInfo();
    EXPECT_EQ(1, allJobInfo.size());
    stub.reset(ADDR(DmeRestClient, UpdateSecureInfo));
    stub.reset(ADDR(DmeRestClient, UpdateDmeAddr));
}

/*
 * 用例名称：AppProtectJobHandler获取任务-唤醒失败
 * 前置条件：获取任务服务启动
 * check点：查看返回值失败
 */
TEST_F(AppProtectJobHandlerTest, WakeUpJob_Fail)
{
    DoLogTest();
    mp_string taskid("test");
    mp_string ip1("127.0.0.1");
    mp_string ip2("127.0.0.2");
    Json::Value WakeUpJsonRootStub;
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_vecAcquireInfo.clear();
    mp_int32 iRet = appProtectJobHandler->WakeUpJob(mp_string("test"), WakeUpJsonRootStub);
    EXPECT_NE(iRet, MP_SUCCESS);
    auto allJobInfo = appProtectJobHandler->GetAllAcquireInfo();
    EXPECT_EQ(allJobInfo.size(), 0);
}

TEST_F(AppProtectJobHandlerTest, AddAcquireInfo)
{
    DoLogTest();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_vecAcquireInfo.clear();

    PluginJobData jobData = { "apptype", "jobId1", "", Json::Value(), AppProtect::MainJobType::BACKUP_JOB };
    MainJobInfoPtr pInfo = std::make_shared<PluginJobData>(jobData);

    stub.set(ADDR(AppProtectJobHandler, JudgeMainJobCnt), StubFailed);
    EXPECT_EQ(MP_FAILED, appProtectJobHandler->AddAcquireInfo(pInfo));

    stub.set(ADDR(AppProtectJobHandler, JudgeMainJobCnt), SuccStub);
    EXPECT_EQ(MP_SUCCESS, appProtectJobHandler->AddAcquireInfo(pInfo));
    EXPECT_EQ(MP_SUCCESS, appProtectJobHandler->AddAcquireInfo(pInfo));

    appProtectJobHandler->DelAcquireInfo(pInfo);
    EXPECT_EQ(0, appProtectJobHandler->m_vecAcquireInfo.size());
}

TEST_F(AppProtectJobHandlerTest, JudgeMainJobCnt)
{
    DoLogTest();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_runJobs.clear();
    {
        PluginJobData jobData = { "apptype", "jobId1", "", Json::Value(), AppProtect::MainJobType::BACKUP_JOB };
        jobData.status = mp_uint32(MainJobState::GENERATE_JOB_RUNNING);
        appProtectJobHandler->m_runJobs.push_back(std::make_shared<AppProtect::PluginMainJob>(jobData));
    }
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), SuccStub);
    appProtectJobHandler->mIsDorado = MP_TRUE;
    EXPECT_EQ(MP_SUCCESS, appProtectJobHandler->JudgeMainJobCnt());
    appProtectJobHandler->mIsDorado = MP_FALSE;
    EXPECT_EQ(MP_SUCCESS, appProtectJobHandler->JudgeMainJobCnt());

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubGetValueInt32);
    stub.set(ADDR(JobTimer, IsOverInterval), ReturnTrueStub);
    stub.set(ADDR(AlarmMgr, SendEvent), SuccStub);
    EXPECT_EQ(MP_FAILED, appProtectJobHandler->JudgeMainJobCnt());
}

TEST_F(AppProtectJobHandlerTest, JudgeSubJobCnt)
{
    DoLogTest();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_runJobs.clear();
    {
        PluginJobData jobData = { "apptype", "jobId1", "", Json::Value(), AppProtect::MainJobType::BACKUP_JOB };
        jobData.status = mp_uint32(MainJobState::GENERATE_JOB_RUNNING);
        appProtectJobHandler->m_runJobs.push_back(std::make_shared<AppProtect::PluginMainJob>(jobData));
    }
    {
        PluginJobData jobData = { "apptype", "jobId1", "subJobId1", Json::Value(), AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB };
        jobData.status = mp_uint32(SubJobState::Running);
        appProtectJobHandler->m_runJobs.push_back(std::make_shared<AppProtect::PluginSubJob>(jobData));
    }
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubFailed);
    appProtectJobHandler->mIsDorado = MP_TRUE;
    EXPECT_EQ(MP_SUCCESS, appProtectJobHandler->JudgeSubJobCnt("apptype"));
    appProtectJobHandler->mIsDorado = MP_FALSE;
    EXPECT_EQ(MP_SUCCESS, appProtectJobHandler->JudgeSubJobCnt("apptype"));

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubGetValueInt32);
    stub.set(ADDR(JobTimer, IsOverInterval), ReturnTrueStub);
    stub.set(ADDR(AlarmMgr, SendEvent), SuccStub);
    EXPECT_EQ(MP_FAILED, appProtectJobHandler->JudgeSubJobCnt("apptype"));
}

/*
 * 用例名称：AppProtectJobHandler子任务进度上报
 * 前置条件：获取任务服务启动
 * check点：查看子任务上报结果是否OK
 */
TEST_F(AppProtectJobHandlerTest, AppProtectJobHandler_report_sub_job_detail_test)
{
    DoLogTest();
    stub.set(ADDR(DmeRestClient, SendRequest), ReportStub);
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_runJobs.clear();

    PluginJobData jobData = {"pluginName", "123456", "654321", Json::Value(), 
        AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::GENERATE_SUB_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    appProtectJobHandler->m_runJobs.push_back(pJob);

    stub.set(ADDR(AppProtectJobHandler, GetUbcIpsByMainJobId), StubGetUbcIpsByMainJobId);
    AppProtect::ActionResult _return;
    AppProtect::SubJobDetails jobInfo;
    jobInfo.jobId = "123456";
    jobInfo.subJobId = "654321";
    jobInfo.jobStatus = AppProtect::SubJobStatus::COMPLETED;
    g_reported = false;
    mp_int32 iRet = appProtectJobHandler->ReportJobDetails(_return, jobInfo);
    EXPECT_EQ(g_reported, true);
    stub.reset(ADDR(DmeRestClient, SendRequest));
}

TEST_F(AppProtectJobHandlerTest, ReportHeartBeatImpl)
{
    DoLogTest();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();

    stub.set(ADDR(DmeRestClient, SendRequest), StubFailed);
    EXPECT_EQ(true, appProtectJobHandler->ReportHeartBeatImpl());

    stub.set(ADDR(DmeRestClient, SendRequest), ReportStub);
    EXPECT_EQ(true, appProtectJobHandler->ReportHeartBeatImpl());
}

/*
 * 用例名称：AppProtectJobHandler获取任务-AcquireJob 成功
 * 前置条件：获取任务服务启动
 * check点：查看任务缓存中 任务信息和打桩信息相等
 */
/*TEST_F(AppProtectJobHandlerTest, GetJobThreadFunc_SUCC)
{
    DoLogTest();
    stub.set(ADDR(AppProtect::AppProtectJobHandler, Initialize), SuccStub);
    stub.set(ADDR(PluginJobFactory, CreatePluginJob), CreatePluginJobStub);
    stub.set(ADDR(DmeRestClient, SendRequest), ADDR(SendRequestMock, SendRequestSucStub));
    stub.set(ADDR(DmeRestClient, UpdateSecureInfo), DmeClientUpdateSecureInfoSucStub);
    stub.set(ADDR(AppProtectJobHandler, AcquireJob), AcquireJobSucStub);
    stub.set(ADDR(AppProtectJobHandler, ParseRspDoradoIP), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, CheckIpConnection), SuccStub);

    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_runJobs.clear();
    std::shared_ptr<PluginJobData> pJobInfo = std::make_shared<PluginJobData>();
    pJobInfo->mainID = g_mainTaskIdStub;
    pJobInfo->status = 1;
    pJobInfo->dmeIps = g_ipList;
    mp_int32 iRet = appProtectJobHandler->Run(pJobInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);
    auto jobs = appProtectJobHandler->GetRunJobs();
    EXPECT_EQ(0, jobs.size());

    stub.reset(ADDR(AppProtect::AppProtectJobHandler, Initialize));
    stub.reset(ADDR(DmeRestClient, SendRequest));
    stub.reset(ADDR(DmeRestClient, UpdateSecureInfo));
    stub.reset(ADDR(AppProtectJobHandler, ParseRspDoradoIP));
    stub.reset(ADDR(PluginJobFactory, CreatePluginJob));
    stub.reset(ADDR(AppProtectJobHandler, AcquireJob));
}*/

const std::string repositories = "{\
	\"repositories\": [{\
		\"endpoint\": {\
			\"ip\": \"8.40.102.115,8.40.102.116\",\
			\"port\": 8088,\
			\"supportProtocol\": 0\
		},\
		\"extendInfo\": {\
			\"copy_format\": 0,\
			\"esn\": \"2102353GTD10L9000007\",\
			\"fsId\": \"102\"\
		},\
		\"id\": \"\",\
		\"path\": [\"/mnt/databackup/KubernetesNamespaceCommon/4c9facc2-0a12-43d3-aaf6-3cc4bb2d498e/meta/KubernetesCommon_e1f54a04-0eb6-3e0b-990b-7a24fdf59ca4/source_policy_e1f54a04-0eb6-3e0b-990b-7a24fdf59ca4_Context_Global_MD/172.17.128.1\", \"/mnt/databackup/KubernetesNamespaceCommon/4c9facc2-0a12-43d3-aaf6-3cc4bb2d498e/meta/KubernetesCommon_e1f54a04-0eb6-3e0b-990b-7a24fdf59ca4/source_policy_e1f54a04-0eb6-3e0b-990b-7a24fdf59ca4_Context_Global_MD/172.18.128.1\", \"/mnt/databackup/KubernetesNamespaceCommon/4c9facc2-0a12-43d3-aaf6-3cc4bb2d498e/meta/KubernetesCommon_e1f54a04-0eb6-3e0b-990b-7a24fdf59ca4/source_policy_e1f54a04-0eb6-3e0b-990b-7a24fdf59ca4_Context_Global_MD/172.19.128.1\", \"/mnt/databackup/KubernetesNamespaceCommon/4c9facc2-0a12-43d3-aaf6-3cc4bb2d498e/meta/KubernetesCommon_e1f54a04-0eb6-3e0b-990b-7a24fdf59ca4/source_policy_e1f54a04-0eb6-3e0b-990b-7a24fdf59ca4_Context_Global_MD/172.20.128.1\"],\
		\"protocol\": 1,\
		\"remoteHost\": [{\
			\"ip\": \"172.17.128.1\",\
			\"port\": 0,\
			\"supportProtocol\": 0\
		}, {\
			\"ip\": \"172.18.128.1\",\
			\"port\": 0,\
			\"supportProtocol\": 0\
		}, {\
			\"ip\": \"172.19.128.1\",\
			\"port\": 0,\
			\"supportProtocol\": 0\
		}, {\
			\"ip\": \"172.20.128.1\",\
			\"port\": 0,\
			\"supportProtocol\": 0\
		}],\
		\"remoteName\": \"\",\
		\"remotePath\": \"/KubernetesCommon_e1f54a04-0eb6-3e0b-990b-7a24fdf59ca4/source_policy_e1f54a04-0eb6-3e0b-990b-7a24fdf59ca4_Context_Global_MD\",\
		\"repositoryType\": 0,\
		\"role\": 0\
	}]\
}";

TEST_F(AppProtectJobHandlerTest, CheckAndSubcribeJobs)
{
    DoLogTest();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_runJobs.clear();
    Json::Value param;
    JsonHelper::JsonStringToJsonValue(repositories, param);
    std::vector<std::shared_ptr<AppProtect::Job>> vecJobs;
    {
        PluginJobData jobData = { "apptype", "jobId1", "", param, AppProtect::MainJobType::BACKUP_JOB };
        vecJobs.push_back(std::make_shared<AppProtect::PluginMainJob>(jobData));
    }

    stub.set(ADDR(AppProtectJobHandler, JudgeMainJobCnt), StubFailed);
    appProtectJobHandler->CheckAndSubcribeJobs(vecJobs);
    EXPECT_EQ(0, appProtectJobHandler->m_runJobs.size());

    
    stub.set(ADDR(AppProtectJobHandler, ReportCheckFailed), SuccStub);
    stub.set(ADDR(AppProtect::Job, ReplaceRemoteHost), SuccStub);
    stub.set(ADDR(AppProtect::Job, FilerUnvalidDoradoIps), SuccStub);
    stub.set(ADDR(JobPool, PushJob), StubPushJob);

    appProtectJobHandler->mIsDorado = MP_TRUE;
    stub.set(ADDR(AppProtectJobHandler, JudgeMainJobCnt), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, ParseRspDoradoIP), StubParseRspDoradoIP);
    stub.set(ADDR(AppProtectJobHandler, CheckIpConnection), StubFailed);
    appProtectJobHandler->CheckAndSubcribeJobs(vecJobs);
    EXPECT_EQ(0, appProtectJobHandler->m_runJobs.size());
    appProtectJobHandler->mIsDorado = MP_FALSE;

    stub.set(ADDR(AppProtectJobHandler, JudgeMainJobCnt), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, ParseRspDoradoIP), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, CheckIpConnection), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, AllowSubcribe), StubFailed);
    appProtectJobHandler->CheckAndSubcribeJobs(vecJobs);
    EXPECT_EQ(0, appProtectJobHandler->m_runJobs.size());

    stub.set(ADDR(AppProtectJobHandler, JudgeMainJobCnt), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, ParseRspDoradoIP), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, CheckIpConnection), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, AllowSubcribe), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, CheckCanBeRunInLocal), StubFailed);
    appProtectJobHandler->CheckAndSubcribeJobs(vecJobs);
    EXPECT_EQ(0, appProtectJobHandler->m_runJobs.size());

    stub.set(ADDR(AppProtectJobHandler, JudgeMainJobCnt), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, ParseRspDoradoIP), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, CheckIpConnection), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, AllowSubcribe), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, CheckCanBeRunInLocal), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, Subcribe), StubFailed);
    appProtectJobHandler->CheckAndSubcribeJobs(vecJobs);
    EXPECT_EQ(0, appProtectJobHandler->m_runJobs.size());

    stub.set(ADDR(AppProtectJobHandler, JudgeMainJobCnt), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, ParseRspDoradoIP), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, CheckIpConnection), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, AllowSubcribe), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, CheckCanBeRunInLocal), SuccStub);
    stub.set(ADDR(AppProtectJobHandler, Subcribe), SuccStub);
    appProtectJobHandler->CheckAndSubcribeJobs(vecJobs);
    EXPECT_EQ(1, appProtectJobHandler->m_runJobs.size());

    {
        vecJobs.clear();
        PluginJobData jobData = { "apptype", "jobId1", "", Json::Value(), AppProtect::MainJobType::BACKUP_JOB };
        Json::Value jAgent;
        jobData.param["agents"].append(jAgent);
        vecJobs.push_back(std::make_shared<AppProtect::PluginMainJob>(jobData));
        appProtectJobHandler->m_vecAcquireInfo.push_back(std::make_shared<PluginJobData>(jobData));
    }
    appProtectJobHandler->CheckAndSubcribeJobs(vecJobs);
    EXPECT_EQ(2, appProtectJobHandler->m_runJobs.size());
}

TEST_F(AppProtectJobHandlerTest, CheckCanBeRunInLocal)
{
    DoLogTest();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    PluginJobData jobData = { "apptype", "jobId1", "", Json::Value(), AppProtect::MainJobType::BACKUP_JOB };
    appProtectJobHandler->m_vecAcquireInfo.push_back(std::make_shared<PluginJobData>(jobData));
    auto pJob = std::make_shared<AppProtect::PluginMainJob>(jobData);

    EXPECT_EQ(ERR_PLUGIN_AUTHENTICATION_FAILED, appProtectJobHandler->CheckCanBeRunInLocal(nullptr));

    stub.set(ADDR(Job, GetThriftClient), GetThriftClientStub);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    EXPECT_EQ(MP_SUCCESS, appProtectJobHandler->CheckCanBeRunInLocal(pJob));
    appProtectJobHandler->m_vecAcquireInfo.clear();
}

TEST_F(AppProtectJobHandlerTest, Subcribe)
{
    DoLogTest();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    mp_string jobID = "jobID";
    mp_string nodeID = "nodeID";
    PluginJobData data;
    data.mainID = jobID;
    std::shared_ptr<Job> job = std::make_shared<MockJob>(data);
    std::vector<mp_string> agentIpLists;

    stub.set(ADDR(DmeRestClient, SendRequest), StubFailed);
    EXPECT_EQ(MP_FAILED, appProtectJobHandler->Subcribe(job, nodeID, agentIpLists));

    stub.set(ADDR(DmeRestClient, SendRequest), StubSendRequetError);
    EXPECT_EQ(10086, appProtectJobHandler->Subcribe(job, nodeID, agentIpLists));

    stub.set(ADDR(DmeRestClient, SendRequest), SendRequetStub);
    EXPECT_EQ(MP_SUCCESS, appProtectJobHandler->Subcribe(job, nodeID, agentIpLists));

    agentIpLists.push_back("192.168.1.1");
    EXPECT_EQ(MP_SUCCESS, appProtectJobHandler->Subcribe(job, nodeID, agentIpLists));

    appProtectJobHandler->m_netplanInfo = "192.168.1.1";
    EXPECT_EQ(MP_SUCCESS, appProtectJobHandler->Subcribe(job, nodeID, agentIpLists));
}

TEST_F(AppProtectJobHandlerTest, GetDoraDoLanNet)
{
    DoLogTest();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    mp_string net;

    //stub.set(ADDR(CRootCaller, Exec), StubFailed);
    //EXPECT_EQ(MP_FAILED, appProtectJobHandler->GetDoraDoLanNet(net));
    //stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    EXPECT_EQ(MP_FAILED, appProtectJobHandler->GetDoraDoLanNet(net));
    //EXPECT_EQ("172.0.0.0/8", net);
}

TEST_F(AppProtectJobHandlerTest, NotifyPluginReload)
{
    DoLogTest();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_runJobs.clear();

    PluginJobData jobData = { "apptype", "jobId1", "subJobId1", Json::Value(), AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB };
    jobData.status = mp_uint32(SubJobState::Running);
    appProtectJobHandler->m_runJobs.push_back(std::make_shared<AppProtect::PluginSubJob>(jobData));


    stub.set(ADDR(JobPool, PushJob), StubPushJob);
    mp_string pluginName;
    mp_string newPluginPID;
    appProtectJobHandler->NotifyPluginReload(pluginName, newPluginPID);
    newPluginPID = "10086";
    appProtectJobHandler->NotifyPluginReload(pluginName, newPluginPID);
}

/*
 * 用例名称：AppProtectJobHandler获取任务-AcquireJob 成功
 * 前置条件：获取任务服务启动
 * check点：查看任务缓存中 任务信息和打桩信息相等
 */
TEST_F(AppProtectJobHandlerTest, AcquireJob_SUCC)
{
    DoLogTest();
    stub.set(ADDR(AppProtect::AppProtectJobHandler, Initialize), SuccStub);
    stub.set(ADDR(DmeRestClient, SendRequest), ADDR(SendRequestMock, SendRequestSucStub));
    stub.set(ADDR(PluginJobFactory, CreatePluginJob), CreatePluginJobStub);

    g_mainTaskIdStub = "aaaaaaaaaaaaa";
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    MainJobInfoPtr mainInfo = std::make_shared<PluginJobData>();
    mainInfo->mainID = g_mainTaskIdStub;
    mainInfo->dmeIps = g_ipList;
    std::vector<std::shared_ptr<Job>> jobs;
    mp_int32 iRet = appProtectJobHandler->AcquireJob(mainInfo, "aaa", jobs);
    EXPECT_EQ(MP_SUCCESS, iRet);
    EXPECT_EQ(1, jobs.size());
    g_mainTaskIdStub = "main-123456789";

    stub.reset(ADDR(DmeRestClient, SendRequest));
    stub.reset(ADDR(AppProtect::AppProtectJobHandler, Initialize));
    stub.reset(ADDR(PluginJobFactory, CreatePluginJob));
}

/*
 * 用例名称：AppProtectJobHandler主任务进度直接上报
 * 前置条件：获取任务服务启动
 * check点：查看子任务上报结果是否OK
 */
TEST_F(AppProtectJobHandlerTest, AppProtectJobHandler_report_main_job_detail_test)
{
    stub.set(ADDR(AppProtect::PluginMainJob, IsTimeoutLastJobReport), MainJobTimeoutStub);
    stub.set(ADDR(DmeRestClient, SendRequest), SendRequetStub);
    stub.set(ADDR(AppProtectJobHandler, GetUbcIpsByMainJobId), StubGetUbcIpsByMainJobId);
    DoLogTest();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_runJobs.clear();
    {
        PluginJobData jobData = {"pluginName", "123456", "", Json::Value(), AppProtect::MainJobType::BACKUP_JOB};
        std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
        appProtectJobHandler->m_runJobs.push_back(pJob);
    }
    {
        PluginJobData jobData = {"pluginName", "123456", "654321", Json::Value(), 
            AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::GENERATE_SUB_JOB};
        std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
        appProtectJobHandler->m_runJobs.push_back(pJob);
    }
    
    g_reportTime = 0;
    appProtectJobHandler->ReportJobDetailsImpl();
    EXPECT_EQ(g_reportTime, 1);
    stub.reset(ADDR(DmeRestClient, SendRequest));   
    appProtectJobHandler->m_runJobs.clear();
}

/*
 * 用例名称：主任务定时生成详情
 * 前置条件：1.主Job任务构造成功,2.上报周期超时
 * check点：检查主任务定时生成详情是否成功
 */

 TEST_F(AppProtectJobHandlerTest, AppProtectJobHandler_fetch_main_job_detail_gen_test_not_empty)
{
    stub.set(ADDR(AppProtect::PluginMainJob, IsTimeoutLastJobReport), MainJobTimeoutStub);
    PluginJobData jobData = {"pluginName", "mainID", "", Json::Value(), AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    AppProtect::SubJobDetails detail;
    pJob->FetchJobDetail(detail);
    EXPECT_EQ(detail.jobId, "mainID");
}

/*
 * 用例名称：主任务stage转换DME上报
 * 前置条件：1.输入主任务状态
 * check点：检查taskStage和taskStatus是否ok
 */
TEST_F(AppProtectJobHandlerTest, AppProtectJobHandler_transfer_dme_task_status)
{
    Json::Value value;
    int32_t jobStage = mp_int32(MainJobState::COMPLETE);
    ReportJobDetailFactory::GetInstance()->TransferJobStageToJson(jobStage, value);
    EXPECT_EQ(4, value["taskStage"].asInt());
    EXPECT_EQ(3, value["taskStatus"].asInt());
    jobStage = mp_int32(MainJobState::FAILED);
    ReportJobDetailFactory::GetInstance()->TransferJobStageToJson(jobStage, value);
    EXPECT_EQ(6, value["taskStatus"].asInt());
}

/*
 * 用例名称：任务执行前不计时
 * 前置条件：1.实例化任务
 * check点：检查Job初始状态是否超时
 */
TEST_F(AppProtectJobHandlerTest, AppProtectJobHandler_job_first_not_timeout_until_start_timing)
{
    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    auto ret = pJob->IsReportTimeout();
    EXPECT_EQ(false, ret);
}

/*
 * 用例名称：任务执行后开始计时
 * 前置条件：1.实例化任务
 * check点：检查Job初始状态是否超时
 */
TEST_F(AppProtectJobHandlerTest, AppProtectJobHandler_job_first_timeout_after_start_timing)
{
    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    pJob->m_timePoint = std::chrono::steady_clock::now() - std::chrono::minutes(20);
    pJob->StartReportTiming();
    auto ret = pJob->IsReportTimeout();
    EXPECT_EQ(true, ret);
}

/*
 * 用例名称：任务上报超时调用PauseJob接口
 * 前置条件：1.实例化任务
 * check点：检查是否调用PauseJob接口
 */
TEST_F(AppProtectJobHandlerTest, AppProtectJobHandler_job_timeout_call_pause_job_test_ok)
{
    stub.set(ADDR(Job, GetThriftClient), GetThriftClientStub);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    DoLogTest();

    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    pJob->m_timePoint = std::chrono::steady_clock::now() - std::chrono::minutes(20);
    pJob->StartReportTiming();
    pJob->m_startTiming = true;
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_runJobs.clear();
    appProtectJobHandler->m_runJobs.push_back(pJob);
    g_execCount = 0;
    appProtectJobHandler->ReportJobDetailsImpl();
    EXPECT_EQ(g_execCount, 1);
    appProtectJobHandler->m_runJobs.clear();
}

/*
 * 用例名称：任务结束上报成功后不在上报任务详情
 * 前置条件：1.实例化任务 2.任务已经结束
 * check点：检查是只上报了一次
 */
TEST_F(AppProtectJobHandlerTest, AppProtectJobHandler_job_success_report_one_time_test_ok)
{
    DoLogTest();
    stub.set(ADDR(Job, GetThriftClient), GetThriftClientStub);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(ADDR(AppProtectJobHandler, GetUbcIpsByMainJobId), StubGetUbcIpsByMainJobId);
    stub.set(ADDR(DmeRestClient, SendRequest), SendRequetStub);
    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    jobData.status = mp_int32(MainJobState::COMPLETE);
    std::shared_ptr<AppProtect::PluginMainJob> mainJob = std::make_shared<AppProtect::PluginMainJob>(jobData);
    mainJob->m_timePoint = std::chrono::steady_clock::now() - std::chrono::minutes(7);
    mainJob->StartReportTiming();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_runJobs.clear();
    appProtectJobHandler->m_runJobs.push_back(mainJob);
    g_reportTime = 0;
    appProtectJobHandler->ReportJobDetailsImpl();
    appProtectJobHandler->ReportJobDetailsImpl();
    EXPECT_EQ(g_reportTime, 1);
    appProtectJobHandler->m_runJobs.clear();
}

/*
 * 用例名称：任务超时后只中止一次
 * 前置条件：1.实例化任务 2.任务已经结束
 * check点：检查只中止了一次
 */
TEST_F(AppProtectJobHandlerTest, AppProtectJobHandler_job_timeout_call_pause_job_only_one_time_test_ok)
{
    stub.set(ADDR(Job, GetThriftClient), GetThriftClientStub);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    DoLogTest();
    PluginJobData jobData = {"pluginName", "123456", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<AppProtect::PluginMainJob> mainJob = std::make_shared<AppProtect::PluginMainJob>(jobData);
    mainJob->m_timePoint = std::chrono::steady_clock::now() - std::chrono::minutes(20);
    mainJob->StartReportTiming();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_runJobs.clear();
    appProtectJobHandler->m_runJobs.push_back(mainJob);
    g_execCount = 0;
    appProtectJobHandler->ReportJobDetailsImpl();
    appProtectJobHandler->ReportJobDetailsImpl();
    EXPECT_EQ(g_execCount, 1);
    appProtectJobHandler->m_runJobs.clear();
}

/*
 * 用例名称：任务完成后不中止任务
 * 前置条件：1.实例化任务 2.任务已经结束
 * check点：检查中止不中止
 */
TEST_F(AppProtectJobHandlerTest, AppProtectJobHandler_job_finished_no_pause_job_not_call_test_ok)
{
    stub.set(ADDR(Job, GetThriftClient), GetThriftClientStub);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    DoLogTest();
    PluginJobData jobData = {"pluginName", "123456", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    jobData.status = mp_int32(MainJobState::COMPLETE);
    std::shared_ptr<AppProtect::PluginMainJob> mainJob = std::make_shared<AppProtect::PluginMainJob>(jobData);
    mainJob->m_timePoint = std::chrono::steady_clock::now() - std::chrono::minutes(20);
    mainJob->StartReportTiming();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_runJobs.clear();
    appProtectJobHandler->m_runJobs.push_back(mainJob);
    g_execCount = 0;
    appProtectJobHandler->ReportJobDetailsImpl();
    appProtectJobHandler->ReportJobDetailsImpl();
    EXPECT_EQ(g_execCount, 0);
    appProtectJobHandler->m_runJobs.clear();
}

/*
 * 用例名称：AppProtectJobHandler主任务失败上报插件日志详情
 * 前置条件：获取插件上报主任务失败和日志详情
 * check点：查看日志详情是否上报DME
 */
TEST_F(AppProtectJobHandlerTest, AppProtectJobHandler_report_main_job_detail_to_dem_test_ok)
{
    DoLogTest();
    stub.set(ADDR(DmeRestClient, SendRequest), ReportStub);
    stub.set(ADDR(AppProtectJobHandler, GetUbcIpsByMainJobId), StubGetUbcIpsByMainJobId);
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_runJobs.clear();
    PluginJobData jobData = {"pluginName", "123456", "123456", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<AppProtect::PluginMainJob> mainJob = std::make_shared<AppProtect::PluginMainJob>(jobData);
    appProtectJobHandler->m_runJobs.push_back(mainJob);

    AppProtect::ActionResult _return;
    AppProtect::SubJobDetails jobInfo;
    jobInfo.jobId = "123456";
    jobInfo.subJobId = "123456";
    jobInfo.jobStatus = AppProtect::SubJobStatus::FAILED;
    AppProtect::LogDetail detail;
    detail.errorCode = -1;
    detail.description = "test";
    jobInfo.logDetail.push_back(detail);
    g_reported = false;
    mp_int32 iRet = appProtectJobHandler->ReportJobDetails(_return, jobInfo);
    EXPECT_EQ(g_reported, true);
    stub.reset(ADDR(DmeRestClient, SendRequest));
    appProtectJobHandler->m_runJobs.clear();
}
/*
 * 用例名称：AppProtectJobHandler执行中止任务成功
 * 前置条件：收到DME中止请求，并且申请中止的任务可以找到
 * check点：成功中止可以找到的任务
 */
 TEST_F(AppProtectJobHandlerTest, AbortJob)
{
    DoLogTest();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_runJobs.clear();
    EXPECT_EQ(ERR_OBJ_NOT_EXIST, appProtectJobHandler->AbortJob("123456", ""));

    PluginJobData mainJobData = {"pluginName", "123456", "", Json::Value(), AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<AppProtect::PluginMainJob> mainJob = std::make_shared<AppProtect::PluginMainJob>(mainJobData);
    appProtectJobHandler->m_runJobs.push_back(mainJob);
    PluginJobData subJobData = {"pluginName", "mainjob", "subjob", Json::Value(), MainJobType::UNDEFINE, 
        SubJobType::type::PRE_SUB_JOB};
    std::shared_ptr<AppProtect::PluginSubJob> subJob = std::make_shared<AppProtect::PluginSubJob>(subJobData);
    appProtectJobHandler->m_runJobs.push_back(subJob);

    mp_int32 iRet = appProtectJobHandler->AbortJob("123456", "");
    EXPECT_EQ(MP_SUCCESS, iRet);
    iRet = appProtectJobHandler->AbortJob("mainjob", "subjob");
    EXPECT_EQ(MP_SUCCESS, iRet);
}
/*
 * 用例名称：终止正在运行的任务
 * 前置条件：收到DME任务结束的命令
 * check点：仅终止运行中任务
 */
 TEST_F(AppProtectJobHandlerTest, AbortRunningJob)
{
    DoLogTest();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_runJobs.clear();
    
    {
        PluginJobData jobData = { "apptype", "jobId1", "", Json::Value(), AppProtect::MainJobType::BACKUP_JOB };
        jobData.status = mp_uint32(MainJobState::GENERATE_JOB_RUNNING);
        appProtectJobHandler->m_runJobs.push_back(std::make_shared<AppProtect::PluginMainJob>(jobData));
    }
    {
        PluginJobData jobData = { "apptype", "jobId1", "subJobId1", Json::Value(), AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::BUSINESS_SUB_JOB };
        jobData.status = mp_uint32(SubJobState::Running);
        appProtectJobHandler->m_runJobs.push_back(std::make_shared<AppProtect::PluginSubJob>(jobData));
    }
    stub.set(ADDR(PluginSubJob, RemovePluginTimer), SuccClearMountPointsStub);
    stub.set(ADDR(Job, SendAbortToPlugin), SuccStub);
    stub.set(&JobStateDB::UpdateStatus, SuccStub);
    appProtectJobHandler->AbortJob("jobId1");
    appProtectJobHandler->m_runJobs.clear();
}

/*
 * 用例名称：AppProtectJobHandler主任务增量转全量任务变成可重试状态
 * 前置条件：主任务增量检查失败，需要转成全量备份
 * check点：查看主任务状态是否变成可重试状态
 */
TEST_F(AppProtectJobHandlerTest, AppProtectJobHandler_main_job_CheckBackupType_failed_turn_to_be_should_retry_ok)
{
    DoLogTest();
    stub.set(&DmeRestClient::SendRequest, StubRestSendResponse);
    stub.set(ADDR(Job, GetThriftClient), GetThriftClientStub);
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClientAndPluginReturnFailed);
    stub.set(&Job::MountNas, MountNasStub);
    stub.set(&Job::SplitRepositories, SuccStub);
    stub.set(&JobStateDB::QueryJob, SuccStub);
    stub.set(&JobStateDB::UpdateStatus, SuccStub);
    stub.set(&PluginMainJob::GenerateMainJob, SuccStub);
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleep);
    stub.set(&PluginSubPrepJob::WaitPluginNotify, SuccStub);
    stub.set(&PluginSubGeneJob::WaitPluginNotify, SuccStub);
    PluginJobData jobData = {"pluginName", "123456", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    jobData.status = mp_uint32(MainJobState::INITIALIZING);
    std::string strValue = R"({"taskId" : "123456","appInfo" :{"type" : "HDFS","subType" : "HDFSFileset",},
	"taskParams" :{"backupType" : "incrementBackup","scripts":{"failPostScript" : null,"postScript" : null,
    "preScript" : null}},"taskType" : 1})";
    Json::Value jsValue;
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);
    jobData.param = jsValue;
    auto job = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_NE(job, nullptr);
    EXPECT_EQ(MP_SUCCESS, job->Initialize());
    job->Exec();
    EXPECT_EQ(true, job->NeedRetry());
}

/*
 * 用例名称：AppProtectJobHandler主任务可重试状态先删除然后在锁定任务
 * 前置条件：主任务设置重试状态
 * check点：查看主任务是否先删除然后在锁定任务
 */
TEST_F(AppProtectJobHandlerTest, AppProtectJobHandler_main_job_retry_delete_frist_then_lock_ok)
{
    stub.set(ADDR(Job, GetThriftClient), GetThriftClientStub);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(&Job::MountNas, MountNasStub);
    stub.set(&Job::SplitRepositories, SplitRepositoriesStub);
    stub.set(&JobStateDB::QueryJob, QueryJobStub);
    stub.set(&JobStateDB::UpdateStatus, SuccStub);
    DoLogTest();
    stub.set(ADDR(DmeRestClient, SendRequest), ReportStub);
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_runJobs.clear();
    PluginJobData jobData = {"pluginName", "123456", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    jobData.status = mp_uint32(MainJobState::INITIALIZING);
    std::shared_ptr<AppProtect::PluginMainJob> mainJob = std::make_shared<AppProtect::PluginMainJob>(jobData);
    appProtectJobHandler->m_runJobs.push_back(mainJob);
    std::string rsp = R"([{"taskId":"123456","taskType":1,"appInfo":{"type":"HDFS","subType":"HDFSFileset"},"taskParams":{"backupType":"fullBackup"}}])";
    mainJob->SetJobRetry(true);
    std::vector<std::shared_ptr<Job>> jobs;
    appProtectJobHandler->ParseRsp(rsp, jobs);
    EXPECT_EQ(1, jobs.size());
    EXPECT_EQ(appProtectJobHandler->m_runJobs.size(), 0);
}

/*
 * 用例名称：AppProtectJobHandler主任务非重试状态不锁定任务
 * 前置条件：主任务设置重试状态
 * check点：查看主任务是否锁定任务
 */
TEST_F(AppProtectJobHandlerTest, AppProtectJobHandler_main_job_not_retry_not_lock_ok)
{
    stub.set(ADDR(Job, GetThriftClient), GetThriftClientStub);  
    stub.set(&Job::GetProtectServiceClient, StubAllowGetThriftClient);
    stub.set(&Job::MountNas, MountNasStub);
    stub.set(&Job::SplitRepositories, SplitRepositoriesStub);
    stub.set(&JobStateDB::QueryJob, QueryJobStub);
    DoLogTest();
    DoLogTest();
    stub.set(ADDR(DmeRestClient, SendRequest), ReportStub);   
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_runJobs.clear();
    PluginJobData jobData = {"pluginName", "123456", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    jobData.status = mp_uint32(MainJobState::INITIALIZING);
    std::shared_ptr<AppProtect::PluginMainJob> mainJob = std::make_shared<AppProtect::PluginMainJob>(jobData);
    appProtectJobHandler->m_runJobs.push_back(mainJob);
    std::string rsp = R"([{"taskId":"123456","taskType":1,"appInfo":{"type":"HDFS","subType":"HDFSFileset"},"copyInfo":{},"taskParams":{"backupType":"fullBackup"}}])";
    mainJob->SetJobRetry(false);
    std::vector<std::shared_ptr<Job>> jobs;
    appProtectJobHandler->ParseRsp(rsp, jobs);
    EXPECT_EQ(jobs.size(), 0);
    EXPECT_EQ(appProtectJobHandler->m_runJobs.size(), 1);
}

bool StubPluginRunning()
{
    return true;
}

bool StubPluginExists()
{
    return true;
}

bool StubPluginResponse()
{
    return true;
}

bool StubPluginNoUsed()
{
    return true;
}

bool StubPluginUsed()
{
    return false;
}

bool StubPluginIsRegisted()
{
    return false;
}

bool StubIsPluginFolderExist(const std::string &pluginName)
{
    return true;
}

mp_void StubExPluginManagerTestLogVoid(mp_void* pthis)
{
    return;
}
/*
* 用例名称：验证在不同情况下，关闭插件的动作
* 前置条件：
1. 当删除任务时，没有其他任何任务在运行
   预期结果：关闭插件
2. 当删除任务时，没有其他归属于同样插件的任务在运行
   预期结果：关闭插件
3. 当删除任务时，有相同的应用类型的任务在运行
   预期结果：不关闭插件
4. 当删除任务时，有不同的应用类型，但归属于相同插件的任务在运行
   预期结果：不关闭插件
* check点：是否能在不同情况下正确处理关闭插件
*/
TEST_F(AppProtectJobHandlerTest, StopPluginInDiffCondition)
{
    stub.set(ADDR(ExternalPluginManager, IsPluginFolderExist), StubIsPluginFolderExist);
    stub.set(&ExternalPlugin::StopPlugin, StopPluginStub);
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    ExternalPluginManager::GetInstance().m_pluginParseMng = std::make_shared<ExternalPluginParse>();
    plugin_info nasPluginInfo;
    nasPluginInfo.application.push_back("NasShare");
    nasPluginInfo.application.push_back("NasFileSystem");
    nasPluginInfo.name = "NasPlugin";
    nasPluginInfo.appTypeStr = "NasFileSystem,NasShare";
    plugin_info hadoopPluginInfo;
    hadoopPluginInfo.application.push_back("HDFSFileset");
    hadoopPluginInfo.application.push_back("HBaseBackupSet");
    hadoopPluginInfo.name = "HadoopPlugin";
    hadoopPluginInfo.appTypeStr = "HBaseBackupSet,HDFSFileset";

    ExternalPluginManager::GetInstance().m_pluginParseMng->m_pluginsInfo["NasPlugin"] = nasPluginInfo;
    ExternalPluginManager::GetInstance().m_pluginParseMng->m_pluginsInfo["HadoopPlugin"] = hadoopPluginInfo;
    /*场景一：当删除任务时，没有其他任何任务在运行*/
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    stub.set(ADDR(ExternalPlugin, IsWaitPluginRegister), StubPluginIsRegisted);
    stub.set(ADDR(ExternalPlugin, IsPluginRunning), StubPluginRunning);
    stub.set(ADDR(ExternalPlugin, IsPluginProcessExist), StubPluginExists);
    stub.set(ADDR(ExternalPlugin, IsPluginResponding), StubPluginResponse);
    stub.set(ADDR(ExternalPlugin, IsNoUseTimeout), StubPluginNoUsed);

    appProtectJobHandler->m_runJobs.clear();
    PluginJobData jobData = {"NasShare", "123456", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    jobData.status = mp_uint32(MainJobState::INITIALIZING);
    std::shared_ptr<AppProtect::PluginMainJob> mainJob = std::make_shared<AppProtect::PluginMainJob>(jobData);
    appProtectJobHandler->m_runJobs.push_back(mainJob);

    ExternalPluginManager::GetInstance().m_pluginMap["NasPlugin"] = std::make_unique<ExternalPlugin>("NasPlugin", "root", false);
    MainJobInfoPtr deleteJob = std::make_shared<PluginJobData>();
    deleteJob->appType = "NasShare";
    deleteJob->mainID = "123456";
    appProtectJobHandler->DelRunJob(deleteJob);
    EXPECT_EQ(1, ExternalPluginManager::GetInstance().m_pluginMap.size());
    ExternalPluginManager::GetInstance().MonitorPlugin();
    EXPECT_EQ(0, ExternalPluginManager::GetInstance().m_pluginMap.size());

    /*场景二：当删除任务时，没有其他归属于同样插件的任务在运行*/
    appProtectJobHandler->m_runJobs.clear();
    PluginJobData jobDataNas = {"NasShare", "123456", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    jobDataNas.status = mp_uint32(MainJobState::INITIALIZING);
    std::shared_ptr<AppProtect::PluginMainJob> mainJobNas = std::make_shared<AppProtect::PluginMainJob>(jobDataNas);
    appProtectJobHandler->m_runJobs.push_back(mainJobNas);
    PluginJobData jobDataHbase = {"HBaseBackupSet", "hbase", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    jobDataHbase.status = mp_uint32(MainJobState::INITIALIZING);
    std::shared_ptr<AppProtect::PluginMainJob> mainJobHbase = std::make_shared<AppProtect::PluginMainJob>(jobDataHbase);
    appProtectJobHandler->m_runJobs.push_back(mainJobHbase);

    ExternalPluginManager::GetInstance().m_pluginMap["NasPlugin"] = std::make_unique<ExternalPlugin>("NasPlugin", "root", false);
    ExternalPluginManager::GetInstance().m_pluginMap["HadoopPlugin"] = std::make_unique<ExternalPlugin>("HadoopPlugin", "root", false);

    appProtectJobHandler->DelRunJob(deleteJob);
    ExternalPluginManager::GetInstance().MonitorPlugin();
    auto iter = ExternalPluginManager::GetInstance().m_pluginMap.find("NasPlugin");
    EXPECT_EQ(ExternalPluginManager::GetInstance().m_pluginMap.end(), iter);

    /*场景三：当删除任务时，有相同的应用类型的任务在运行*/
    stub.set(ADDR(ExternalPlugin, IsNoUseTimeout), StubPluginUsed);
    appProtectJobHandler->m_runJobs.clear();
    appProtectJobHandler->m_runJobs.push_back(mainJobNas);
    PluginJobData jobDataNas1 = {"NasShare", "11", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    jobDataNas1.status = mp_uint32(MainJobState::INITIALIZING);
    std::shared_ptr<AppProtect::PluginMainJob> mainJob1 = std::make_shared<AppProtect::PluginMainJob>(jobDataNas1);
    appProtectJobHandler->m_runJobs.push_back(mainJob1);

    ExternalPluginManager::GetInstance().m_pluginMap["NasPlugin"] = std::make_unique<ExternalPlugin>("NasPlugin", "root", false);
    appProtectJobHandler->DelRunJob(deleteJob);
    ExternalPluginManager::GetInstance().MonitorPlugin();
    iter = ExternalPluginManager::GetInstance().m_pluginMap.find("NasPlugin");
    EXPECT_NE(ExternalPluginManager::GetInstance().m_pluginMap.end(), iter);

    /*场景四：当删除任务时，有不同的应用类型，但归属于相同插件的任务在运行*/
    appProtectJobHandler->m_runJobs.clear();
    appProtectJobHandler->m_runJobs.push_back(mainJobNas);
    PluginJobData jobDataNasFs = {"NasFileSystem", "11", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    jobDataNasFs.status = mp_uint32(MainJobState::INITIALIZING);
    std::shared_ptr<AppProtect::PluginMainJob> mainJobFs = std::make_shared<AppProtect::PluginMainJob>(jobDataNasFs);
    appProtectJobHandler->m_runJobs.push_back(mainJobFs);

    ExternalPluginManager::GetInstance().m_pluginMap["NasPlugin"] = std::make_unique<ExternalPlugin>("NasPlugin", "root", false);
    appProtectJobHandler->DelRunJob(deleteJob);
    ExternalPluginManager::GetInstance().MonitorPlugin();
    iter = ExternalPluginManager::GetInstance().m_pluginMap.find("NasPlugin");
    EXPECT_NE(ExternalPluginManager::GetInstance().m_pluginMap.end(), iter);
}

TEST_F(AppProtectJobHandlerTest, Umount)
{
    DoLogTest();
    auto pHandler = AppProtect::AppProtectJobHandler::GetInstance();
    pHandler->m_runJobs.clear();
    
    {
        PluginJobData jobData = { "apptype", "jobId1", "", Json::Value(), AppProtect::MainJobType::BACKUP_JOB };
        std::vector<mp_string> mountPoints;
        mountPoints.push_back("/tmp/advabackup");
        jobData.mountPoints.insert(make_pair(RepositoryDataType::type::META_REPOSITORY, mountPoints));
        pHandler->m_runJobs.push_back(std::make_shared<AppProtect::PluginMainJob>(jobData));
    }
    stub.set(ADDR(PrepareFileSystem, UmountNasFileSystem), SuccStub);
    pHandler->Umount("jobId1");
}

/*
 * 用例名称：AppProtectJobHandler请求正在运行的任务
 * 前置条件：1. mock SendRequest 2. mock ParseRsp
 * check点：查看请求到的任务数量和ParseRsp解析出的任务数量是否相同
 */
TEST_F(AppProtectJobHandlerTest, AcquireRunningJob)
{
    DoLogTest();
    stub.set(ADDR(DmeRestClient, SendRequest), ReportStub);
    stub.set(ADDR(AppProtect::AppProtectJobHandler, ParseRsp), StubParseRsp);
    
    PluginJobData data;
    data.mainID = "mainID";
    std::shared_ptr<Job> job = std::make_shared<MockJob>(data);
    auto pHandler = AppProtect::AppProtectJobHandler::GetInstance();
    EXPECT_EQ(1, pHandler->AcquireRunningJob(job->GetData()).size());
}

/*
 * 用例名称：AppProtectJobHandler请求正在运行的任务
 * 前置条件：1. mock SendRequest 2. mock ParseRsp
 * check点：未从数据库查询到任务，查看正在执行的任务数量是否为0
 */
TEST_F(AppProtectJobHandlerTest, StartRedoProcess_NoJob)
{
    DoLogTest();
    stub.set(ADDR(JobStateDB, QueryAllJob), SuccStub);

    auto pHandler = AppProtect::AppProtectJobHandler::GetInstance();
    pHandler->m_runJobs.clear();

    pHandler->StartRedoProcess();
    EXPECT_EQ(0, pHandler->m_runJobs.size());
}

/*
 * 用例名称：AppProtectJobHandler请求正在运行的任务
 * 前置条件：1. mock SendRequest 2. mock ParseRsp
 * check点：未从数据库查询到任务，查看正在执行的任务数量是否为0
 */
TEST_F(AppProtectJobHandlerTest, StartRedoProcess)
{
    DoLogTest();
    stub.set(ADDR(JobStateDB, QueryAllJob), StubQueryAllJob);
    stub.set(ADDR(DmeRestClient, SendRequest), ReportStub);
    stub.set(ADDR(AppProtect::AppProtectJobHandler, ParseRsp), StubParseRsp);
    stub.set(ADDR(JobPool, PushJob), StubPushJob);

    auto pHandler = AppProtect::AppProtectJobHandler::GetInstance();
    pHandler->m_runJobs.clear();

    pHandler->StartRedoProcess();
    EXPECT_EQ(1, pHandler->m_runJobs.size());
    EXPECT_EQ("mainJobID", pHandler->m_runJobs.front()->GetData().mainID);
}

/*
 * 用例名称：AppProtectJobHandler获取连通的host ip list
 * 前置条件：host ip 8.40.1.2 与storage ip连通, 其余IP与storage ip不连通
 * check点：获取到连通的host ip list 为8.40.1.2
 */

TEST_F(AppProtectJobHandlerTest, CheckIpConnectionTest)
{
    DoLogTest();
    stub.set(ADDR(CIP, GetHostIPList), StubGetValidIpList);
    stub.set(ADDR(CSocket, CheckHostLinkStatus), StubCheckHostLinkStatus);
    auto testins = AppProtect::AppProtectJobHandler::GetInstance();
    testins->m_ipCacheObj.SetAgentType(MP_FALSE);
    const std::multimap<mp_string, std::vector<mp_string>> doradoIpv4List = {{"/Data_test", {"1.1.1.1", "1.1.1.2"}}};
    std::vector<mp_string> outLists;
    std::vector<mp_string> validDoradoIps;
    testins->CheckIpConnection(doradoIpv4List, outLists, validDoradoIps);
    EXPECT_EQ(1, outLists.size());
    EXPECT_EQ("8.40.1.2", outLists[0]);
    std::vector<mp_string> outLists2;
    testins->CheckIpConnection(doradoIpv4List, outLists2, validDoradoIps);
    EXPECT_EQ(1, outLists2.size());
    EXPECT_EQ("8.40.1.2", outLists2[0]);
}

/*
 * 用例名称：AppProtectJobHandler检查文件系统逻辑端口连通性
 * 前置条件：1、文件系统Data_test1 逻辑端口 8.40.1.2 与storage ip连通, 其余文件系统IP与storage ip不连通
            2、所有文件系统逻辑端口都不通
            3、文件系统Data_test1与Data_test2都有逻辑端口能连通
            4、文件系统Data_test1有逻辑端口能连通，文件系统Data_test2下发逻辑端口为空
 * check点：1、检查连通性失败，返回特定错误码
            2、检查连通性失败，返回特定错误码
            3、检查连通性成功
            4、检查连通性成功
 */

TEST_F(AppProtectJobHandlerTest, CheckRepoIpConnectionTest)
{
    DoLogTest();
    stub.set(ADDR(CIP, GetHostIPList), StubGetValidIpList);
    stub.set(ADDR(CacheHostIp, GetIpList), StubGetIpList);
    auto testins = AppProtect::AppProtectJobHandler::GetInstance();
    testins->m_ipCacheObj.SetAgentType(MP_FALSE);
    const std::multimap<mp_string, std::vector<mp_string>> doradoIpv4List = {{"/Data_test1", {"8.40.1.1", "8.40.1.2"}},
        {"/Data_test2", {"8.40.1.3", "8.40.1.4"}}};
    std::vector<mp_string> outLists;
    std::vector<mp_string> validDoradoIps;
    mp_int32 ret = testins->CheckIpConnection(doradoIpv4List, outLists, validDoradoIps);
    EXPECT_EQ(ERR_DISCONNECT_STORAGE_NETWORK, ret);

    stub.set(ADDR(CacheHostIp, GetIpList), StubGetIpListFail);
    ret = testins->CheckIpConnection(doradoIpv4List, outLists, validDoradoIps);
    EXPECT_EQ(ERR_DISCONNECT_STORAGE_NETWORK, ret);

    stub.set(ADDR(CacheHostIp, GetIpList), StubGetIpListSuc);
    ret = testins->CheckIpConnection(doradoIpv4List, outLists, validDoradoIps);
    EXPECT_EQ(MP_SUCCESS, ret);

    const std::multimap<mp_string, std::vector<mp_string>> doradoIpv4ListV2 = {{"/Data_test1", {"8.40.1.1", "8.40.1.2"}},
        {"/Data_test2", {}}};
    stub.set(ADDR(CacheHostIp, GetIpList), StubGetIpList);
    ret = testins->CheckIpConnection(doradoIpv4ListV2, outLists, validDoradoIps);
    EXPECT_EQ(MP_SUCCESS, ret);
}

/*
 * 用例名称：当前节点任务数较少时可以订阅
 * 前置条件：stub SendRequest
 * check点：当前节点任务数0，其他节点任务数0，可以订阅
 */
TEST_F(AppProtectJobHandlerTest, AllowSubcribe_SUCCESS)
{
    DoLogTest();
    stub.set(ADDR(DmeRestClient, SendRequest), StubStatisticInfo);
    auto testins = AppProtect::AppProtectJobHandler::GetInstance();
    testins->m_nodeId = "id1";
    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<Job> pJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    EXPECT_EQ(MP_SUCCESS, testins->AllowSubcribe(pJob));
}

/*
 * 用例名称：测试JobTimer IsOverInterval函数
 * 前置条件：超时时间为-1
 * check点：第一次未启动计时，返回false, 第二次进入返回true
 */
TEST_F(AppProtectJobHandlerTest, CheckIsOverInterval)
{
    DoLogTest();
    JobTimer m_timerins(-1);
    EXPECT_EQ(false, m_timerins.IsOverInterval());
    EXPECT_EQ(true, m_timerins.IsOverInterval());
    JobTimer ins(10);
    ins.IsOverInterval();
    EXPECT_EQ(false, ins.IsOverInterval());
}

/*
 * 用例名称：AppProtectJobHandler重启后续作，重新将已完成的任务放到队列中
 * 前置条件：1. agent重启获取到已完成的任务
 * check点：确认已完成的主任务只会加到任务队列中，不会放到线程池中
 */
TEST_F(AppProtectJobHandlerTest, Finished_Main_Job_Redo_test_add_to_runningJobs_ok)
{
    DoLogTest();
    stub.set(ADDR(JobPool, PushJob), StubPushJob);
    stub.set(ADDR(AppProtectJobHandler, AcquireRunningJob), StubAcquireRunningJob);
    stub.set(ADDR(AppProtectJobHandler, AddAcquireInfo), StubAddAcquireInfo);
    stub.set(ADDR(DmeRestClient, UpdateDmeAddr), UpdateDmeAddrStub);
    
    PluginJobData jobData = {"pluginName", "mainJobID", "", Json::Value(),  AppProtect::MainJobType::BACKUP_JOB};
    jobData.status = static_cast<int32_t>(MainJobState::COMPLETE);
    auto pHandler = AppProtect::AppProtectJobHandler::GetInstance();
    pHandler->m_runJobs.clear();
    g_pushJop = false;
    pHandler->RedoJob(jobData);
    EXPECT_NE(true, g_pushJop);
}

/*
 * 用例名称：是否上报主任务详情
 * 前置条件：
 * check点：1.插件上报不能执行任务的错误码，并且任务状态为RUNNING，不上报主任务详情
            2.其余情况都上报
 */
TEST_F(AppProtectJobHandlerTest, AppProtectJobHandler_is_report_main_job_detail)
{
    Json::Value value;
    AppProtect::SubJobDetails jobInfo;
    AppProtect::LogDetail detail;
    detail.errorCode = 0x5E02503B;
    jobInfo.logDetail.push_back(detail);
    value["taskStatus"] = 1;
    mp_bool ret = ReportJobDetailFactory::GetInstance()->IsReportMainJobDetail(jobInfo, value);
    EXPECT_EQ(ret, MP_FALSE);
    value["taskStatus"] = 6;
    ret = ReportJobDetailFactory::GetInstance()->IsReportMainJobDetail(jobInfo, value);
    EXPECT_EQ(ret, MP_TRUE);
}

/*
 * 用例名称：判断任务是否可在当前节点运行
 * 前置条件：
 * check点：1.在优先主/备策略下，当前节点与策略不匹配，但是符合策略的节点都已经执行失败，则当前节点可执行
            2.在优先主/备策略下，当前节点与策略不匹配，但是符合策略的节点还有未执行的，则当前节点不可执行
 */
TEST_F(AppProtectJobHandlerTest, AppProtectJobHandler_CheckCanBeRunInLocal_Success)
{
    // DoLogTest();
    // stub.set(ADDR(CHost, GetHostSN), StubGetHostSNSuccess);
    // stub.set(ADDR(PluginSubBusiJob, ExecAllowBackupInLocalNode), ExecAllowBackupInLocalNodeSuccess);
    // stub.set(((void)(Job::*)(void (ApplicationServiceIf::*DiscoverHostCluster)(ApplicationEnvironment&, ApplicationEnvironment), ApplicationEnvironment&, ApplicationEnvironment))ADDR(Job,ApplicationServiceNormalCall),StubApplicationServiceNormalCall);

    // Json::Value jsValue;
    // jsValue[REQUESTID] = REQUESTID;
    // jsValue[TASKID] = TASKID;
    // jsValue[SUBTASKID] = SUBTASKID;
    // jsValue[TASKTYPE] = 0;

    // ApplicationEnvironment env;
    // ApplicationEnvironment node;
    // node.id = "123456";
    // node.name = "123456";
    // node.extendInfo = "{\"role\":1}";
    // env.nodes.push_back(node);
    // Json::Value jsEnv;
    // StructToJson(env, jsEnv);
    // jsValue[ENVINFO] = jsEnv;
    // Json::Value jsTmp3;
    // jsTmp3["slaveNodeFirst"] = true;
    // Json::Value jsTmp2;
    // jsTmp2["extParameters"] = jsTmp3;
    // jsValue[EXTENDINFO] = jsTmp2;

    // PluginJobData pluginData = {"appType",
    //         jsValue[TASKID].asString(),
    //         jsValue[SUBTASKID].asString(),
    //         jsValue,
    //         MainJobType::BACKUP_JOB,
    //         SubJobType::type::BUSINESS_SUB_JOB};
    // auto pJob = std::make_shared<PluginSubBusiJob>(pluginData);
    // auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    // mp_int32 iRet = appProtectJobHandler->CheckCanBeRunInLocal(pJob);
    // EXPECT_EQ(iRet, MP_SUCCESS);

    // stub.reset(ADDR(CHost, GetHostSN));
    // //stub.reset(ADDR(PluginSubBusiJob, UpdateRole));
    // stub.reset(ADDR(PluginSubBusiJob, ExecAllowBackupInLocalNode));
}

/*
 * 用例名称：任务详情添加nodeid
 * 前置条件：
 * check点：1.插件上报不能执行任务的错误码，并且任务状态为FAILED，将nodeID信息添加到任务详情中
 */
TEST_F(AppProtectJobHandlerTest, AppProtectJobHandler_job_detail_add_nodeid)
{
    Json::Value value;
    AppProtect::SubJobDetails jobInfo;
    AppProtect::LogDetail detail;
    detail.errorCode = 0x5E02503B;
    jobInfo.logDetail.push_back(detail);
    value["taskStatus"] = 6;
    stub.set(ADDR(CHost, GetHostSN), StubGetHostSN);
    ReportJobDetailFactory::GetInstance()->JobDetailAddNodeId(jobInfo, value);
    EXPECT_EQ(value["extendInfo"]["nodeId"], "123");
}

/*
 * 用例名称：通过主任务Id获取ubcIP
 * 前置条件：
 * check点：1.任务列表为空，获取ubcIP失败2.主任务Id存在于主任务列表中，ubcIp获取成功
 */
TEST_F(AppProtectJobHandlerTest, GetUbcIpsByMainJobId)
{
    DoLogTest();
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    appProtectJobHandler->m_vecAcquireInfo.clear();

    PluginJobData jobData = {"apptype", "jobId1", "", Json::Value(), AppProtect::MainJobType::BACKUP_JOB};
    MainJobInfoPtr pInfo = std::make_shared<PluginJobData>(jobData);
    std::vector<mp_string> ubcIps;
    stub.set(ADDR(AppProtectJobHandler, JudgeMainJobCnt), SuccStub);
    EXPECT_EQ(MP_FAILED, appProtectJobHandler->GetUbcIpsByMainJobId("jobId1", ubcIps));
    appProtectJobHandler->AddAcquireInfo(pInfo);
    EXPECT_EQ(MP_SUCCESS, appProtectJobHandler->GetUbcIpsByMainJobId("jobId1", ubcIps));
}
