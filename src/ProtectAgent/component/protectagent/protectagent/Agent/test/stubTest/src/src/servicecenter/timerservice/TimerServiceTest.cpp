#define private public
#define protected public


#include "timerservice/TimerServiceTest.h"
#include "servicefactory/include/ServiceFactory.h"
#include "servicecenter/timerservice/detail/TimerService.h"

using namespace servicecenter;
using namespace timerservice;
using namespace timerservice::detail;

static int count = 0;

bool TimerFunc(){
    ++count;
    return true;
}

bool TimerOnceFunc(){
    ++count;
    return false;
}

/*
* 用例名称：获取默认timer服务句柄
* 前置条件：1、timer服务已注册到服务中心
* check点：1、默认timer服务句柄不为空
*/
TEST_F(TimerServiceTest, timere_service_get_timer_test_not_null)
{   
    auto ret = ServiceFactory::GetInstance()->Register<TimerService>("ITimerService");
    std::shared_ptr<ITimerService> service = ServiceFactory::GetInstance()->GetService<ITimerService>("ITimerService");
    auto timer = service->CreateTimer();
    EXPECT_NE(timer, nullptr);
    ServiceFactory::GetInstance()->Unregister("ITimerService");
}

/*
* 用例名称：删除Timer句柄
* 前置条件：1、timer服务已注册到服务中心
* check点：1、删除Timer句柄成功
*/
TEST_F(TimerServiceTest, timere_service_del_timer_test_ok)
{   
    auto ret = ServiceFactory::GetInstance()->Register<TimerService>("ITimerService");
    std::shared_ptr<ITimerService> service = ServiceFactory::GetInstance()->GetService<ITimerService>("ITimerService");
    auto timer = service->CreateTimer();
    std::shared_ptr<TimerService> timeservice = std::dynamic_pointer_cast<TimerService>(service);
    EXPECT_EQ(timeservice->m_timers.size(), 1);
    service->DeleteTimer(timer);
    EXPECT_EQ(timeservice->m_timers.size(), 0);
    ServiceFactory::GetInstance()->Unregister("ITimerService");
}

/*
* 用例名称：未启动Timer线程Stop无异常
* 前置条件：1、timer服务已注册到服务中心
* check点：1、Stop接口返回true
*/
TEST_F(TimerServiceTest, timere_service_timer_stop_test_not_null)
{   
    auto ret = ServiceFactory::GetInstance()->Register<TimerService>("ITimerService");
    std::shared_ptr<ITimerService> service = ServiceFactory::GetInstance()->GetService<ITimerService>("ITimerService");
    auto timer = service->CreateTimer();
    ret = timer->Stop();
    EXPECT_EQ(ret, true);
    ServiceFactory::GetInstance()->Unregister("ITimerService");
}

/*
* 用例名称：Timer回调返回true下个时间点继续执行
* 前置条件：1、Timer线程已启动，Timer回调已注册
* check点：1、Timer回调次数大于1
*/
TEST_F(TimerServiceTest, timer_service_timer_func_run_multi_time)
{   
    auto ret = ServiceFactory::GetInstance()->Register<TimerService>("ITimerService");
    std::shared_ptr<ITimerService> service = ServiceFactory::GetInstance()->GetService<ITimerService>("ITimerService");
    auto timer = service->CreateTimer();
    count = 0;
    timer->Start();
    timer->AddTimeoutExecutor([]()->bool{
        return TimerFunc();
    }, 100);
    sleep(1);
    timer->Stop();
    EXPECT_GE(count, 8);
    EXPECT_LE(count, 11);
    ServiceFactory::GetInstance()->Unregister("ITimerService");
}

/*
* 用例名称：Timer回调返回false下个时间点不再执行
* 前置条件：1、Timer线程已启动，Timer回调已注册
* check点：1、Timer回调次数等于1
*/
TEST_F(TimerServiceTest, timer_service_timer_func_run_one_time)
{   
    auto ret = ServiceFactory::GetInstance()->Register<TimerService>("ITimerService");
    std::shared_ptr<ITimerService> service = ServiceFactory::GetInstance()->GetService<ITimerService>("ITimerService");
    auto timer = service->CreateTimer();
    count = 0;
    timer->Start();
    timer->AddTimeoutExecutor([]()->bool{
        return TimerOnceFunc();
    }, 100);
    sleep(1);
    timer->Stop();
    EXPECT_EQ(count, 1);
    ServiceFactory::GetInstance()->Unregister("ITimerService");
}

/*
* 用例名称：Timer删除回调函数后不再执行回调
* 前置条件：1、Timer线程已启动，Timer回调已注册，然后删除回调
* check点：1、Timer回调次数大于1
*/
TEST_F(TimerServiceTest, timer_service_timer_func_remove_test_not_run_again)
{   
    auto ret = ServiceFactory::GetInstance()->Register<TimerService>("ITimerService");
    std::shared_ptr<ITimerService> service = ServiceFactory::GetInstance()->GetService<ITimerService>("ITimerService");
    auto timer = service->CreateTimer();
    count = 0;
    timer->Start();
    auto id = timer->AddTimeoutExecutor([]()->bool{
        return TimerFunc();
    }, 100);
    usleep(500000);
    timer->RemoveTimeoutExecutor(id);
    usleep(300000);
    timer->Stop();
    EXPECT_GE(count, 3);
    EXPECT_LE(count, 8);
    ServiceFactory::GetInstance()->Unregister("ITimerService");
}

/*
* 用例名称：多个Timer回调执行OK
* 前置条件：1、Timer线程已启动，多个Timer回调已注册
* check点：1、Timer回调次数大于1
*/
TEST_F(TimerServiceTest, timer_service_multi_timer_func_run_test_ok)
{   
    auto ret = ServiceFactory::GetInstance()->Register<TimerService>("ITimerService");
    std::shared_ptr<ITimerService> service = ServiceFactory::GetInstance()->GetService<ITimerService>("ITimerService");
    auto timer = service->CreateTimer();
    count = 0;
    timer->Start();
    timer->AddTimeoutExecutor([]()->bool{
        return TimerFunc();
    }, 100);
    timer->AddTimeoutExecutor([]()->bool{
        return TimerOnceFunc();
    }, 200);
    sleep(1);
    timer->Stop();
    EXPECT_GE(count, 9);
    EXPECT_LE(count, 12);
    ServiceFactory::GetInstance()->Unregister("ITimerService");
}

/*
* 用例名称：运行Timer回调继续添加回调执行OK
* 前置条件：1、Timer线程已启动，多个Timer回调已注册
* check点：1、Timer回调次数大于1
*/
/*TEST_F(TimerServiceTest, timer_service_multi_timer_func_add_test_ok)
{   
    auto ret = ServiceFactory::GetInstance()->Register<TimerService>("ITimerService");
    std::shared_ptr<ITimerService> service = ServiceFactory::GetInstance()->GetService<ITimerService>("ITimerService");
    auto timer = service->CreateTimer();
    count = 0;
    timer->Start();
    timer->AddTimeoutExecutor([]()->bool{
        return TimerFunc();
    }, 100);
    usleep(400000);
    timer->AddTimeoutExecutor([]()->bool{
        return TimerFunc();
    }, 200);
    usleep(400000);
    timer->Stop();
    EXPECT_GE(count, 5);
    EXPECT_LE(count, 9);
    ServiceFactory::GetInstance()->Unregister("ITimerService");
}*/

/*
* 用例名称：超时回调中支持删除定时器
* 前置条件：1、Timer线程已启动，Timer回调已注册
* check点：1、是否能正常执行成功
*/
TEST_F(TimerServiceTest, timer_service_remover_timer_in_timeout_func_test_ok)
{   
    auto ret = ServiceFactory::GetInstance()->Register<TimerService>("ITimerService");
    std::shared_ptr<ITimerService> service = ServiceFactory::GetInstance()->GetService<ITimerService>("ITimerService");
    auto timer = service->CreateTimer();
    int32_t count = 0;
    timer->Start();
    int32_t id = timer->AddTimeoutExecutor([id, &count, timer]()->bool{
        timer->RemoveTimeoutExecutor(id);
        ++count;
        return true;
    }, 100);
    sleep(1);
    timer->Stop();
    EXPECT_GE(count, 1);
    ServiceFactory::GetInstance()->Unregister("ITimerService");
}

/*
* 用例名称：TimerService服务初始化
* 前置条件：1、TimerService服务已实例化
* check点：1、是否能正常执行成功
*/
TEST_F(TimerServiceTest, timer_service_remover_Initailize_test_ok)
{   
    std::shared_ptr<TimerService> service = std::make_shared<TimerService>();
    EXPECT_EQ(true, service->Initailize());
    EXPECT_EQ(true, service->Uninitailize());
    EXPECT_EQ(service->Uninitailize(), true);
}

