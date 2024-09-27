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
#ifndef __UPDATE_IPROUTE_POLICY_RESPONSE__
#define __UPDATE_IPROUTE_POLICY_RESPONSE__

#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"

VIRT_PLUGIN_NAMESPACE_BEGIN

/*
 * Response body: {"error":{"code":0}}
 */
struct ErrorInfo {
    int64_t code;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(code)
    END_SERIAL_MEMEBER
};

struct ResponseMsg {
    ErrorInfo error;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(error)
    END_SERIAL_MEMEBER
};

class UpdateIpRoutePolicyResponse : public VirtPlugin::ResponseModel {
public:
    UpdateIpRoutePolicyResponse() = default;
    ~UpdateIpRoutePolicyResponse() = default;

    int64_t GetErrorCode();
    bool Serial();

protected:
    ResponseMsg m_rspMsg;
};

VIRT_PLUGIN_NAMESPACE_END

#endif // __UPDATE_IPROUTE_POLICY_RESPONSE__