#define private public
#include <fstream>
#include "pluginfx/ExternalPluginManagerTest.h"
#include "pluginfx/ExternalPluginManager.h"
#include <thread>
#include "common/Path.h"
#include "common/Log.h"
#include "common/CMpTime.h"
#include "common/Uuid.h"
#include "securecom/RootCaller.h"
#include "common/ConfigXmlParse.h"
#include "common/CSystemExec.h"
#include "stub.h"
#include "servicecenter/thriftservice/detail/ThriftService.h"
#include "servicecenter/thriftservice/include/IThriftServer.h"
#include "servicecenter/thriftservice/detail/ThriftClient.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
using namespace thriftservice;
using namespace AppProtect;

mp_void StubExPluginManagerTestLogVoid(mp_void* pthis){
    return;
}

mp_bool StubFileExistSucess(const mp_string& pszFilePath)
{
    return MP_TRUE;
}
mp_bool StubFileExistFail(const mp_string& pszFilePath)
{
    return MP_FALSE;
}

mp_int32 StubExecSuccess(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

mp_int32 StubExecFail(mp_int32 iCommandID, const std::string &scriptCmd)
{
    return MP_FAILED;
}

mp_int32 StubCConfigXmlParserInitSuccess(mp_string strCfgFilePath)
{
    return MP_SUCCESS;
}

mp_int32 StubGetConfigValueInt32Success(const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
//    iValue = 0;
    return MP_SUCCESS;
}

mp_int32 StubGetValueBoolSuccess(const mp_string& strSection, const mp_string& strKey, mp_bool& bValue)
{
    bValue = MP_TRUE;
    return MP_SUCCESS;
}

void ThreadUpdatePluginStatus()
{
    CMpTime::DoSleep(1000);
    ExternalPluginManager::GetInstance().UpdatePluginStatus(std::string("NasPlugin"), EX_PLUGIN_STATUS::ISREGISTERED);        
}

mp_void* StubMonPluginStatusThread(mp_void *pThis)
{
    return nullptr;
}

mp_int32 StubGetPluginNameByAppTypeSucess(void *obj, const mp_string &appType, mp_string &pluginName)
{
    pluginName = "NasPlugin";
    return MP_SUCCESS;
}

mp_string StubGetStartUserSuccess(const mp_string &pluginName)
{
    return "root";
}

std::shared_ptr<thriftservice::IThriftServer> StubRegisterServer(void *obj, const std::string& host, int32_t port)
{
    return nullptr;
}

mp_int32 StubExecTestSucc(void *, mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&) = NULL, void* pTaskStep = NULL)
{
    if (pvecResult == NULL) {
        return MP_SUCCESS;
    }
    pvecResult->push_back(" exe -> /opt/DataBackup/ProtectClient/Plugins/NasPlug");
    return MP_SUCCESS;
}

mp_int32 StubExecTestFail(void *, mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&) = NULL, void* pTaskStep = NULL)
{
    return MP_FAILED;
}

std::pair<mp_int32, mp_string> StubGetAbsolutePath(void *, mp_string path)
{
    if (path.empty()) {
        return std::make_pair(MP_SUCCESS, "/opt/ceanProtect/rotectClient/ProtectClient-E//../Plugins/");
    }
    return std::make_pair(MP_SUCCESS, "/opt/DataBackup/ProtectClient/Plugins/");
}

static int32_t g_countStart = 0;
class ThriftClientStub : public thriftservice::IThriftClient {
public:
    virtual bool Start()
    {
        return --g_countStart == 0;
    }
    virtual bool Stop()
    {
        return true;
    }
    virtual std::shared_ptr<apache::thrift::protocol::TProtocol> GetTProtocol()
    {
        return nullptr;
    }
    virtual std::shared_ptr<apache::thrift::async::TConcurrentClientSyncInfo> GetSyncInfo()
    {
        return nullptr;
    }
};

std::shared_ptr<thriftservice::IThriftClient> GetThriftClientFromServiceStub(void *obj, const std::string& host, int32_t port)
{
    return std::make_shared<ThriftClientStub>();
}

static mp_bool StubDirExist(const mp_char* pszDirPath){
    return MP_FALSE;
}


mp_int32 StubExecStartPlugin(void* obj)
{
    return MP_SUCCESS;
}

bool StubIsPluginFolderExist(const std::string &pluginName)
{
    return true;
}

bool StubPluginRunning()
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

std::shared_ptr<ExternalPlugin> StubGetPluginByRest(void* obj, const mp_string &appType)
{
    return std::make_shared<ExternalPlugin>("HDFS", "test1", false, 59570);
}

std::shared_ptr<ExternalPlugin> StubGetPluginByRestNullptr(void* obj, const mp_string &appType)
{
    return nullptr;
}

std::shared_ptr<thriftservice::IThriftClient> StubGetPluginClient(void* obj)
{
    return std::make_shared<ThriftClientStub>();
}

std::shared_ptr<thriftservice::IThriftClient> StubGetPluginClientNullptr(void* obj)
{
    return nullptr;
}

class ApplicationServiceClientSub : public ApplicationServiceConcurrentClient {
public:
    ApplicationServiceClientSub(std::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
        ApplicationServiceConcurrentClient(prot, nullptr) {}

    void CheckApplication(ActionResult& _return, const ApplicationEnvironment& appEnv, const Application& application) override
    {
        _return.code = MP_SUCCESS;
    }
    void DiscoverAppCluster(ApplicationEnvironment& _return, const ApplicationEnvironment& appEnv, const Application& application) override
    {
        _return.id = "xxxx2";
        _return.name = "test_cluster";
    }
	
	void ListApplicationResourceV2(ResourceResultByPage& _return, const ListResourceRequest& request) override
    {
    }

    void ListApplicationConfig(std::map<std::string, std::string>& _return, const std::string& script) override
    {
    }

    void OracleCheckArchiveArea(ActionResult& _return, 
        const std::string& appType, const std::vector<AppProtect::OracleDBInfo>& dbInfoList) override
    {
        _return.code = MP_SUCCESS;
    }
};

static std::shared_ptr<ApplicationServiceClientSub> g_ApplicationServiceClient;
static std::shared_ptr<ApplicationServiceConcurrentClient> StubGetApplicationServiceClient(
    mp_void* pThis, const std::shared_ptr<thriftservice::IThriftClient>& pThriftClient)
{
    if (g_ApplicationServiceClient.get() == nullptr) {
        g_ApplicationServiceClient = std::make_shared<ApplicationServiceClientSub>(nullptr);
    }
    return g_ApplicationServiceClient;
}

/*
*用例名称：启动插件失败，因传入的插件类型为空
*前置条件：1、插件类型不存在
*check点：不能启动插件类型为空的插件
*/
TEST_F(ExternalPluginManagerTest, startPluginFailForEmptyPluginType) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_uint32 ret = ExternalPluginManager::GetInstance().StartPlugin("");
    EXPECT_EQ(ret, MP_FAILED);
}
/*
*用例名称：启动插件失败，因启动脚本不存在
*前置条件：1、插件类型正确，但启动脚本不存在
*check点：脚本不存在无法正常启动插件
*/
TEST_F(ExternalPluginManagerTest, startPluginFailScriptNoexist)
{
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(mp_string))ADDR(CConfigXmlParser,Init), 
                StubCConfigXmlParserInitSuccess); 
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), 
                StubGetConfigValueInt32Success); 
    mp_stub.set(ADDR(CMpFile, FileExist), StubFileExistFail);        
    mp_uint32 ret = ExternalPluginManager::GetInstance().StartPlugin("NAS_File_System");
    EXPECT_EQ(ret, MP_FAILED);    
}
/*
*用例名称：停止插件失败，因停止脚本执行失败
*前置条件：1、插件已经启动，脚本存在，但脚本不可执行
*check点：脚本执行失败，插件停止失败
*/
TEST_F(ExternalPluginManagerTest, startPluginExecScriptFail) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(CMpFile, FileExist), StubFileExistSucess);
    mp_stub.set(ADDR(CRootCaller, ExecUserDefineScript), StubExecFail);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(mp_string))ADDR(CConfigXmlParser,Init), 
                StubCConfigXmlParserInitSuccess); 
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), 
                StubGetConfigValueInt32Success); 
    mp_stub.set(ADDR(ExternalPluginParse, GetPluginNameByAppType), StubGetPluginNameByAppTypeSucess);
    mp_uint32 ret = ExternalPluginManager::GetInstance().StartPlugin("NAS_File_System");
    EXPECT_EQ(ret, MP_FAILED);
}
/*
用例名称：启动插件因无法获取到启动用户而失败
前置条件：1、存在插件启动脚本，且脚本可执行 2. 插件配置文件中没有合法的启动用户
check点：如果无法获取到合法的启动用户，则无法启动插件
*/
TEST_F(ExternalPluginManagerTest, startPluginFailForIlegalUser) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(CMpFile, FileExist), StubFileExistSucess);
    mp_stub.set(ADDR(CRootCaller, ExecUserDefineScript), StubExecSuccess);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(mp_string))ADDR(CConfigXmlParser,Init), 
                StubCConfigXmlParserInitSuccess); 
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), 
                StubGetConfigValueInt32Success); 
    mp_stub.set(ADDR(ExternalPluginParse, GetPluginNameByAppType), StubGetPluginNameByAppTypeSucess);
    auto ret = ExternalPluginManager::GetInstance().GetPlugin("NAS_File_System");
    EXPECT_EQ(ret, nullptr);
}
/*
用例名称：启动插件成功
前置条件：1、存在插件启动脚本，且脚本可执行
check点：插件启动正常流程
*/
TEST_F(ExternalPluginManagerTest, startPluginSuccess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(CMpFile, FileExist), StubFileExistSucess);
    mp_stub.set(ADDR(CRootCaller, ExecUserDefineScript), StubExecSuccess);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(mp_string))ADDR(CConfigXmlParser,Init), 
                StubCConfigXmlParserInitSuccess); 
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), 
                StubGetConfigValueInt32Success); 
    mp_stub.set(ADDR(ExternalPluginParse, GetPluginNameByAppType), StubGetPluginNameByAppTypeSucess);
    mp_stub.set(ADDR(ExternalPluginParse, GetStartUser), StubGetStartUserSuccess);
    std::thread testT(ThreadUpdatePluginStatus);
    testT.detach();
    auto ret = ExternalPluginManager::GetInstance().GetPlugin("NAS_File_System");
    EXPECT_NE(ret, nullptr);
}
/*
*用例名称：停止插件成功
*前置条件：1、存在插件停止脚本，且脚本可执行
*check点：插件停止正常流程
*/
TEST_F(ExternalPluginManagerTest, stopPluginSuccess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(CMpFile, FileExist), StubFileExistSucess);
    mp_stub.set(ADDR(CRootCaller, ExecUserDefineScript), StubExecSuccess);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(mp_string))ADDR(CConfigXmlParser,Init), 
                StubCConfigXmlParserInitSuccess); 
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), 
                StubGetConfigValueInt32Success); 
    mp_stub.set(ADDR(ExternalPluginParse, GetPluginNameByAppType), StubGetPluginNameByAppTypeSucess);
    mp_stub.set(ADDR(ExternalPluginParse, GetStartUser), StubGetStartUserSuccess);
    std::thread testT(ThreadUpdatePluginStatus);
    testT.detach();
    auto ret = ExternalPluginManager::GetInstance().GetPlugin("NAS_File_System");
    EXPECT_NE(ret, nullptr);
    ExternalPluginManager::GetInstance().ReleasePlugin("NAS_File_System");
}
/*
*用例名称：停止插件失败，因插件未启动
*前置条件：1、插件未启动
*check点：无法停止未启动的插件
*/
TEST_F(ExternalPluginManagerTest, stopPluginFailForNotStart) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    ExternalPluginManager::GetInstance().ReleasePlugin("NAS_File_System");
}

/*
*用例名称：停止插件失败，因停止脚本不存在
*前置条件：1、插件已经启动，但停止脚本不存在
*check点：脚本不存在无法正常停止插件
*/
TEST_F(ExternalPluginManagerTest, stopPluginFailScriptNotExists) {
    Stub mp_stub;
    mp_stub.set(ADDR(CRootCaller, Exec), StubExecSuccess);
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(CMpFile, FileExist), StubFileExistSucess);
    mp_stub.set(ADDR(CRootCaller, ExecUserDefineScript), StubExecSuccess);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(mp_string))ADDR(CConfigXmlParser,Init), 
                StubCConfigXmlParserInitSuccess); 
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), 
                StubGetConfigValueInt32Success); 
    //mp_stub.set(ADDR(ExternalPlugin, InitPluginClient), StubInitPluginClientSuccess);
    mp_stub.set(ADDR(ExternalPluginParse, GetPluginNameByAppType), StubGetPluginNameByAppTypeSucess);
    mp_stub.set(ADDR(ExternalPluginParse, GetStartUser), StubGetStartUserSuccess);
    std::thread testT(ThreadUpdatePluginStatus);
    testT.detach();
    auto ret = ExternalPluginManager::GetInstance().GetPlugin("NAS_File_System");
    EXPECT_NE(ret, nullptr);
    mp_stub.set(ADDR(CMpFile, FileExist), StubFileExistFail);
    ExternalPluginManager::GetInstance().ReleasePlugin("NAS_File_System");
}

/*
*用例名称：外部插件初始化成功
*前置条件：1、相关配置已配置完成
*check点：成功初始化外部插件管理
*/
TEST_F(ExternalPluginManagerTest, initExternalPluginSuccess) {
    using namespace thriftservice::detail;
    using namespace thriftservice;
    Stub mp_stub;
    ExternalPluginManager externalPluginManager;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(mp_string))ADDR(CConfigXmlParser,Init), 
                StubCConfigXmlParserInitSuccess); 
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), 
                StubGetConfigValueInt32Success); 
    mp_stub.set(ADDR(ExternalPluginManager, MonPluginStatusThread), StubMonPluginStatusThread);
    typedef std::shared_ptr<IThriftServer> (*fptr_reg)(ThriftService*,const std::string&, int32_t);
    fptr_reg reg = (fptr_reg)(&ThriftService::RegisterServer);   //obtaining an address
    mp_stub.set(reg, StubRegisterServer);

//    mp_int32 ret = externalPluginManager.Init();
//    EXPECT_EQ(ret, MP_SUCCESS);
}

// /*
// *用例名称：外部插件未注册获取thrift接口失败
// *前置条件：1、相关配置已配置完成
// *check点：在外部插件未注册的情况下获取thrift客户端
// */
// TEST_F(ExternalPluginManagerTest, ExternalPlugin_GetPluginClient_wait_register_register_timeout_test_ok) {
//     using namespace thriftservice::detail;
//     using namespace thriftservice;
//     Stub mp_stub;
//     mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
//             const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);

//     ExternalPlugin plugin("HDFS", "test1", false);
//     plugin.m_waitTimeout = 1;
//     bool timeout = false;
//     std::shared_ptr<thriftservice::IThriftClient> client;
//     std::thread th1(
//         [&plugin, &client](){
//             client = plugin.GetPluginClient();
//         }
//     );
//     std::thread th2(
//         [&plugin, &timeout](){
//             plugin.CheckPluginRegister(timeout);
//         }
//     );
//     th1.join();
//     th2.join();
//     EXPECT_EQ(client, nullptr);
//     EXPECT_EQ(timeout, true);
// }

/*
*用例名称：外部插件注册后获取thrift接口成功
*前置条件：1、相关配置已配置完成
*check点：在外部插件注册的情况下获取thrift客户端
*/
TEST_F(ExternalPluginManagerTest, ExternalPlugin_after_monitor_notify_register_can_get_client_test_ok) {
    using namespace thriftservice::detail;
    using namespace thriftservice;
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(ExternalPlugin, GetThriftClientFromService), GetThriftClientFromServiceStub);
    ExternalPlugin plugin("HDFS", "test1", false);
    plugin.m_waitTimeout = 60;
    plugin.m_pluginInfo.endPoint = "127.0.0.1";
    plugin.m_pluginInfo.port = 59610;
    bool timeout = false;
    g_countStart = 1;
    std::shared_ptr<thriftservice::IThriftClient> client;
    std::thread th1(
        [&plugin, &client](){
            client = plugin.GetPluginClient();
        }
    );
    std::thread th2(
        [&plugin, &timeout](){
            int count = 2;
            while (--count) {
                plugin.CheckPluginRegister(timeout);
            }
            plugin.ChangeStatus(EX_PLUGIN_STATUS::ISREGISTERED);
            plugin.CheckPluginRegister(timeout);
        }
    );
    th1.join();
    th2.join();
    EXPECT_NE(client, nullptr);
    EXPECT_EQ(timeout, false);
}

/*
*用例名称：外部插件注册后获取thrift重试三次失败接口返回成功
*前置条件：1、相关配置已配置完成
*check点：在外部插件注册的情况下获取thrift客户端,判断是否重试三次
*/
TEST_F(ExternalPluginManagerTest, ExternalPlugin_after_register_can_get_client_retry_three_time_test_ok) {
    using namespace thriftservice::detail;
    using namespace thriftservice;
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(ExternalPlugin, GetThriftClientFromService), GetThriftClientFromServiceStub);
    g_countStart = 4;
    ExternalPlugin plugin("HDFS", "test1", false);
    plugin.m_pluginInfo.endPoint = "127.0.0.1";
    plugin.m_pluginInfo.port = 59610;
    plugin.ChangeStatus(EX_PLUGIN_STATUS::ISREGISTERED);
    std::shared_ptr<thriftservice::IThriftClient> client = plugin.GetPluginClient();
    EXPECT_EQ(client, nullptr);
    EXPECT_EQ(3, g_countStart);
}

/*
*用例名称：停止所有外置插件
*前置条件： 所有外置插件的停止接口调用均ok
*check点：函数成功调用
*/
TEST_F(ExternalPluginManagerTest, StopExPluginSuccess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginParse, GetPluginNameByAppType), StubGetPluginNameByAppTypeSucess);
    mp_stub.set(ADDR(CMpFile, FileExist), StubFileExistSucess);
    mp_stub.set(ADDR(CRootCaller, ExecUserDefineScript), StubExecSuccess);
    mp_stub.set(ADDR(ExternalPluginParse, GetStartUser), StubGetStartUserSuccess);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(mp_string))ADDR(CConfigXmlParser,Init), 
                StubCConfigXmlParserInitSuccess); 
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), 
                StubGetConfigValueInt32Success); 
    std::thread testT(ThreadUpdatePluginStatus);
    testT.detach();
    ExternalPluginManager::GetInstance().StartPlugin("NAS_File_System");
    ExternalPluginManager::GetInstance().StopAllPlugsHandle(SIGUSR1);
}

/*
*用例名称：external mgr初始化时停止NAS外置插件
*前置条件：1、所有外置插件的停止接口调用均ok
*check点：NAS插件PID被删除
*/
TEST_F(ExternalPluginManagerTest, KillExternalPlugsSuccess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
        const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_string agentRootPath = "./ProtectClient-E";
    mp_string PluginPath = "./ProtectClient-E/conf/PluginPid";
    mp_string nasPidPath = PluginPath + PATH_SEPARATOR + "NasPlugin";
    CPath::GetInstance().SetRootPath(agentRootPath);
    CMpFile::CreateDir(agentRootPath.c_str());
    CMpFile::CreateDir(PluginPath.c_str());
    CMpFile::CreateFile(nasPidPath);
    std::ofstream stream;
    stream.open(nasPidPath.c_str());
    stream << "123";
    stream.close();
    mp_stub.set(ADDR(CMpFile,  DirExist), StubFileExistSucess);
    ExternalPluginManager::GetInstance().KillExternalPlugs();
    mp_int32 iRet = CMpFile::FileExist(nasPidPath);
    EXPECT_EQ(MP_FALSE, iRet);
    CMpFile::DelDirAndSubFile(nasPidPath.c_str());
    CMpFile::DelDir(agentRootPath.c_str());
    CMpFile::DelDir(PluginPath.c_str());
}


/*
*用例名称：外部插件注册后通知ExternalPlugin获取thrift客户端
*前置条件：1、相关配置已配置完成
*check点：在外部插件注册的情况下获取thrift客户端,判断是否重试三次
*/
TEST_F(ExternalPluginManagerTest, ExternalPlugin_after_register_can_get_client_test_ok) {
    using namespace thriftservice::detail;
    using namespace thriftservice;
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(ExternalPlugin, GetThriftClientFromService), GetThriftClientFromServiceStub);
    
    ExternalPlugin plugin("HDFS", "test1", false);
    plugin.m_pluginInfo.endPoint = "127.0.0.1";
    plugin.m_pluginInfo.port = 59610;
    std::shared_ptr<thriftservice::IThriftClient> client;
    std::thread th1(
        [&plugin, &client](){
            client = plugin.GetPluginClient();
        }
    );

    std::thread th2(
        [&plugin](){
            std::this_thread::sleep_for(std::chrono::seconds(1));
            plugin.ChangeStatus(EX_PLUGIN_STATUS::ISREGISTERED);
        }
    );
    th1.join();
    th2.join();
    EXPECT_EQ(client, nullptr);
}

/*
*用例名称：外部插件注册后,定时检测插件状态
*前置条件：1、相关配置已配置完成
*check点：判断插件进程文件不存在后监控线程是否拉起插件
*/
TEST_F(ExternalPluginManagerTest, ExternalPluginManger_monotor_thread_check_plugin_thread_test_ok) {
    using namespace thriftservice::detail;
    using namespace thriftservice;
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(ExternalPlugin, GetThriftClientFromService), GetThriftClientFromServiceStub);
    mp_stub.set(ADDR(CMpFile, DirExist), StubDirExist);
    mp_stub.set(ADDR(ExternalPluginParse, GetStartUser), StubGetStartUserSuccess);
    mp_stub.set(ADDR(ExternalPlugin, ExecStartPlugin), StubExecStartPlugin);
    mp_stub.set(ADDR(ExternalPluginManager, IsPluginFolderExist), StubIsPluginFolderExist);
    g_countStart = 4;
    std::shared_ptr<ExternalPlugin> plugin= std::make_shared<ExternalPlugin>("HDFS", "test1", false, 59570);
    plugin->m_pluginInfo.endPoint = "127.0.0.1";
    plugin->m_pluginInfo.port = 59610;
    plugin->ChangeStatus(EX_PLUGIN_STATUS::ISREGISTERED);
    plugin->m_pluginInfo.processId = "12345";
    plugin->m_waitTimeout = 0;
    ExternalPluginManager manager;
    manager.m_pluginMap.insert({"HDFS", plugin});
    //std::shared_ptr<thriftservice::IThriftClient> client = plugin.GetPluginClient();
    manager.MonitorPlugin();
    std::shared_ptr<ExternalPlugin> newPlugin = manager.m_pluginMap["HDFS"];
    EXPECT_NE(newPlugin, nullptr);
    EXPECT_NE(newPlugin, plugin);
    //EXPECT_EQ(g_countStart, 1);
}

/*
*用例名称：外部插件超过超时时间后停止插件
*前置条件：1、相关配置已配置完成
*check点：判断插件已经长时间没有任务或者rest调用，停止插件，并从插件列表中清除
*/
TEST_F(ExternalPluginManagerTest, ExternalPluginManger_monitor_thread_plugin_notask_rest) {
    using namespace thriftservice::detail;
    using namespace thriftservice;
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(ExternalPlugin, GetThriftClientFromService), GetThriftClientFromServiceStub);
    mp_stub.set(ADDR(CMpFile,DirExist), StubDirExist);
    mp_stub.set(ADDR(ExternalPluginParse, GetStartUser), StubGetStartUserSuccess);
    mp_stub.set(ADDR(ExternalPlugin, ExecStartPlugin), StubExecStartPlugin);
    mp_stub.set(ADDR(ExternalPlugin, IsPluginProcessExist), StubPluginRunning);
    mp_stub.set(ADDR(ExternalPlugin, IsPluginResponding), StubPluginResponse);
    mp_stub.set(ADDR(ExternalPlugin, IsNoUseTimeout), StubPluginNoUsed);
    
    g_countStart = 4;
    std::shared_ptr<ExternalPlugin> plugin= std::make_shared<ExternalPlugin>("HDFS", "test1", false, 59570);
    plugin->m_pluginInfo.endPoint = "127.0.0.1";
    plugin->m_pluginInfo.port = 59610;
    plugin->ChangeStatus(EX_PLUGIN_STATUS::ISREGISTERED);
    plugin->m_pluginInfo.processId = "12345";
    plugin->m_waitTimeout = 0;
    ExternalPluginManager manager;
    manager.m_pluginMap.insert({"HDFS", plugin});
    
    EXPECT_EQ(1, manager.m_pluginMap.size());
    manager.MonitorPlugin();
    EXPECT_EQ(0, manager.m_pluginMap.size());
}

/*
*用例名称：调用插件CheckApplication接口成功
*前置条件：mock GetApplicationServiceClient
*check点：返回的errorCode为字符串，并且值为"0"
*/
TEST_F(ExternalPluginManagerTest, CheckPlugin_TEST) {
    using namespace thriftservice::detail;
    using namespace thriftservice;
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, GetPluginByRest), StubGetPluginByRest);
    mp_stub.set(ADDR(ExternalPlugin, GetPluginClient), StubGetPluginClient);
    mp_stub.set(ADDR(ExternalPluginManager, GetApplicationServiceClient), StubGetApplicationServiceClient);
    ExternalPluginManager manager;
    mp_string strAppType;
    Json::Value JsonReq;
    ApplicationEnvironment appEnv;
    StructToJson(appEnv, JsonReq["appEnv"]);
    Application productApp;
    StructToJson(productApp, JsonReq["application"]);
    CRequestMsg requestMsg;
    requestMsg.SetJsonData(JsonReq);
    CResponseMsg responseMsg;
    EXPECT_EQ(MP_SUCCESS, manager.CheckPlugin(strAppType, requestMsg, responseMsg));
    EXPECT_EQ("0", responseMsg.GetJsonValueRef()["errorCode"].asString());
}

/*
*用例名称：查询本机及远端应用集群接口成功
*前置条件：mock GetApplicationServiceClient
*check点：1. 查询本机集群成功， 返回ApplicationEnvironment，未抛出异常
* 2. 查询远端集群成功， 返回ApplicationEnvironment，Appliation, 未抛出异常
*/
TEST_F(ExternalPluginManagerTest, DiscoverCluster_TEST) {
    using namespace thriftservice::detail;
    using namespace thriftservice;
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, GetPluginByRest), StubGetPluginByRest);
    mp_stub.set(ADDR(ExternalPlugin, GetPluginClient), StubGetPluginClient);
    mp_stub.set(ADDR(ExternalPluginManager, GetApplicationServiceClient), StubGetApplicationServiceClient);
    ExternalPluginManager manager;
    mp_string strAppType;
    Json::Value JsonReq;
    ApplicationEnvironment appEnv;
    StructToJson(appEnv, JsonReq["appEnv"]);
    Application productApp;
    StructToJson(productApp, JsonReq["application"]);
    CRequestMsg requestMsg;
    requestMsg.SetJsonData(JsonReq);
    CResponseMsg responseMsg;
    EXPECT_EQ(manager.QueryRemoteCluster(strAppType, requestMsg, responseMsg), MP_SUCCESS);
}

/*
*用例名称：调用插件ListApplicationV2接口成功
*前置条件：mock GetApplicationServiceClient
*check点：返回的errorCode为字符串，并且值为"0"
*/
TEST_F(ExternalPluginManagerTest, QueryPluginDetailV2_TEST) {
    using namespace thriftservice::detail;
    using namespace thriftservice;
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, GetPluginByRest), StubGetPluginByRest);
    mp_stub.set(ADDR(ExternalPlugin, GetPluginClient), StubGetPluginClient);
    mp_stub.set(ADDR(ExternalPluginManager, GetApplicationServiceClient), StubGetApplicationServiceClient);
    ExternalPluginManager manager;

    mp_string strAppType;
    Json::Value JsonReq;
    ApplicationEnvironment appEnv;
    std::vector<Application> applications;
    Application tmps;
    applications.push_back(tmps);
    StructToJson(appEnv, JsonReq["appEnv"]);
    for (int index = 0; index < applications.size(); ++index) {
        Json::Value temp;
        StructToJson(applications[index], temp);
        JsonReq["applications"].append(temp);
    }
    mp_int32 page_no = 0;
    mp_int32 page_size = 0;
    JsonHelper::TypeToJsonValue(page_no, JsonReq["pageNo"]);
    JsonHelper::TypeToJsonValue(page_size, JsonReq["pageSize"]);
    mp_string orders;
    mp_string conditions;
    JsonHelper::TypeToJsonValue(orders, JsonReq["orders"]);
    JsonHelper::TypeToJsonValue(conditions, JsonReq["conditions"]);
    CRequestMsg requestMsg;
    requestMsg.SetJsonData(JsonReq);
    CResponseMsg responseMsg;
    EXPECT_EQ(MP_SUCCESS, manager.QueryPluginDetailV2(strAppType, requestMsg, responseMsg));
    EXPECT_EQ("0", responseMsg.GetJsonValueRef()["errorCode"].asString());
}

/*
*用例名称：调用插件ListApplicationConfig接口成功
*前置条件：mock GetApplicationServiceClient
*check点：返回的errorCode为字符串，并且值为"0"
*/
TEST_F(ExternalPluginManagerTest, QueryPluginConfig_TEST) {
    using namespace thriftservice::detail;
    using namespace thriftservice;
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, GetPluginByRest), StubGetPluginByRest);
    mp_stub.set(ADDR(ExternalPlugin, GetPluginClient), StubGetPluginClient);
    mp_stub.set(ADDR(ExternalPluginManager, GetApplicationServiceClient), StubGetApplicationServiceClient);
    ExternalPluginManager manager;

    mp_string strAppType;
    CRequestMsg requestMsg;
    requestMsg.m_url.m_queryParam["script"] = "hana";
    CResponseMsg responseMsg;
    EXPECT_EQ(MP_SUCCESS, manager.QueryPluginConfig(strAppType, requestMsg, responseMsg));
} 
/*
*用例名称：调用插件ListApplicationConfig接口成功
*前置条件：mock GetApplicationServiceClient
*check点：返回的errorCode为字符串，并且值为"0"
*/
TEST_F(ExternalPluginManagerTest, QueryPluginConfig_TEST1) {
    using namespace thriftservice::detail;
    using namespace thriftservice;
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, GetPluginByRest), StubGetPluginByRest);
    mp_stub.set(ADDR(ExternalPlugin, GetPluginClient), StubGetPluginClientNullptr);
    mp_stub.set(ADDR(ExternalPluginManager, GetApplicationServiceClient), StubGetApplicationServiceClient);
    ExternalPluginManager manager;

    mp_string strAppType;
    CRequestMsg requestMsg;
    requestMsg.m_url.m_queryParam["script"] = "hana";
    CResponseMsg responseMsg;
    EXPECT_EQ(MP_FAILED, manager.QueryPluginConfig(strAppType, requestMsg, responseMsg));
}

/*
*用例名称：调用插件ListApplicationConfig接口成功
*前置条件：mock GetApplicationServiceClient
*check点：返回的errorCode为字符串，并且值为"0"
*/
TEST_F(ExternalPluginManagerTest, QueryPluginConfig_TEST2) {
    using namespace thriftservice::detail;
    using namespace thriftservice;
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, GetPluginByRest), StubGetPluginByRestNullptr);
    mp_stub.set(ADDR(ExternalPlugin, GetPluginClient), StubGetPluginClient);
    mp_stub.set(ADDR(ExternalPluginManager, GetApplicationServiceClient), StubGetApplicationServiceClient);
    ExternalPluginManager manager;

    mp_string strAppType;
    CRequestMsg requestMsg;
    requestMsg.m_url.m_queryParam["script"] = "hana";
    CResponseMsg responseMsg;
    EXPECT_EQ(MP_FAILED, manager.QueryPluginConfig(strAppType, requestMsg, responseMsg));
}







/*
*用例名称：调用插件ListApplicationConfig接口成功
*前置条件：mock GetApplicationServiceClient
*check点：返回的errorCode为字符串，并且值为"0"
*/
TEST_F(ExternalPluginManagerTest, RemoveProtectTEST) {
    using namespace thriftservice::detail;
    using namespace thriftservice;
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, GetPluginByRest), StubGetPluginByRestNullptr);
    mp_stub.set(ADDR(ExternalPlugin, GetPluginClient), StubGetPluginClient);
    mp_stub.set(ADDR(ExternalPluginManager, GetApplicationServiceClient), StubGetApplicationServiceClient);
    ExternalPluginManager manager;

    mp_string strAppType;
    CRequestMsg requestMsg;
    requestMsg.m_url.m_queryParam["script"] = "hana";
    CResponseMsg responseMsg;
    EXPECT_EQ(MP_FAILED, manager.RemoveProtect(strAppType, requestMsg, responseMsg));
}

/*
*用例名称：调用插件ListApplicationConfig接口成功
*前置条件：mock GetApplicationServiceClient
*check点：返回的errorCode为字符串，并且值为"0"
*/
TEST_F(ExternalPluginManagerTest, RemoveProtectTEST1) {
    using namespace thriftservice::detail;
    using namespace thriftservice;
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerTestLogVoid);
    mp_stub.set(ADDR(ExternalPluginManager, GetPluginByRest), StubGetPluginByRest);
    mp_stub.set(ADDR(ExternalPlugin, GetPluginClient), StubGetPluginClientNullptr);
    mp_stub.set(ADDR(ExternalPluginManager, GetApplicationServiceClient), StubGetApplicationServiceClient);
    ExternalPluginManager manager;

    mp_string strAppType;
    CRequestMsg requestMsg;
    requestMsg.m_url.m_queryParam["script"] = "hana";
    CResponseMsg responseMsg;
    EXPECT_EQ(MP_FAILED, manager.RemoveProtect(strAppType, requestMsg, responseMsg));
}