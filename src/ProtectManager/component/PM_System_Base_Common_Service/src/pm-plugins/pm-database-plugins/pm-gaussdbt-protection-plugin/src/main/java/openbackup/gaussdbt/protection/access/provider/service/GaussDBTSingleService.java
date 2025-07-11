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
package openbackup.gaussdbt.protection.access.provider.service;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;

import java.util.List;

/**
 * GaussDBT单机服务
 *
 */
public interface GaussDBTSingleService {
    /**
     * 检查单机数据库资源是否已经被注册
     *
     * @param environment 单机资源信息
     */
    void checkSingleIsRegistered(ProtectedEnvironment environment);

    /**
     * 检查单机资源的用户名是否被修改
     *
     * @param environment 单机资源信息
     */
    void checkSingleInstallUserIsChanged(ProtectedEnvironment environment);

    /**
     * 检查单机资源连通性
     *
     * @param environment 单机资源信息
     * @return 返回检查结果
     */
    String checkConnection(ProtectedEnvironment environment);

    /**
     * 填充单机资源的属性值
     *
     * @param environment 单机资源信息
     * @param checkResult agent返回的结果
     */
    void fillGaussDBTSingleProperties(ProtectedEnvironment environment, String checkResult);

    /**
     * 检查更新GaussDBT单机版环境的状态
     *
     * @param environment 单机资源信息
     */
    void checkGaussDTSingleStatus(ProtectedEnvironment environment);

    /**
     * 根据id查询资源
     *
     * @param resourceId 单机资源信息
     * @return 返回资源
     */
    ProtectedResource getResourceById(String resourceId);

    /**
     * 根据资源提取nodes
     *
     * @param resource 资源信息
     * @return 返回nodes
     */
    List<TaskEnvironment> getEnvNodes(ProtectedResource resource);

    /**
     * 根据资源提取agents
     *
     * @param resource 资源信息
     * @return 返回agents
     */
    List<Endpoint> getAgents(ProtectedResource resource);

    /**
     * 检查是否支持恢复
     *
     * @param task 恢复任务
     */
    void checkSupportRestore(RestoreTask task);
}
