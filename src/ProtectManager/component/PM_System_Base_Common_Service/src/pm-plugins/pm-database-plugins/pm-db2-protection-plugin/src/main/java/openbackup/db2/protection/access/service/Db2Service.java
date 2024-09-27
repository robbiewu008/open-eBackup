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
package openbackup.db2.protection.access.service;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;

import java.util.List;

/**
 * db2服务
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-01-13
 */
public interface Db2Service {
    /**
     * 根据资源获取对应的agents
     *
     * @param resource 资源信息
     * @return List<Endpoint> agents信息
     */
    List<Endpoint> getAgentsByInstanceResource(ProtectedResource resource);

    /**
     * 根据资源获取对应的环境nodes
     *
     * @param resource 资源信息
     * @return List<TaskEnvironment> agents信息
     */
    List<TaskEnvironment> getEnvNodesByInstanceResource(ProtectedResource resource);

    /**
     * 检查是否支持恢复
     *
     * @param task 恢复任务
     */
    void checkSupportRestore(RestoreTask task);

    /**
     * 更新hadr数据库状态
     *
     * @param protectObject 保护的资源对象
     * @param resourceStatus 资源的状态
     */
    void updateHadrDatabaseStatus(TaskResource protectObject, String resourceStatus);

    /**
     * 查询数据库的数据量大小
     *
     * @param resource 保护的资源对象
     * @return 数据量大小
     */
    String queryDatabaseSize(ProtectedResource resource);
}
