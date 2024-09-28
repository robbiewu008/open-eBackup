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
package openbackup.gaussdb.protection.access.interceptor;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.backup.v2.PostBackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.gaussdb.protection.access.constant.GaussDBConstant;
import openbackup.gaussdb.protection.access.provider.GaussDBAgentProvider;
import openbackup.gaussdb.protection.access.service.GaussDBService;
import openbackup.gaussdb.protection.access.util.GaussDBClusterUtils;
import com.huawei.oceanprotect.repository.service.LocalStorageService;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;

/**
 * GaussDb环境注册提供者
 *
 */

@Component
@Slf4j
public class GaussDBBackupInterceptor extends AbstractDbBackupInterceptor {
    private final LocalStorageService localStorageService;

    private final GaussDBService gaussDbService;

    private final GaussDBAgentProvider gaussDBAgentProvider;

    /**
     * 构造器
     *
     * @param localStorageService localStorageService
     * @param gaussDbService gaussDBService
     * @param gaussDBAgentProvider gaussDBAgentProvider
     */
    public GaussDBBackupInterceptor(LocalStorageService localStorageService, GaussDBService gaussDbService,
        GaussDBAgentProvider gaussDBAgentProvider) {
        this.localStorageService = localStorageService;
        this.gaussDbService = gaussDbService;
        this.gaussDBAgentProvider = gaussDBAgentProvider;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.HCS_GAUSSDB_INSTANCE.getType().equals(object);
    }

    /**
     * 设置注册的agent信息到备份任务
     *
     * @param backupTask backupTask
     */
    @Override
    protected void supplyAgent(BackupTask backupTask) {
        // 获取环境ID
        String envId = backupTask.getProtectEnv().getUuid();
        ProtectedEnvironment protectedEnvironment = gaussDbService.getEnvironmentById(envId);
        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(protectedEnvironment)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .build();
        backupTask.setAgents(gaussDBAgentProvider.getSelectedAgents(agentSelectParam));
    }

    /**
     * PM侧校验日志备份，并将校验结果设置到任务的额外参数中
     *
     * @param backupTask 通用备份框架备份参数对象
     */
    private void checkIsLogBackup(BackupTask backupTask) {
        log.info("Check Is LogBackup, TaskType: {}", backupTask.getBackupType());
        if (GaussDBConstant.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            Map<String, String> advanceParams = backupTask.getAdvanceParams();
            advanceParams.put(GaussDBConstant.IS_CHECK_BACKUP_JOB_TYPE, "true");
        }
    }

    /**
     * 填充node信息
     *
     * @param backupTask backupTask
     */
    @Override
    protected void supplyNodes(BackupTask backupTask) {
        List<TaskEnvironment> taskEnvironments = gaussDbService.supplyNodes(backupTask.getProtectEnv().getUuid());
        backupTask.getProtectEnv().setNodes(taskEnvironments);

        // 日志备份加上检查任务类型校验
        checkIsLogBackup(backupTask);
    }

    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        GaussDBClusterUtils.addRepositoryEsnAndRole(backupTask.getRepositories().get(0),
            localStorageService.getStorageInfo().getEsn());

        // 更新环境中的信息
        gaussDbService.modifyBackupTaskParam(backupTask);
        return backupTask;
    }

    @Override
    public void finalize(PostBackupTask postBackupTask) {
        gaussDbService.setNextBackupTypeWhenLogBackFail(postBackupTask);
    }
}
