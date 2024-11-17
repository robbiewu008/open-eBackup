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
#ifndef APPPLUGINS_VIRTUALIZATION_NUTANIXRETRYCODESTRUCT_H
#define APPPLUGINS_VIRTUALIZATION_NUTANIXRETRYCODESTRUCT_H

#include "log/Log.h"

namespace NutanixPlugin {
class NutanixRetryCodeStruct {
public:
    NutanixRetryCodeStruct()
    {
        std::string retryCode = Module::ConfigReader::getString("NutanixConfig", "RetryCode");
        if (!Module::JsonHelper::JsonStringToStruct(retryCode, *this)) {
            ERRLOG("Convert %s failed.", WIPE_SENSITIVE(retryCode).c_str());
        }
    };

    bool IsHttpStatusRetryCode(uint32_t httpStatusCode)
    {
        if (httpRetryCode.empty()) {
            ERRLOG("httpRetryCode is empty.");
            return false;
        }
        if (std::find(httpRetryCode.begin(), httpRetryCode.end(), httpStatusCode) != httpRetryCode.end()) {
            WARNLOG("find the http status retry code %d", httpStatusCode);
            return true;
        }
        return false;
    }

    bool IsCurlRetryCode(uint32_t curlCode)
    {
        if (curlRetryCode.empty()) {
            ERRLOG("curlRetryCode is empty.");
            return false;
        }
        if (std::find(curlRetryCode.begin(), curlRetryCode.end(), curlCode) != curlRetryCode.end()) {
            WARNLOG("find the curl retry code %d", curlCode);
            return true;
        }
        return false;
    }

    ~NutanixRetryCodeStruct() = default;

    std::vector<uint32_t> httpRetryCode;
    std::vector<int32_t> curlRetryCode;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(httpRetryCode)
    SERIAL_MEMEBER(curlRetryCode)
    END_SERIAL_MEMEBER
};
}
#endif