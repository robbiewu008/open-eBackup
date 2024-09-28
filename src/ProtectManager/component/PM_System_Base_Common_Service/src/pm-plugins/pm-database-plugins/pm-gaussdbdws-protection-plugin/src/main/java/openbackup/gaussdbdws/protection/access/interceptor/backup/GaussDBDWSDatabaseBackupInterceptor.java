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
package openbackup.gaussdbdws.protection.access.interceptor.backup;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.gaussdbdws.protection.access.provider.GaussDBDWSAgentProvider;
import openbackup.gaussdbdws.protection.access.service.GaussDBBaseService;
import openbackup.gaussdbdws.protection.access.util.DwsBuildRepositoryUtil;
import com.huawei.oceanprotect.repository.service.LocalStorageService;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * GaussDbDws 集群备份任务下发Provider
 *
 */
@Slf4j
@Component
public class GaussDBDWSDatabaseBackupInterceptor extends AbstractDbBackupInterceptor {
    private final LocalStorageService localStorageService;

    private final GaussDBBaseService gaussDBBaseService;

    private final GaussDBDWSAgentProvider agentProvider;

    private final ClusterBasicService clusterBasicService;

    private final DeployTypeService deployTypeService;

    /**
     * 构造器
     *
     * @param localStorageService 本地存储 service
     * @param agentProvider agentProvider
     * @param gaussDBBaseService DWS 应用基本的Service
     * @param clusterBasicService clusterBasicService
     * @param deployTypeService deployTypeService
     */
    public GaussDBDWSDatabaseBackupInterceptor(LocalStorageService localStorageService,
        GaussDBDWSAgentProvider agentProvider, GaussDBBaseService gaussDBBaseService,
        ClusterBasicService clusterBasicService, DeployTypeService deployTypeService) {
        this.localStorageService = localStorageService;
        this.agentProvider = agentProvider;
        this.gaussDBBaseService = gaussDBBaseService;
        this.clusterBasicService = clusterBasicService;
        this.deployTypeService = deployTypeService;
    }

    /**
     * 填充agent信息
     *
     * @param backupTask backupTask
     */
    @Override
    protected void supplyAgent(BackupTask backupTask) {
        AgentSelectParam agentSelectParam = super.buildAgentSelectParam(backupTask);
        backupTask.setAgents(agentProvider.getSelectedAgents(agentSelectParam));
        gaussDBBaseService.addSupplyAgent(backupTask.getAgents(), backupTask.getProtectEnv().getRootUuid(),
            "DWS-clusterPlugin");
    }

    /**
     * 数据库各自应用信息
     *
     * @param backupTask backupTask
     * @return BackupTask
     */
    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        if (!deployTypeService.isE1000()) {
            DwsBuildRepositoryUtil.addRepositoryEsnAndRole(backupTask.getRepositories().get(0),
                clusterBasicService.getCurrentClusterEsn());
        }
        gaussDBBaseService.modifyBackupTaskParam(backupTask);
        gaussDBBaseService.modifyAdvanceParams(backupTask.getAdvanceParams(), backupTask.getProtectObject().getUuid());
        return backupTask;
    }

    /**
     * 填充node信息
     *
     * @param backupTask backupTask
     */
    @Override
    protected void supplyNodes(BackupTask backupTask) {
        List<TaskEnvironment> taskEnvironments = gaussDBBaseService.supplyNodes(
            backupTask.getProtectObject().getRootUuid());
        backupTask.getProtectEnv().setNodes(taskEnvironments);
    }

    /**
     * 配置对应applicable集对象
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.GAUSSDB_DWS_DATABASE.getType().equals(object);
    }
}
