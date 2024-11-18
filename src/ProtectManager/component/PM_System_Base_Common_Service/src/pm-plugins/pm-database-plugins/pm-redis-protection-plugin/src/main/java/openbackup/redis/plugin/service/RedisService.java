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
package openbackup.redis.plugin.service;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

/**
 * Redis Service
 *
 */
public interface RedisService {
    /**
     * 在创建或者更新集群之前做一些校验工作。
     *
     * @param protectedEnvironment 集群
     */
    void preCheck(ProtectedEnvironment protectedEnvironment);

    /**
     * 选择一个在线的agent
     *
     * @param child child
     * @return agent
     */
    Endpoint selectAgent(ProtectedResource child);

    /**
     * 校验集群节点是否已经添加过。
     *
     * @param protectedResource 集群节点
     */
    void checkNodeExists(ProtectedResource protectedResource);
}