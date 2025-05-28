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
package openbackup.oceanprotect.k8s.protection.access.provider;

import openbackup.oceanprotect.k8s.protection.access.constant.K8sConstant;
import openbackup.oceanprotect.k8s.protection.access.service.K8sCommonService;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.backup.v2.PostBackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * k8s备份provider
 *
 */
@Component
public class K8sBackupProvider implements BackupInterceptorProvider {
    private final K8sCommonService commonService;
    private final ProtectedEnvironmentService environmentService;

    /**
     * 构造函数
     *
     * @param commonService K8sCommonService
     * @param environmentService ProtectedEnvironmentService
     */
    public K8sBackupProvider(K8sCommonService commonService, ProtectedEnvironmentService environmentService) {
        this.commonService = commonService;
        this.environmentService = environmentService;
    }

    @Override
    public BackupTask initialize(BackupTask backupTask) {
        ProtectedEnvironment environment = environmentService
                .getEnvironmentById(backupTask.getProtectObject().getRootUuid());
        commonService.addIpRule(environment);
        checkConnectivity(backupTask);
        handleRepositories(backupTask);
        setDeployType(backupTask);
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);
        // 拼接包含标签和排除标签
        setLabels(backupTask);
        if (ResourceSubTypeEnum.KUBERNETES_NAMESPACE_COMMON.equalsSubType(backupTask.getProtectObject().getSubType())) {
            backupTask.getProtectObject().setParentName(backupTask.getProtectObject().getName());
        }
        commonService.fillVpcInfo(backupTask.getAdvanceParams(), backupTask.getProtectObject().getUuid());
        commonService.fillBackUpAgentConnectedIps(backupTask.getAgents());
        return backupTask;
    }

    @Override
    public void finalize(PostBackupTask postBackupTask) {
        ProtectedEnvironment environment =
            environmentService.getEnvironmentById(postBackupTask.getProtectedObject().getEnvUuid());
        commonService.deleteIpRule(environment);
    }

    private void checkConnectivity(BackupTask backupTask) {
        String uuid = backupTask.getProtectObject().getRootUuid();
        commonService.checkConnectivity(environmentService.getEnvironmentById(uuid));
    }

    private void setLabels(BackupTask backupTask) {
        Map<String, String> extendInfo = backupTask.getProtectObject().getExtendInfo();
        String excludeLabels = extendInfo.get("excludeLabels");
        String labels = extendInfo.get("labels");
        List<String> labelList = new ArrayList<>();
        if (!VerifyUtil.isEmpty(labels)) {
            labelList.add(labels);
        }
        if (!VerifyUtil.isEmpty(excludeLabels)) {
            labelList.add(excludeLabels);
        }
        labels = StringUtils.join(labelList, ',');
        extendInfo.put("labels", labels);
    }

    private void setDeployType(BackupTask backupTask) {
        // 部署类型
        Map<String, String> extendInfo = backupTask.getProtectEnv().getExtendInfo();
        extendInfo.put(K8sConstant.DEPLOY_TYPE, K8sConstant.DISTRIBUTED);
    }

    private Endpoint getEndpoint(ProtectedEnvironment env) {
        Endpoint endpoint = new Endpoint();
        endpoint.setId(env.getUuid());
        endpoint.setIp(env.getEndpoint());
        endpoint.setPort(env.getPort());
        return endpoint;
    }

    private void handleRepositories(BackupTask task) {
        List<StorageRepository> repositories = task.getRepositories();
        // 添加存储仓库类型：2-CACHE_REPOSITORY
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        task.addRepository(cacheRepository);
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.KUBERNETES_NAMESPACE_COMMON.equalsSubType(object)
                || ResourceSubTypeEnum.KUBERNETES_DATASET_COMMON.equalsSubType(object);
    }
}
