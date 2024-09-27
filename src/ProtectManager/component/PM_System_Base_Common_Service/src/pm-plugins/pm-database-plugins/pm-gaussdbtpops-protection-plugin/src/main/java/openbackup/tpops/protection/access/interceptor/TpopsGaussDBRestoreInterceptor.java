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
package openbackup.tpops.protection.access.interceptor;

import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.BeanTools;
import openbackup.tpops.protection.access.constant.TpopsGaussDBConstant;
import openbackup.tpops.protection.access.provider.TpopsGaussDBAgentProvider;
import openbackup.tpops.protection.access.service.TpopsGaussDBService;
import openbackup.tpops.protection.access.util.TpopsGaussDBClusterUtils;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * TPOPS 恢复
 *
 * @author x30021699
 * @since 2023-05-09
 */
@Slf4j
@Component
public class TpopsGaussDBRestoreInterceptor extends AbstractDbRestoreInterceptorProvider {
    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final TpopsGaussDBService tpopsGaussDbService;

    private final CopyRestApi copyRestApi;

    private final ClusterNativeApi clusterNativeApi;

    private JobCenterRestApi jobCenterRestApi;

    private final TpopsGaussDBAgentProvider tpopsGaussDBAgentProvider;

    private EncryptorService encryptorService;

    private DeployTypeService deployTypeService;

    private final ResourceService resourceService;

    /**
     * 构造器
     *
     * @param protectedEnvironmentService protectedEnvironmentService
     * @param tpopsGaussDbService tpopsGaussDbService
     * @param copyRestApi copyRestApi
     * @param clusterNativeApi clusterNativeApi
     * @param tpopsGaussDBAgentProvider tpopsGaussDBAgentProvider
     * @param resourceService resourceService
     * @param jobCenterRestApi jobCenterRestApi
     */
    public TpopsGaussDBRestoreInterceptor(ProtectedEnvironmentService protectedEnvironmentService,
        TpopsGaussDBService tpopsGaussDbService, CopyRestApi copyRestApi, ClusterNativeApi clusterNativeApi,
        TpopsGaussDBAgentProvider tpopsGaussDBAgentProvider, ResourceService resourceService,
        JobCenterRestApi jobCenterRestApi) {
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.tpopsGaussDbService = tpopsGaussDbService;
        this.copyRestApi = copyRestApi;
        this.clusterNativeApi = clusterNativeApi;
        this.tpopsGaussDBAgentProvider = tpopsGaussDBAgentProvider;
        this.resourceService = resourceService;
        this.jobCenterRestApi = jobCenterRestApi;
    }

    @Autowired
    public void setEncryptorService(EncryptorService encryptorService) {
        this.encryptorService = encryptorService;
    }

    @Autowired
    public void setDeployTypeService(DeployTypeService deployTypeService) {
        this.deployTypeService = deployTypeService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType().equals(object);
    }

    @Override
    public RestoreTask supplyRestoreTask(RestoreTask task) {
        String isAllowRestoreFlag = resourceService.judgeResourceRestoreLevel(task.getTargetObject().getUuid());
        log.info("Tpops restore get isAllowRestoreFlag: {}, resource: {}", isAllowRestoreFlag,
            task.getTargetEnv().getUuid());
        if (StringUtils.equals(isAllowRestoreFlag, ResourceConstants.NOT_ALLOW_RESTORE)) {
            JobLogBo jobLogBo = new JobLogBo();
            jobLogBo.setJobId(task.getTaskId());
            jobLogBo.setStartTime(System.currentTimeMillis());
            jobLogBo.setLogInfo(TpopsGaussDBConstant.DATABASE_RESTORE_FAIL_NOT_ALLOW_LABEL);
            jobLogBo.setLevel(JobLogLevelEnum.ERROR.getValue());
            UpdateJobRequest request = new UpdateJobRequest();
            request.setJobLogs(Collections.singletonList(jobLogBo));
            jobCenterRestApi.updateJob(task.getTaskId(), request);
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_LINK_STATUS_ERROR,
                "TPOPS-GaussDb  " + task.getTargetObject().getUuid() + " not allow restore");
        }
        ProtectedEnvironment environment = tpopsGaussDbService.getEnvironmentById(task.getTargetEnv().getUuid());
        if (!EnvironmentLinkStatusHelper.isOnlineAdaptMultiCluster(environment)) {
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_LINK_STATUS_ERROR,
                "TPOPS-GaussDb  " + environment.getName() + " not online");
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
        advancedParams.put(TpopsGaussDBConstant.EXTEND_INFO_KEY_TARGET_LOCATION,
            task.getTargetLocation().getLocation());
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        JSONObject properties = JSONObject.fromObject(copy.getProperties());
        String backupToolTypeVaules = properties.getString(TpopsGaussDBConstant.ADVANCE_PARAMS_KEY_TOOL_TYPE);
        advancedParams.put(TpopsGaussDBConstant.ADVANCE_PARAMS_KEY_TOOL_TYPE, backupToolTypeVaules);
        advancedParams.put(TpopsGaussDBConstant.SPEED_STATISTICS, SpeedStatisticsEnum.APPLICATION.getType());
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
        task.getTargetEnv().setNodes(tpopsGaussDbService.supplyNodes(task.getTargetEnv().getUuid()));
    }

    private void supplyAgent(RestoreTask task) {
        ProtectedEnvironment environment = BeanTools.copy(task.getTargetEnv(), ProtectedEnvironment::new);

        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(environment)
            .jobType(JobTypeEnum.RESTORE.getValue())
            .build();
        List<Endpoint> endpointList = tpopsGaussDBAgentProvider.getSelectedAgents(agentSelectParam);
        log.info("GaussDb restore(requestId:{}) supply agent count:{}", task.getRequestId(), endpointList.size());
        task.setAgents(endpointList);
    }

    private void supplyTargetEnv(RestoreTask task) {
        ProtectedEnvironment environment = protectedEnvironmentService.getEnvironmentById(
            task.getTargetEnv().getUuid());
        TpopsGaussDBClusterUtils.initProtectEnvOfGaussDbUser(task.getTargetEnv(), environment.getAuth().getAuthKey());
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
    }

    private Endpoint toEndpoint(ProtectedEnvironment protectedEnvironment) {
        Endpoint endpoint = new Endpoint();
        endpoint.setId(protectedEnvironment.getUuid());
        log.debug("GaussDb cluster restore add agent(ID:{}).", protectedEnvironment.getUuid());
        endpoint.setIp(protectedEnvironment.getEndpoint());
        endpoint.setPort(protectedEnvironment.getPort());
        return endpoint;
    }

    private void supplyAuth(RestoreTask task) {
        List<StorageRepository> repositories = task.getRepositories();
        repositories.forEach(
            repository -> TpopsGaussDBClusterUtils.buildAuthentication(repository, clusterNativeApi, encryptorService,
                deployTypeService, true));
    }
}
