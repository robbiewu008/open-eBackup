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

import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.gaussdb.protection.access.constant.GaussDBConstant;
import openbackup.gaussdb.protection.access.provider.GaussDBAgentProvider;
import openbackup.gaussdb.protection.access.service.GaussDBService;
import openbackup.gaussdb.protection.access.util.GaussDBClusterUtils;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * HCS-GaussDb 恢复任务基础数据Provider
 *
 */
@Slf4j
@Component
public class GaussDBRestoreInterceptor extends AbstractDbRestoreInterceptorProvider {
    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final GaussDBService gaussDbService;

    private final CopyRestApi copyRestApi;

    private final ClusterNativeApi clusterNativeApi;

    private final GaussDBAgentProvider gaussDBAgentProvider;

    private EncryptorService encryptorService;

    /**
     * 构造器
     *
     * @param protectedEnvironmentService protectedEnvironmentService
     * @param gaussDbService gaussDbService
     * @param copyRestApi copyRestApi
     * @param clusterNativeApi clusterNativeApi
     * @param gaussDBAgentProvider gaussDBAgentProvider
     */
    public GaussDBRestoreInterceptor(ProtectedEnvironmentService protectedEnvironmentService,
        GaussDBService gaussDbService, CopyRestApi copyRestApi, ClusterNativeApi clusterNativeApi,
        GaussDBAgentProvider gaussDBAgentProvider) {
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.gaussDbService = gaussDbService;
        this.copyRestApi = copyRestApi;
        this.clusterNativeApi = clusterNativeApi;
        this.gaussDBAgentProvider = gaussDBAgentProvider;
    }

    @Autowired
    public void setEncryptorService(EncryptorService encryptorService) {
        this.encryptorService = encryptorService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.HCS_GAUSSDB_INSTANCE.getType().equals(object);
    }

    @Override
    public RestoreTask supplyRestoreTask(RestoreTask task) {
        ProtectedEnvironment environment = gaussDbService.getEnvironmentById(task.getTargetEnv().getUuid());
        if (!EnvironmentLinkStatusHelper.isOnlineAdaptMultiCluster(environment)) {
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_LINK_STATUS_ERROR,
                "HCS-GaussDb  " + environment.getName() + " not online");
        }
        supplyAgent(task);
        supplyNodes(task);
        supplyTargetEnv(task);
        supplyRestoreMode(task);
        supplyAdvancedParams(task);
        supplyAuth(task);
        return task;
    }

    private void supplyAdvancedParams(RestoreTask task) {
        Map<String, String> advancedParams = Optional.ofNullable(task.getAdvanceParams()).orElse(new HashMap<>());
        advancedParams.put(GaussDBConstant.EXTEND_INFO_KEY_TARGET_LOCATION, task.getTargetLocation().getLocation());
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        JSONObject properties = JSONObject.fromObject(copy.getProperties());
        String backupToolTypeVaules = properties.getString(GaussDBConstant.ADVANCE_PARAMS_KEY_TOOL_TYPE);
        advancedParams.put(GaussDBConstant.ADVANCE_PARAMS_KEY_TOOL_TYPE, backupToolTypeVaules);
        advancedParams.put(GaussDBConstant.SPEED_STATISTICS, SpeedStatisticsEnum.APPLICATION.getType());
        task.setAdvanceParams(advancedParams);
    }

    /**
     * 设置恢复模式
     *
     * @param task 恢复任务
     */
    public void supplyRestoreMode(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        String generatedBy = copy.getGeneratedBy();
        if (CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value().equals(generatedBy)) {
            task.setRestoreMode(RestoreModeEnum.REMOTE_RESTORE.getMode());
        } else if (CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value().equals(generatedBy)) {
            task.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
        } else {
            task.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }
        log.info("build GaussDb copy restore mode. copy id: {}, mode: {}", task.getCopyId(), task.getRestoreMode());
    }

    private void supplyNodes(RestoreTask task) {
        task.getTargetEnv().setNodes(gaussDbService.supplyNodes(task.getTargetEnv().getUuid()));
    }

    private void supplyAgent(RestoreTask task) {
        ProtectedEnvironment environment = BeanTools.copy(task.getTargetEnv(), ProtectedEnvironment::new);

        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(environment)
            .jobType(JobTypeEnum.RESTORE.getValue())
            .build();
        List<Endpoint> endpointList = gaussDBAgentProvider.getSelectedAgents(agentSelectParam);
        log.info("GaussDb restore(requestId:{}) supply agent count:{}", task.getRequestId(), endpointList.size());
        task.setAgents(gaussDBAgentProvider.getSelectedAgents(agentSelectParam));
    }

    private void supplyTargetEnv(RestoreTask task) {
        ProtectedEnvironment environment = protectedEnvironmentService.getEnvironmentById(
            task.getTargetEnv().getUuid());
        GaussDBClusterUtils.initProtectEnvOfGaussDbUser(task.getTargetEnv(), environment.getAuth().getAuthKey());
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
    }

    private void supplyAuth(RestoreTask task) {
        List<StorageRepository> repositories = task.getRepositories();
        repositories.forEach(
            repository -> GaussDBClusterUtils.buildAuthentication(repository, clusterNativeApi, encryptorService,
                true));
    }
}
