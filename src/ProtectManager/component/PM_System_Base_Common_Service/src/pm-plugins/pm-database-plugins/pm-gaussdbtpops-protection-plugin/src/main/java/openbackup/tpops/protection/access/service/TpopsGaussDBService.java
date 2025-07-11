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
package openbackup.tpops.protection.access.service;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.backup.v2.PostBackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.List;
import java.util.Map;

/**
 * 与agent插件交互的接口
 *
 */
public interface TpopsGaussDBService {
    /**
     * 获取agent对象
     *
     * @param envId agent的uuid
     * @return 查询资源对象
     */
    ProtectedEnvironment getEnvironmentById(String envId);

    /**
     * 检查连通性
     *
     * @param protectedResource protectedResource
     */
    void checkConnention(ProtectedResource protectedResource);

    /**
     * 获取agent信息
     *
     * @param agentResources agentResources
     * @param protectedResource protectedResource
     * @return AppEnvResponse AppEnvResponse agent 信息
     */
    AppEnvResponse getAppEnvResponse(List<ProtectedResource> agentResources, ProtectedResource protectedResource);

    /**
     * 获取数据库已经注册的资源
     *
     * @param subType 查询资源类型，项目或者实例
     * @param filter 查询条件信息入库
     * @return 已存在的GaussDB资源信息
     */
    List<ProtectedResource> getExistingGaussDbResources(String subType, Map<String, Object> filter);

    /**
     * 修改task备份信息
     *
     * @param backupTask backupTask
     */
    void modifyBackupTaskParam(BackupTask backupTask);

    /**
     * 返回节点信息列表
     *
     * @param uuid 集群uuid
     * @return 节点信息列表
     */
    List<TaskEnvironment> supplyNodes(String uuid);

    /**
     * 更新资源的状态
     *
     * @param resourceId resourceId 资源id
     * @param status status 资源状态
     */
    void updateResourceLinkStatus(String resourceId, String status);

    /**
     * 若上一次日志备份失败, 设置下一次备份为全量备份
     *
     * @param postBackupTask 任务参数
     */
    void setNextBackupTypeWhenLogBackFail(PostBackupTask postBackupTask);
}
