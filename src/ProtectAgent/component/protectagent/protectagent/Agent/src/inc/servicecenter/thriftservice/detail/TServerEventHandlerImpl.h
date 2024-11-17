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
#ifndef TSERVER_EVENT_HANDLE_IMPL_H_
#define TSERVER_EVENT_HANDLE_IMPL_H_
#include <future>
#include "thrift/server/TServer.h"

namespace thriftservice {
namespace detail {
class TServerEventHandlerImpl : public apache::thrift::server::TServerEventHandler {
public:
    using TServerEventHandler::TServerEventHandler;
    void preServe() override;
    void SetPromise(std::shared_ptr<std::promise<bool>> p);

private:
    std::shared_ptr<std::promise<bool>> m_promise;
};
}  // namespace detail
}  // namespace thriftservice
#endif