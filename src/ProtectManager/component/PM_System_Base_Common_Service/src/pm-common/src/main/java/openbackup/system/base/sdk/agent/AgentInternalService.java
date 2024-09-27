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
package openbackup.system.base.sdk.agent;

import java.util.List;

/**
 * Agent服务
 *
 * @author w00504341
 * @since 2023-07-21
 */
public interface AgentInternalService {
    /**
     * 查询节点上配了LAN-FREE的Agent
     *
     * @param esn esn
     * @return 集群节点数量
     */
    List<String> queryLanFreeAgentByClusterNode(String esn);
}
