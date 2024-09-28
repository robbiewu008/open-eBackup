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
#include "ThreadProxy.h"
#include "thrift/transport/TTransportException.h"
#include "log/Log.h"

using namespace apache::thrift::concurrency;
using namespace apache::thrift::transport;

namespace {
    constexpr auto MODULE = "ThreadProxy";
}

namespace thriftservice {
namespace detail {
void ThreadProxy::threadMainProxy(std::shared_ptr<Thread> thread)
{
    try {
        Thread::threadMain(thread);
    } catch(TTransportException& ex) {
        HCP_Log(INFO, MODULE) << "thrift server main thread TTransportException: "
            <<  WIPE_SENSITIVE(ex.getType()) << ", errmsg: " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
    } catch(...) {
        HCP_Log(ERR, MODULE) << "thrift server main thread error" << HCPENDLOG;
    }
    INFOLOG("Thread Main Thread Exit!");
    if (thread->getState() != stopping && thread->getState() != stopped) {
        thread->setState(stopping);
    }
    auto ptr = std::dynamic_pointer_cast<ThreadProxy>(thread);
    if (ptr) {
        ptr->Notify();
    }
}

void ThreadProxy::SetPromise(std::shared_ptr<std::promise<bool>> p)
{
    m_promise = p;
}

Thread::thread_funct_t ThreadProxy::getThreadFunc() const
{
    return threadMainProxy;
}

void ThreadProxy::Notify()
{
    m_promise->set_value(false);
}
}
}
