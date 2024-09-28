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
#include "thriftplugin/ThriftPluginTest.h"
#include "servicefactory/include/ServiceFactory.h"
#include "servicefactory/include/IService.h"
#include "thriftservice/include/IThriftService.h"
#include "thriftservice/include/IThriftClient.h"
#include "thriftservice/include/IThriftServer.h"
#include "thriftservice/detail/ThriftClient.h"
#include "thriftservice/detail/ThriftService.h"

#include "PluginJob.h"
#include "JobService.h"
#include "ShareResource.h"
#include "PluginService.h"

using namespace thriftservice;
using namespace thriftservice::detail;


void ThriftServerStartStub()
{
    return;
}
bool ThriftClientStub()
{
    return true;
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

/*
* 用例名称：thrift服务默认注册
* 前置条件：1、通过服务中心获取thrift服务
* check点：1、检查获取服务对象不为空指针
*/
TEST_F(ThriftPluginTest, servicefactory_get_default_thrift_service_test_true)
{
    auto ret = ServiceFactory::GetInstance()->Register<ThriftService>("IThriftService");
    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    EXPECT_NE(thriftservice, nullptr);
}
