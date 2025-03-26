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
#ifndef ASYNC_LIST_JOB_H
#define ASYNC_LIST_JOB_H

#include <BasicJob.h>

using namespace AppProtect;
/* VerifyJob steps */

#ifdef WIN32
class AGENT_API AsyncListJob : public BasicJob {
#else
class AsyncListJob : public BasicJob {
#endif
public:
    explicit AsyncListJob(const ListResourceRequest& request) :
        m_request(request) {}
    virtual ~AsyncListJob() = default;

    virtual int ExecuteAsyncJob() override;

protected:
    ListResourceRequest m_request;
};
#endif
