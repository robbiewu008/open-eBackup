/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.gaussdbdws.protection.access.interceptor.backup;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
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
 * @author swx1010572
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-23
 */
@Slf4j
@Component
public class GaussDBDWSTableBackupInterceptor extends AbstractDbBackupInterceptor {
    private final LocalStorageService localStorageService;

    private final GaussDBBaseService gaussDBBaseService;

    private final GaussDBDWSAgentProvider agentProvider;

    private final ClusterBasicService clusterBasicService;

    private final DeployTypeService deployTypeService;

    /**
     * 构造器
     *
     * @param localStorageService 本地存储 service
     * @param gaussDBBaseService DWS 应用基本的Service
     * @param agentProvider agentProvider
     * @param clusterBasicService clusterBasicService
     * @param deployTypeService deployTypeService
     */
    public GaussDBDWSTableBackupInterceptor(LocalStorageService localStorageService,
        GaussDBBaseService gaussDBBaseService, GaussDBDWSAgentProvider agentProvider,
        ClusterBasicService clusterBasicService, DeployTypeService deployTypeService) {
        this.localStorageService = localStorageService;
        this.agentProvider = agentProvider;
        this.gaussDBBaseService = gaussDBBaseService;
        this.clusterBasicService = clusterBasicService;
        this.deployTypeService = deployTypeService;
    }

    /**
     * 填充node信息
     *
     * @param backupTask backupTask
     */
    @Override
    protected void supplyNodes(BackupTask backupTask) {
        gaussDBBaseService.checkLinkStatus(backupTask.getProtectEnv().getUuid());
        List<TaskEnvironment> taskEnvironments = gaussDBBaseService.supplyNodes(
            backupTask.getProtectObject().getRootUuid());
        backupTask.getProtectEnv().setNodes(taskEnvironments);
    }

    /**
     * 填充agent信息
     *
     * @param backupTask backupTask
     */
    @Override
    protected void supplyAgent(BackupTask backupTask) {
        backupTask.setAgents(agentProvider.getSelectedAgents(super.buildAgentSelectParam(backupTask)));
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
     * 配置对应applicable集对象
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType().equals(object);
    }
}
