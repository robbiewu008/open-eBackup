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
package openbackup.access.framework.resource.service;

import openbackup.access.framework.resource.dto.DeliverTaskReq;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;

import java.util.List;

/**
 * 对接agent的业务service
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-27
 */
public interface AgentBusinessService {
    /**
     * 传递任务状态
     *
     * @param deliverTaskReq 传递任务状态请求体
     */
    void deliverTaskStatus(DeliverTaskReq deliverTaskReq);

    /**
     * 查询内置agent
     *
     * @return internal agent list
     */
    List<Endpoint> queryInternalAgents();

    /**
     * 查询内置agent
     *
     * @return internal agent env list
     */
    List<ProtectedEnvironment> queryInternalAgentEnv();
}
