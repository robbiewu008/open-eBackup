#define private public
#define protected public

#include "servicecenter/servicefactory/detail/ServiceCenter.h"
#include "servicecenter/servicefactory/include/ServiceFactory.h"
#include "servicecenter/servicefactory/include/IService.h"
#include "servicecenter/servicefactory/include/IServiceCenter.h"
#include "servicecenter/servicefactory/ServiceCenterTest.h"

using namespace servicecenter;
using namespace servicecenter::detail;

class AutoResetG_Instance{
public:
    ~AutoResetG_Instance() {
        IServiceCenter::g_instance = nullptr;
    }
};

class AutoInstall_Instance{
public:
    ~AutoInstall_Instance() {
        ServiceFactory::GetServiceCenter()->Initailize();
    }
};

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


/*
* 用例名称：服务中心初始化
* 前置条件：1、构造服务中心对象 2、调用服务中心的Initailize接口
* check点：1、检查Initailize接口返回值为true
*/
TEST_F(ServiceCenterTest, servicecenter_init_test_true)
{   
    {
        AutoResetG_Instance autoreset;
    }
    std::shared_ptr<ServiceCenter> center = std::make_shared<ServiceCenter>();
    EXPECT_EQ(center->Initailize(), true);
}

/*
* 用例名称：服务中心多次初始化返回失败
* 前置条件：1、构造服务中心对象 2、多次调用服务中心的Initailize接口
* check点：1、检查第二次Initailize接口返回值为false
*/
TEST_F(ServiceCenterTest, servicecenter_init_test_false_when_double_init)
{
    {
        AutoResetG_Instance autoreset;
    }
    std::shared_ptr<ServiceCenter> center = std::make_shared<ServiceCenter>();
    center->Initailize();
    EXPECT_EQ(center->Initailize(), false);
    AutoResetG_Instance autoreset;
}

/*
* 用例名称：服务中心注册IService的子类对象返回true
* 前置条件：1、注册服务到服务中心
* check点：1、检查服务的注册返回值为true
*/
TEST_F(ServiceCenterTest, service_register_return_true_when_T_nheritance_from_IService)
{
    {
        AutoResetG_Instance autoreset;
    }
    {
        AutoInstall_Instance autoreset;
    }
    auto ret = ServiceFactory::GetInstance()->Register<TestServiceStub>("TestServiceStub");

    EXPECT_EQ(ret, true);
}

/*
* 用例名称：服务中心获取到服务后服务已经初始化
* 前置条件：1、注册服务到服务中心 2、通过服务中心获取服务
* check点：1、检查服务的初始化状态为已初始化
*/
TEST_F(ServiceCenterTest, service_init_status_test_true_when_getservice_from_servicefactory)
{
    {
        AutoResetG_Instance autoreset;
    }
    {
        AutoInstall_Instance autoreset;
    }
    ServiceFactory::GetInstance()->Register<TestServiceStub>("TestServiceStub");
    auto service = ServiceFactory::GetInstance()->GetService<TestServiceStub>("TestServiceStub");

    EXPECT_EQ(service->m_inited, true);
}

/*
* 用例名称：服务中心多次注册服务
* 前置条件：1、多次注册服务到服务中心
* check点：1、检查第二次注册服务后的返回值为false
*/
TEST_F(ServiceCenterTest, service_register_test_false_when_double_register_servicefactory)
{
    {
        AutoResetG_Instance autoreset;
    }
    {
        AutoInstall_Instance autoreset;
    }
    bool ret = ServiceFactory::GetInstance()->Register<TestServiceStub>("TestServiceStub");
    ret = ServiceFactory::GetInstance()->Register<TestServiceStub>("TestServiceStub");

    EXPECT_EQ(ret, false);
}

/*
* 用例名称：未注册服务时服务中心无法获取到服务
* 前置条件：1、通过服务中心获取为注册服务
* check点：1、检查获取服务对象为空指针
*/
TEST_F(ServiceCenterTest, getservice_test_false_when_no_register_service)
{
    {
        AutoResetG_Instance autoreset;
    }
    {
        AutoInstall_Instance autoreset;
    }
    auto service = ServiceFactory::GetInstance()->GetService<TestServiceStub>("TestServiceStub");

    EXPECT_EQ(service, nullptr);
}

/*
* 用例名称：取消注册服务后服务中心无法获取到服务
* 前置条件：1、通过服务中心获取为注册服务 2、取消服务注册 3、获取服务
* check点：1、检查获取服务对象为空指针
*/
TEST_F(ServiceCenterTest, getservice_test_false_when_unregister_service)
{
    {
        AutoResetG_Instance autoreset;
    }
    {
        AutoInstall_Instance autoreset;
    }
    ServiceFactory::GetInstance()->Register<TestServiceStub>("TestServiceStub");
    auto service = ServiceFactory::GetInstance()->GetService<TestServiceStub>("TestServiceStub");

    EXPECT_NE(service, nullptr);
    ServiceFactory::GetInstance()->Unregister("TestServiceStub");
    service = ServiceFactory::GetInstance()->GetService<TestServiceStub>("TestServiceStub");
    EXPECT_EQ(service, nullptr);
}
