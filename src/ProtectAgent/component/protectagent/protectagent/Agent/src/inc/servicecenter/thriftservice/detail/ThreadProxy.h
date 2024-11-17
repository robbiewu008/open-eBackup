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
#ifndef THREAD_PROXY_H_
#define THREAD_PROXY_H_
#include <future>
#include "thrift/concurrency/Thread.h"
namespace thriftservice {
namespace detail {
class ThreadProxy : public apache::thrift::concurrency::Thread {
public:
    using Thread::Thread;
    static void threadMainProxy(std::shared_ptr<Thread> thread);
    void SetPromise(std::shared_ptr<std::promise<bool>> p);
    void Notify();

protected:
    virtual apache::thrift::concurrency::Thread::thread_funct_t getThreadFunc() const override;
    std::shared_ptr<std::promise<bool>> m_promise;
};
}  // namespace detail
}  // namespace thriftservice
#endif