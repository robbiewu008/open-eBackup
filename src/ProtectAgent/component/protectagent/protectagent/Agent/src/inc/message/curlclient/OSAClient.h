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
#ifndef __OSA_CLIENT_H__
#define __OSA_CLIENT_H__

#ifdef LINUX
#include <string>
#include "common/Types.h"
#include "message/curlclient/CurlHttpClient.h"

/*
 * Request body: {"task_type":"backup","destination_ip":"192.168.115.120"}
 */
struct IpPolicyParams {
    std::string oper = "add"; // "add"/"delete"
    std::string taskType = "backup";
    std::string destinationIp;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(taskType, task_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(destinationIp, destination_ip)
    END_SERIAL_MEMEBER
};

/*
 * Response body: {"error":{"code":0}}
 */
class OSAClient {
public:
    explicit OSAClient() {}
    explicit OSAClient(const std::string &osaIp) : m_osaIp(osaIp) {}
    virtual ~OSAClient() {}
    virtual int32_t ModifyIpPolicy(const IpPolicyParams &reqParams);

private:
    bool CheckIpPolicyParams(const IpPolicyParams &reqParams);
    template<typename T> bool BuildRequestBody(const T &reqParams, std::string &body);

private:
    std::string m_osaIp = "protectengine.dpa.svc.cluster.local";
};
#endif

#endif // __OSA_CLIENT_H__