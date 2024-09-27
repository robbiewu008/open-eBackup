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
#include <servicefactory/include/ServiceFactory.h>
#include <servicefactory/detail/ServiceCenter.h>
#include <messageservice/MessageServiceTest.h>
#include <messageservice/include/IObserver.h>
#include <messageservice/include/IEvent.h>
#include <messageservice/include/IMessageService.h>
#include <messageservice/detail/MessageService.h>

using namespace servicecenter;
using namespace servicecenter::detail;
using namespace messageservice;
using namespace messageservice::detail;
class EventTest : public IEvent {
public:
    void SetInt(int32_t value) {
        m_value = value;
    }

    int32_t GetInt() {
        return m_value;
    }
private:
    int32_t m_value {0};
};

class ObserverTest : public IObserver {
public:
    virtual void Update(std::shared_ptr<IEvent> event) {
        std::shared_ptr<EventTest> eventtest = std::dynamic_pointer_cast<EventTest>(event);
        if (eventtest != nullptr) {
            m_value = eventtest->GetInt();
        }
    }

    int32_t GetInt() {
        return m_value;
    }
private:
    int32_t m_value {0};
};


/*
* 用例名称：Event初始化为NULL类型
* 前置条件：1、构造Event对象，2、未设置Event对象类型
* check点：1、检查Event的类型为NULL_TYPE
*/
TEST_F(MessageServiceTest, event_init_get_event_test_eq_null_type)
{
    IEvent event;
    EXPECT_EQ(event.GetEvent(), EVENT_TYPE::NULL_TYPE);
}

/*
* 用例名称：Event设置类型后获取类型
* 前置条件：1、构造Event对象，2、设置Event对象类型
* check点：1、检查Event的类型为设置的类型
*/
TEST_F(MessageServiceTest, event_get_event_test_after_set_event)
{
    IEvent event;
    event.SetEvent(EVENT_TYPE::RPC_PUBLISH_TYPE);
    EXPECT_EQ(event.GetEvent(), EVENT_TYPE::RPC_PUBLISH_TYPE);
}

/*
* 用例名称：MessageService注册到服务中心获取message服务
* 前置条件：1、注册message服务到服务中心 2、获取message服务
* check点：1、检查message服务对象不为空
*/
TEST_F(MessageServiceTest, get_message_service_test_eq_not_null)
{
    bool res = ServiceFactory::GetInstance()->Register<MessageService>("IMessageService");
    EXPECT_EQ(res, true);
    std::shared_ptr<IMessageService> messageService = ServiceFactory::GetInstance()->GetService<IMessageService>("IMessageService");
    EXPECT_NE(messageService, nullptr);
}

/*
* 用例名称：MessageService获取Subject对象不为空
* 前置条件：1、获取message服务 2、通过message服务获取Subject对象
* check点：1、检查Subject对象不为空
*/
TEST_F(MessageServiceTest, get_subject_test_eq_not_null)
{
    std::shared_ptr<IMessageService> messageService = ServiceFactory::GetInstance()->GetService<IMessageService>("IMessageService");
    std::shared_ptr<ISubject> subject = messageService->GetSubject();
    EXPECT_NE(subject, nullptr);
}

/*
* 用例名称：Subject对象注册Observer对象
* 前置条件：1、获取message服务 2、通过message服务获取Subject对象 3、通过Subject对象注册Observer对象
* check点：1、检查Observer对象不为空
*/
TEST_F(MessageServiceTest, register_observer_return_should_be_true)
{
    std::shared_ptr<IMessageService> messageService = ServiceFactory::GetInstance()->GetService<IMessageService>("IMessageService");
    std::shared_ptr<ISubject> subject = messageService->GetSubject();
    std::shared_ptr<ObserverTest> test = std::make_shared<ObserverTest>();
    bool ret = subject->Register(EVENT_TYPE::RPC_PUBLISH_TYPE, test);
    EXPECT_EQ(ret, true);
}

/*
* 用例名称：Subject对象发布消息
* 前置条件：1、获取message服务 2、通过message服务获取Subject对象 3、通过Subject对象注册Observer对象 4、通过Subject对象发布消息
* check点：1、检查消息内容是否为发布前的状态
*/
TEST_F(MessageServiceTest, observer_get_value_when_notify_event_should_be_one)
{
    std::shared_ptr<IMessageService> messageService = ServiceFactory::GetInstance()->GetService<IMessageService>("IMessageService");
    std::shared_ptr<ISubject> subject = messageService->GetSubject();
    std::shared_ptr<ObserverTest> observertest = std::make_shared<ObserverTest>();
    bool ret = subject->Register(EVENT_TYPE::RPC_PUBLISH_TYPE, observertest);
    std::shared_ptr<EventTest> eventtest = std::make_shared<EventTest>();
    eventtest->SetInt(1);
    subject->Notify(EVENT_TYPE::RPC_PUBLISH_TYPE, eventtest);
    EXPECT_EQ(observertest->GetInt(), 1);
}

/*
* 用例名称：Subject对象注销Observer对象
* 前置条件：1、获取message服务 2、通过message服务获取Subject对象 3、通过Subject对象注册Observer对象
* check点：1、检查注销Observer对象返回值为true
*/
TEST_F(MessageServiceTest, unregister_observer_return_should_be_true)
{
    std::shared_ptr<IMessageService> messageService = ServiceFactory::GetInstance()->GetService<IMessageService>("IMessageService");
    std::shared_ptr<ISubject> subject = messageService->GetSubject();
    std::shared_ptr<ObserverTest> test = std::make_shared<ObserverTest>();
    bool ret = subject->Register(EVENT_TYPE::RPC_PUBLISH_TYPE, test);
    ret = subject->Unregister(EVENT_TYPE::RPC_PUBLISH_TYPE, test);
    EXPECT_EQ(ret, true);
}

/*
* 用例名称：Subject对象注销非已注册的Observer对象
* 前置条件：1、获取message服务 2、通过message服务获取Subject对象
* check点：1、检查注销Observer对象返回值为true
*/
TEST_F(MessageServiceTest, unregister_no_register_observer_return_should_be_true)
{
    std::shared_ptr<IMessageService> messageService = ServiceFactory::GetInstance()->GetService<IMessageService>("IMessageService");
    std::shared_ptr<ISubject> subject = messageService->GetSubject();
    std::shared_ptr<ObserverTest> test = std::make_shared<ObserverTest>();
    auto ret = subject->Unregister(EVENT_TYPE::RPC_PUBLISH_TYPE, test);
    EXPECT_EQ(ret, true);
}

