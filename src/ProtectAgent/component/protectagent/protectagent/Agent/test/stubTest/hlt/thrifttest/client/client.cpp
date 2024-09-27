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
#include <iostream>
#include <string>
#include <set>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/protocol/TMultiplexedProtocol.h>
#include <chrono>
#include <thread>
#include "TaskService.h"
#include "MountService.h"

#include <servicecenter/servicefactory/include/ServiceFactory.h>
#include <servicecenter/thriftservice/include/IThriftService.h>
#include "common/ConfigXmlParse.h"
#include "securecom/CryptAlg.h"
#include "common/Path.h"
#include "common/Log.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace thrifttest;
using namespace thriftservice;
using namespace servicecenter;

namespace {
    constexpr int32_t SLEEP_MILL_SEC = 50;
}

int main()
{
    std::string rootpath = "/home/code/caomin_dev/Agent";
    CPath::GetInstance().SetRootPath(rootpath);

    auto path = CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF);
    auto ret = CConfigXmlParser::GetInstance().Init(path);
    if (ret != MP_SUCCESS) {
        return false;
    }

    CLogger::GetInstance().Init("taskclientserver.log", CPath::GetInstance().GetLogPath());

    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    thriftservice::ClientSocketOpt opt = { "localhost", 9091 };
    auto thriftclient = thriftservice->RegisterClient(opt);
    while(1) {
        try {
            auto ret = thriftclient->Start();
            assert(ret == true);

            auto taskclient = thriftclient->GetConcurrentClientIf<TaskServiceConcurrentClient>("TaskService");
            assert(taskclient != nullptr);

            auto mountclient = thriftclient->GetConcurrentClientIf<MountServiceConcurrentClient>("MountService");

            assert(mountclient != nullptr);

            std::thread th1([&] {
                while(1) {
                    Task task;
                    taskclient->GetTask(task, 1);
                    std::cout << task.info << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MILL_SEC));
                }
            });

            std::thread th2([&] {
                while(1) {
                    Config config;
                    config.port = 0;
                    config.host = "127.0.0.1";
                    auto res = mountclient->Mount(config, "/root");
                    std::cout <<"first mount root res = "<< res << std::endl;
                    res = mountclient->Mount(config, "/root");
                    std::cout <<"second mount root res = "<< res << std::endl;
                    res = mountclient->Unmount("/root");
                    std::cout <<"first unmount root res = "<< res << std::endl;
                    res = mountclient->Unmount("/root");
                    std::cout <<"second unmount root res = "<< res << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MILL_SEC));
                    auto now = std::chrono::system_clock::now();
                    auto now_c = std::chrono::system_clock::to_time_t(now);
                    std::cout << std::put_time(std::localtime(&now_c), "%c") << '\n';
                }
            });

            th1.join();
            th2.join();

            thriftclient->Stop();
        } catch (TException& tx) {
            std::cout << "ERROR: " << tx.what() << std::endl;
        }
    }
}
