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
#ifndef JOB_SERVICE_FACTORY_H_
#define JOB_SERVICE_FACTORY_H_

#include <thrift/TProcessor.h>
#include "servicecenter/messageservice/include/RpcPublishObserver.h"
#include "message/curlclient/DmeRestClient.h"

namespace jobservice {
class JobServiceFactory {
public:
    static std::shared_ptr<JobServiceFactory> GetInstance();
    std::shared_ptr<apache::thrift::TProcessor> CreateProcessor();
    std::shared_ptr<messageservice::RpcPublishObserver> CreateObserver();
};
}
#endif