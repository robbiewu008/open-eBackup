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
#include "log/Log.h"
#include "AsyncListJob.h"
#include <OpenLibMgr.h>
#include <ClientInvoke.h>

namespace {
    constexpr auto MODULE = "AsyncListJob";
    constexpr int INVOKE_PLUGIN_SUCCESS = 0;
    constexpr int INVOKE_PLUGIN_FAILED = -1;
}
using namespace AppProtect;
using ListApplicationResourceV2Fun = void(ResourceResultByPage& page, const ListResourceRequest& request);

int AsyncListJob::ExecuteAsyncJob()
{
    ResourceResultByPage page;
    int code = -1;
    try {
        auto fun = OpenLibMgr::GetInstance().GetObj<ListApplicationResourceV2Fun>("ListApplicationResourceV2");
        if (fun == nullptr) {
            HCP_Log(ERR, MODULE) << "Get AsyncListApplicationResourceV2 function failed" << HCPENDLOG;
            return Module::FAILED;
        }
        fun(page, m_request);
        code = INVOKE_PLUGIN_SUCCESS;
        HCP_Log(DEBUG, MODULE) << "Get AsyncListApplicationResourceV2 function ReportAsyncJobDetails" << HCPENDLOG;
    } catch (AppProtectPluginException &e) {
    HCP_Log(ERR, MODULE) << "PluginException exception. Code: "<< e.code << " Message: " <<
        e.message.c_str() << HCPENDLOG;
        code = e.code;
    } catch (const std::exception& ex) {
        HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        code = INVOKE_PLUGIN_FAILED;
    } catch (...) {
        HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        code = INVOKE_PLUGIN_FAILED;
    }
    SetJobToFinish();
    ActionResult returnValue;
    JobService::ReportAsyncJobDetails(returnValue, m_jobId, code, page);
    if (returnValue.code != 0) {
        HCP_Log(ERR, MODULE) << "ReportAsyncJobDetails function failed, code:" << returnValue.code <<
            "error: " << returnValue.bodyErr << "message: "  << returnValue.message.c_str() << HCPENDLOG;
    } else {
        HCP_Log(INFO, MODULE) << "ReportAsyncJobDetails function success, code:" << returnValue.code <<
            "error: " << returnValue.bodyErr << "message: "  << returnValue.message.c_str() << HCPENDLOG;
    }
    return Module::SUCCESS;
}
