#include "apps/appprotect/plugininterface/SecurityServiceHandlerTest.h"
#include "apps/appprotect/plugininterface/SecurityServiceHandler.h"
#include "servicecenter/messageservice/include/IObserver.h"
#include "pluginfx/ExternalPluginManager.h"
#include "common/Log.h"
#include "message/curlclient/CurlHttpClient.h"
#include <iostream>

using namespace AppProtect;
using namespace servicecenter;
using namespace messageservice;

namespace {
mp_void LogTest()
{}
#define DoLogTest()                                                                                                    \
    do {                                                                                                               \
        stub.set(ADDR(CLogger, Log), LogTest);                                                                         \
    } while (0)
}  // namespace

void StubTRegisterObserverSuccess(messageservice::EVENT_TYPE type, std::shared_ptr<IObserver> observer)
{
    return;
}

/*
*用例名称：证书指纹获取校验
*前置条件：无
*check点：成功校验证书指纹
*/
TEST_F(SecurityServiceHandlerTest, CheckCertThumbPrintFailed) 
{
    DoLogTest();
    stub.set(&ExternalPluginManager::RegisterObserver, StubTRegisterObserverSuccess);
    std::shared_ptr<IService> tempService = 
        ServiceFactory::GetInstance()->GetService<IService>("SecurityService");
    std::shared_ptr<SecurityServiceHandler> handler(std::dynamic_pointer_cast<SecurityServiceHandler>(tempService));
    ActionResult ret;
    handler->CheckCertThumbPrint(ret, "127.0.0.1", 8088, "TEST");
    EXPECT_EQ(ret.code, MP_FAILED);
    EXPECT_EQ(handler->Uninitailize(), true);
    auto tmpThriftService = ServiceFactory::GetInstance()->GetService<thriftservice::detail::ThriftService>("IThriftService");
    std::shared_ptr<thriftservice::IThriftServer> tmpThriftServiceNew =
        std::dynamic_pointer_cast<thriftservice::IThriftServer>(tmpThriftService);
    std::shared_ptr<messageservice::RpcPublishEvent> event =
        std::make_shared<messageservice::RpcPublishEvent>(tmpThriftServiceNew);
    handler->Update(event);
}