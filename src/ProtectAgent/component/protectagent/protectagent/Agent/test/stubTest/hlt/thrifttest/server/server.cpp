/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @brief  test for thrift server
 * @version 1.1.0
 * @date 2021-1-11
 * @author caomin 00511255
 */

#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/concurrency/ThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/processor/TMultiplexedProcessor.h>
#include <thrift/server/TThreadedServer.h>
#include <map>
#include <memory>
#include <iostream>
#include <assert.h>

#include <servicecenter/servicefactory/include/ServiceFactory.h>
#include <servicecenter/thriftservice/include/IThriftService.h>

#include "common/ConfigXmlParse.h"
#include "securecom/CryptAlg.h"
#include "common/Path.h"
#include "common/Log.h"
#include "openssl/ssl.h"
#include <csignal>
#include "Interface.h"

using namespace servicecenter;
using namespace thriftservice;
using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using namespace apache::thrift::server;

namespace {
    constexpr int32_t SLEEP_SEC = 60;
}
int main()
{
    std::string rootpath = "/home/code/caomin_dev/Agent";
    CPath::GetInstance().SetRootPath(rootpath);

    auto path = CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF);
    auto ret0 = CConfigXmlParser::GetInstance().Init(path);
    if (ret0 != MP_SUCCESS) {
        return false;
    }

    CLogger::GetInstance().Init("taskserver.log", CPath::GetInstance().GetLogPath());

    auto handler1 = std::shared_ptr<TaskServiceHandler>(std::make_shared<TaskServiceHandler>());
    auto processor1 = std::shared_ptr<TProcessor>(std::make_shared<TaskServiceProcessor>(handler1));
    auto handler2 = std::shared_ptr<MountServiceHandler>(std::make_shared<MountServiceHandler>());
    auto processor2 = std::shared_ptr<TProcessor>(std::make_shared<MountServiceProcessor>(handler2));

    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    auto thriftserver = thriftservice->RegisterServer("127.0.0.1", 9091);
    assert(thriftserver != nullptr);
    bool ret1 = thriftserver->RegisterProcessor("TaskService", processor1);
    assert(ret1 == true);
    ret1 = thriftserver->RegisterProcessor("MountService", processor2);
    assert(ret1 == true);

    std::cout << "Starting the server.... 127" << std::endl;
    ret1 = thriftserver->Start();
    assert(ret1 == true);

    while(1) {
        std::cout << "in sever ...."<< std::endl;
        sleep(SLEEP_SEC);
    }
    return 0;
}
