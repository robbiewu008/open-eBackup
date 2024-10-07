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
#ifndef __UPDATE_IPROUTE_POLICY_REQUEST__
#define __UPDATE_IPROUTE_POLICY_REQUEST__

#include <string>
#include "common/model/ModelBase.h"

using VirtPlugin::ModelBase;

VIRT_PLUGIN_NAMESPACE_BEGIN

/*
 * Request body: {"task_type":"backup","destination_ip":"192.168.115.120"}
 */
struct RequestBody {
    std::string taskType;
    std::string destinationIp;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(taskType, task_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(destinationIp, destination_ip)
    END_SERIAL_MEMEBER
};

class UpdateIpRoutePolicyRequest : public ModelBase {
public:
    UpdateIpRoutePolicyRequest() = default;
    ~UpdateIpRoutePolicyRequest() = default;

    Scope GetScopeType() const override;
    ApiType GetApiType() override;
    bool BuildRequestBody(std::string &body);
    void SetTaskType(const std::string &type);
    void SetDestIp(const std::string &ip);
    std::string GetDestIp();

private:
    std::string m_taskType = "backup";
    std::string m_destIp;
};

VIRT_PLUGIN_NAMESPACE_END

#endif // __UPDATE_IPROUTE_POLICY_REQUEST__