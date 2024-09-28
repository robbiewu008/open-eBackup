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
package openbackup.data.protection.access.provider.sdk.agent;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;

import java.util.List;

/**
 * 根据资源选择selector
 *
 */
public interface AgentSelector extends DataProtectionProvider<AgentSelectParam> {
    /**
     * 执行备份、扫描任务时根据资源选择agent
     *
     * 父方法{@link DataProtectionProvider#applicable(Object)} 入参为AgentSelectParam
     *
     * @param agentSelectParam agentSelectParam
     * @return Endpoint
     */
    List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam);
}
