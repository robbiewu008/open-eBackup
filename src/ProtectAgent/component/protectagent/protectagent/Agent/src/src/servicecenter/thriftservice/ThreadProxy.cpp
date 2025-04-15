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
#include "thrift/transport/TTransportException.h"
#include "common/Log.h"
#include "servicecenter/thriftservice/detail/ThreadProxy.h"

using namespace apache::thrift::concurrency;
using namespace apache::thrift::transport;
namespace thriftservice {
namespace detail {
void ThreadProxy::threadMainProxy(std::shared_ptr<Thread> thread)
{
    try {
        Thread::threadMain(thread);
    } catch(TTransportException ex) {
        ERRLOG("thrift server main thread TTransportException:%d, errmsg:%s", ex.getType(), ex.what());
    } catch(...) {
        ERRLOG("thrift server main thread error");
    }
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