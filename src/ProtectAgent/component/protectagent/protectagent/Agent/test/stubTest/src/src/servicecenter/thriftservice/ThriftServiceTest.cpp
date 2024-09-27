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
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/concurrency/ThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/processor/TMultiplexedProcessor.h>
#include <thrift/server/TThreadedServer.h>

#include "servicefactory/detail/ServiceCenter.h"
#include "servicefactory/include/ServiceFactory.h"
#include "servicefactory/include/IService.h"
#include "servicefactory/include/IServiceCenter.h"
#include "thriftservice/include/IThriftService.h"
#include "thriftservice/ThriftServiceTest.h"
#include "thriftservice/include/IThriftClient.h"
#include "thriftservice/detail/ThriftClient.h"
#include "thriftservice/detail/ThriftServer.h"
#include "thriftservice/detail/ThriftFactory.h"
#include "thriftservice/detail/ThriftService.h"
#include "thriftservice/detail/ThreadProxy.h"
#include "thriftservice/detail/TServerEventHandlerImpl.h"
#include "certificateservice/include/ICertificateService.h"
#include "certificateservice/detail/CertificateService.h"

#include "common/ConfigXmlParse.h"
#include "securecom/CryptAlg.h"

#include "TaskService.h"
#include "MountService.h"

using namespace servicecenter;
using namespace thriftservice;
using namespace thriftservice::detail;
using namespace certificateservice;
using namespace certificateservice::detail;

using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using namespace apache::thrift::server;
using namespace thrifttest;

namespace {
mp_void LogTest()
{}
#define DoLogTest()                                                                                                    \
    do {                                                                                                               \
        stub11.set(ADDR(CLogger, Log), LogTest);                                                                         \
    } while (0)

auto StubgetState()
{
    return Thread::uninitialized;
}
void Stubstart()
{
    return;
}
}  // namespace

class TaskServiceHandler : public TaskServiceIf {
public:
    TaskServiceHandler()
    {
        m_tasks.emplace(1, "Task1");
        m_tasks.emplace(2, "Task2");
        m_tasks.emplace(3, "Task3");
    }

    virtual void GetTask(Task& task, int64_t id) {
        auto it = m_tasks.find(id);
        if (it != m_tasks.end()) {
            task.jobID = it->first;
            task.info = it->second;
        }
    }
private:
    std::map<int64_t, std::string> m_tasks;
};

class MountServiceHandler : public MountServiceIf {
public:
    virtual int64_t Mount(const Config& config, const std::string& path)
    {
        if (m_paths.find(path) == m_paths.end()) {
            m_paths.emplace(path);
            return 1;
        }
        return 0;
    }
  virtual int64_t Unmount(const std::string& path) {
        auto it = m_paths.find(path);
        if (it != m_paths.end()) {
            m_paths.erase(it);
            return 1;
        }
        return 0;
  }
private:
    std::set<std::string> m_paths;
};

std::shared_ptr<Thread> ThriftFactoryStub(std::shared_ptr<Runnable> runner, std::shared_ptr<std::promise<bool>> p)
{
    return nullptr;
}

void ThriftServerStartStub()
{
    return;
}
bool ThriftClientStub()
{
    return true;
}
static
std::shared_ptr<ThriftServer> GetSslServerStub() 
{
    return std::make_shared<ThriftServer>();
}

class TestServiceStub : public IService{
public:
    virtual ~TestServiceStub(){}

    virtual bool Initailize() {
        m_inited = true;
        return true;
    }
    virtual bool Uninitailize() {
        m_inited = false;
        return true;
    }
private:
    bool m_inited {false};
};

static bool InitPathAndLog() 
{
    std::string strRootPath = "../../..";
    CPath::GetInstance().SetRootPath(strRootPath);
    auto path = CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF);
    auto ret = CConfigXmlParser::GetInstance().Init(path);
    if (ret != MP_SUCCESS) {
        return false;
    }
    return true;
}

void threadMainProxyStub(std::shared_ptr<Thread> thread)
{
    auto ptr = std::dynamic_pointer_cast<ThreadProxy>(thread);
    if (ptr) {
        ptr->Notify();
    }  
}

class FakeRunnable : public apache::thrift::concurrency::Runnable
{
public:
    void run()
    {
        return;
    }

};

/*
* 用例名称：thrift服务默认注册
* 前置条件：1、通过服务中心获取thrift服务
* check点：1、检查获取服务对象不为空指针
*/
TEST_F(ThriftServiceTest, servicefactory_get_default_thrift_service_test_true)
{
    auto ret = ServiceFactory::GetInstance()->Register<ThriftService>("IThriftService");
    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    EXPECT_NE(thriftservice, nullptr);
}

/*
* 用例名称：thrift服务获取服务端
* 前置条件：1、通过服务中心获取thrift服务
* check点：1、检查获取thrift服务端是否为空
*/
TEST_F(ThriftServiceTest, thriftservice_register_server_test_true)
{
    stub11.set(&ThriftServer::StartThread, ThriftServerStartStub);
    stub11.set(&ThriftFactory::GetThread, ThriftFactoryStub);

    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    auto thriftserver = thriftservice->RegisterServer("localhost", 9090);
    EXPECT_NE(thriftserver, nullptr);
    thriftservice->UnRegisterServer("localhost", 9090);
}


/*
* 用例名称：thrift服务获取ssl服务端
* 前置条件：1、通过服务中心获取thrift服务
* check点：1、检查获取thrift ssl服务端是否为空
*/
TEST_F(ThriftServiceTest, thriftservice_register_ssl_server_test_true)
{
    DoLogTest();
    stub11.set(&ThriftServer::StartThread, ThriftServerStartStub);
    stub11.set(&ThriftFactory::GetThread, ThriftFactoryStub);
    stub11.set(&ThriftFactory::GetSslServer, GetSslServerStub);

    InitPathAndLog();

    auto ret = ServiceFactory::GetInstance()->Register<CertificateService>("ICertificateService");

    auto certificateservice = ServiceFactory::GetInstance()->GetService<ICertificateService>("ICertificateService");
    auto handler = certificateservice->GetCertificateHandler();

    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    auto thriftserver = thriftservice->RegisterSslServer("localhost", 9090, handler);
    EXPECT_NE(thriftserver, nullptr);
    thriftservice->UnRegisterServer("localhost", 9090);
}

/*
* 用例名称：thrift服务多次获取服务端
* 前置条件：1、通过服务中心获取thrift服务 2、多次注册同一端口的服务端
* check点：1、检查第二次获取thrift服务端是否为空
*/
TEST_F(ThriftServiceTest, thriftservice_register_twice_server_test_false)
{
    DoLogTest();
    stub11.set(&ThriftServer::StartThread, ThriftServerStartStub);
    stub11.set(&ThriftFactory::GetThread, ThriftFactoryStub);
    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    auto thriftserver = thriftservice->RegisterServer("localhost", 9090);
    auto thriftserver1 = thriftservice->RegisterServer("localhost", 9090);
    EXPECT_EQ(thriftserver1, nullptr);
    thriftservice->UnRegisterServer("localhost", 9090);
}

/*
* 用例名称：thrift服务端注册Processor
* 前置条件：1、通过thrift服务获取thrift服务端
* check点：1、检查获取thrift服务端注册Processor接口返回值为true
*/
TEST_F(ThriftServiceTest, thriftservice_register_processor_test_true)
{
    DoLogTest();
    stub11.set(&ThriftServer::StartThread, ThriftServerStartStub);
    stub11.set(&ThriftFactory::GetThread, ThriftFactoryStub);

    auto handler1 = std::shared_ptr<TaskServiceHandler>(new TaskServiceHandler());
    auto processor1 = std::shared_ptr<TProcessor>(new TaskServiceProcessor(handler1));
    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    auto thriftserver = thriftservice->RegisterServer("localhost", 9090);
    auto ret = thriftserver->RegisterProcessor("TaskService", processor1);
    EXPECT_EQ(ret, true);
    thriftservice->UnRegisterServer("localhost", 9090);
}


/*
* 用例名称：thrift服务统一端口多次注册服务注册
* 前置条件：1、通过thrift服务多次注册服务
* check点：1、检查注册服务接口接口返回值为true
*/
TEST_F(ThriftServiceTest, thriftservice_double_register_server_test_true)
{
    DoLogTest();
    typedef bool (*pTServer)(ThriftServer*,bool);
    pTServer pClientAddr = (pTServer)(&ThriftServer::Stop);
    stub11.set(pClientAddr, ThriftClientStub);

    pClientAddr = (pTServer)(&ThriftServer::Start);
    stub11.set(pClientAddr, ThriftClientStub);
    
    stub11.set(&ThriftFactory::GetThread, ThriftFactoryStub);

    auto handler1 = std::shared_ptr<TaskServiceHandler>(new TaskServiceHandler());
    auto processor1 = std::shared_ptr<TProcessor>(new TaskServiceProcessor(handler1));
    auto handler2 = std::shared_ptr<MountServiceHandler>(new MountServiceHandler());
    auto processor2 = std::shared_ptr<TProcessor>(new MountServiceProcessor(handler2));

    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    auto thriftserver = thriftservice->RegisterServer("localhost", 9090);
    auto ret1 = thriftserver->RegisterProcessor("TaskService", processor1);
    ret1 = thriftserver->RegisterProcessor("MountService", processor2);
    EXPECT_EQ(ret1, true);
    thriftservice->UnRegisterServer("localhost", 9090);
}

/*
* 用例名称：thrift服务注册thrift客户端对象
* 前置条件：1、通过thrift服务注册客户端对象
* check点：1、检查注册客户服务对象不为空
*/
TEST_F(ThriftServiceTest, thriftservice_register_client_test_true)
{
    DoLogTest();
    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    thriftservice::ClientSocketOpt opt = { "localhost", 9090 };
    auto thriftclient = thriftservice->RegisterClient(opt);
    auto taskclient = thriftclient->GetClientIf<TaskServiceClient>("TaskService");
    EXPECT_NE(taskclient, nullptr);
    thriftservice->UnRegisterClient("localhost", 9090);
}

/*
* 用例名称：thrift客户端注册多个客户端serivce interface
* 前置条件：1、通过thrift服务注册客户端对象
* check点：1、检查注册客户服务对象不为空
*/
TEST_F(ThriftServiceTest, thriftservice_register_client_twice_test_true)
{
    DoLogTest();
    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    thriftservice::ClientSocketOpt opt = { "localhost", 9090 };
    auto thriftclient = thriftservice->RegisterClient(opt);
    auto taskclient = thriftclient->GetClientIf<TaskServiceClient>("TaskService");
    auto mountclient = thriftclient->GetClientIf<MountServiceClient>("MountService");
    EXPECT_NE(taskclient, nullptr);
    EXPECT_NE(mountclient, nullptr);
    thriftservice->UnRegisterClient("localhost", 9090);
}

/*
* 用例名称：thrift客户端对象启动接口
* 前置条件：1、通过thrift服务注册客户端对象 2、调用客户端启动接口
* check点：1、检查用例名称：thrift客户端对象启动接口返回值为true
*/
TEST_F(ThriftServiceTest, thriftservice_start_client_test_true)
{
    DoLogTest();
    typedef bool (*pTClient)(ThriftClient*,bool);
    pTClient pClientAddr = (pTClient)(&ThriftClient::Start);
    stub11.set(pClientAddr, ThriftClientStub);

    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    thriftservice::ClientSocketOpt opt = { "localhost", 9090 };
    auto thriftclient = thriftservice->RegisterClient(opt);
    auto taskclient = thriftclient->GetClientIf<TaskServiceClient>("TaskService");
    auto ret = thriftclient->Start();

    EXPECT_EQ(ret, true);
    thriftservice->UnRegisterClient("localhost", 9090);
}

/*
* 用例名称：监听线程抛出异常测试
* 前置条件：1、实例化ThriftServer
* check点：1、ThreadProxy::threadMainProxy接口抛出异常后ThriftServer Start接口返回false
*/
TEST_F(ThriftServiceTest, thriftservice_start_listen_error_test_ok)
{
    DoLogTest();
    stub11.set(&ThriftServer::StartThread, ThriftServerStartStub);
    stub11.set(&ThriftFactory::GetThread, ThriftFactoryStub);
    stub11.set(&ThreadProxy::threadMainProxy, threadMainProxyStub);

    std::shared_ptr<std::promise<bool>> promise = std::make_shared<std::promise<bool>>();
    std::shared_ptr<TServerEventHandlerImpl> handler = std::make_shared<TServerEventHandlerImpl>();
    handler->SetPromise(promise);

    std::shared_ptr<FakeRunnable> r = std::make_shared<FakeRunnable>();
    std::shared_ptr<ThreadProxy> thProxy = std::make_shared<ThreadProxy>(true, r);
    thProxy->SetPromise(promise);

    std::shared_ptr<ThriftServer> thrift = std::make_shared<ThriftServer>();

    thrift->m_future = promise->get_future();
    thProxy->threadMainProxy(thProxy);
    auto ret = thrift->Start();
    
    EXPECT_EQ(ret, false);
}

/*
* 用例名称：监听线程正常运行没有抛出异常
* 前置条件：1、实例化ThriftServer
* check点：1、TServerEventHandlerImpl::preServe接口运行后ThriftServer Start接口返回true
*/
TEST_F(ThriftServiceTest, thriftservice_start_listen_ok_test_ok)
{
    DoLogTest();
    stub11.set(&ThriftServer::StartThread, ThriftServerStartStub);
    stub11.set(&ThriftFactory::GetThread, ThriftFactoryStub);
    stub11.set(&ThreadProxy::threadMainProxy, threadMainProxyStub);

    std::shared_ptr<std::promise<bool>> promise = std::make_shared<std::promise<bool>>();
    std::shared_ptr<TServerEventHandlerImpl> handler = std::make_shared<TServerEventHandlerImpl>();
    handler->SetPromise(promise);

    std::shared_ptr<FakeRunnable> r = std::make_shared<FakeRunnable>();
    std::shared_ptr<ThreadProxy> thProxy = std::make_shared<ThreadProxy>(true, r);
    thProxy->SetPromise(promise);

    std::shared_ptr<ThriftServer> thrift = std::make_shared<ThriftServer>();

    thrift->m_future = promise->get_future();
    thrift->m_thread = thProxy;
    handler->preServe();
    auto ret = thrift->Start();
    
    EXPECT_EQ(ret, true);
}

/*
* 用例名称：thrift服务注册线程安全thrift客户端对象
* 前置条件：1、通过thrift服务注册客户端对象
* check点：1、检查注册客户服务对象不为空
*/
TEST_F(ThriftServiceTest, thriftservice_register_concurrent_client_test_true)
{
    DoLogTest();
    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    thriftservice::ClientSocketOpt opt = { "localhost", 9090 };
    auto thriftclient = thriftservice->RegisterClient(opt);
    auto taskclient = thriftclient->GetConcurrentClientIf<TaskServiceConcurrentClient>("TaskService");
    EXPECT_NE(taskclient, nullptr);
    thriftservice->UnRegisterClient("localhost", 9090);
}

/*
* 用例名称：thrift服务每次获取客户端都重新生成
* 前置条件：1、通过thrift服务注册客户端对象
* check点：1、检查两次客户服务对象是否为同一个对象
*/
TEST_F(ThriftServiceTest, thriftservice_register_mutil_time_client_test_client_is_not_same)
{
    DoLogTest();
    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    thriftservice::ClientSocketOpt opt = { "localhost", 9090 };
    auto thriftclient1 = thriftservice->RegisterClient(opt);
    auto thriftclient2 = thriftservice->RegisterClient(opt);
    EXPECT_NE(thriftclient1, thriftclient2);
    thriftservice->UnRegisterClient("localhost", 9090);
}

TEST_F(ThriftServiceTest, ThriftServer_RegisterProcessor)
{
    DoLogTest();
    Stub stub;
    ThriftServer thriftServer;
    std::string name;
    std::shared_ptr<TProcessor> processor = nullptr;

    mp_bool ret = thriftServer.RegisterProcessor(name, processor);
    EXPECT_EQ(ret, false);
}