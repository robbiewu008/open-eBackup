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
package openbackup.gaussdbdws.protection.access.interceptor.copy;

import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.service.GaussDBBaseService;
import openbackup.gaussdbdws.protection.access.util.DwsBuildRepositoryUtil;
import openbackup.gaussdbdws.protection.access.util.DwsTaskEnvironmentUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.ClustersInfoVo;
import openbackup.system.base.sdk.cluster.model.TargetClusterRequestParm;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.OpServiceUtil;

import org.apache.commons.collections.CollectionUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * GaussDBDWS副本删除Provider
 *
 */
@Slf4j
@Component
public class GaussDBDWSCopyDeleteInterceptor extends AbstractDbCopyDeleteInterceptor {
    private final ClusterNativeApi clusterNativeApi;

    private final EncryptorService encryptorService;

    private final ProviderManager providerManager;

    private ProtectedResourceChecker protectedResourceChecker;

    private GaussDBBaseService gaussDBBaseService;

    private DeployTypeService deployTypeService;

    /**
     * Constructor
     *
     * @param copyRestApi copyRestApi
     * @param clusterNativeApi clusterNativeApi
     * @param encryptorService encryptorService
     * @param providerManager providerManager
     * @param resourceService resourceService
     */
    public GaussDBDWSCopyDeleteInterceptor(CopyRestApi copyRestApi, ClusterNativeApi clusterNativeApi,
        EncryptorService encryptorService, ProviderManager providerManager, ResourceService resourceService) {
        super(copyRestApi, resourceService);
        this.clusterNativeApi = clusterNativeApi;
        this.encryptorService = encryptorService;
        this.providerManager = providerManager;
    }

    @Autowired
    @Qualifier("unifiedResourceConnectionChecker")
    public void setProtectedResourceChecker(ProtectedResourceChecker protectedResourceChecker) {
        this.protectedResourceChecker = protectedResourceChecker;
    }

    @Autowired
    public void setGaussDBBaseService(GaussDBBaseService gaussDBBaseService) {
        this.gaussDBBaseService = gaussDBBaseService;
    }

    @Autowired
    public void setDeployTypeService(DeployTypeService deployTypeService) {
        this.deployTypeService = deployTypeService;
    }

    @Override
    public boolean applicable(String subType) {
        return Arrays.asList(ResourceSubTypeEnum.GAUSSDB_DWS.getType(),
            ResourceSubTypeEnum.GAUSSDB_DWS_DATABASE.getType(), ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType(),
            ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType()).contains(subType);
    }

    /**
     * 副本级联删除
     *
     * @param copyId 副本ID
     * @return 需要删除的副本uuid集合
     */
    @Override
    public List<String> getAssociatedCopy(String copyId) {
        return super.getAssociatedCopy(copyId);
    }

    @Override
    protected List<String> getCopiesCopyTypeIsDifferenceIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        // 增量副本（返回增量副本到下个全量副本之间的增量副本）
        List<Copy> differenceCopies = CopyUtil.getCopiesByCopyType(copies, BackupTypeConstants.DIFFERENCE_INCREMENT);
        return CopyUtil.getCopyUuidsBetweenTwoCopy(differenceCopies, thisCopy, nextFullCopy);
    }

    private void addAuth(StorageRepository storageRepository) {
        if (storageRepository.getProtocol() == RepositoryProtocolEnum.S3.getProtocol()
            || storageRepository.getProtocol() == RepositoryProtocolEnum.TAPE.getProtocol()) {
            return;
        }
        if (!deployTypeService.isE1000()) {
            ClustersInfoVo clustersInfoVo = clusterNativeApi.getCurrentClusterVoInfo();
            if (clustersInfoVo.getStorageEsn().equals(storageRepository.getId())) {
                log.info("Local storage(id:{}) not need add auth", storageRepository.getId());
                return;
            }
        }
        TargetClusterRequestParm targetClusterRequestParm = new TargetClusterRequestParm();
        targetClusterRequestParm.setEsnList(Lists.newArrayList(storageRepository.getId()));
        List<ClusterDetailInfo> clusterDetailInfos = clusterNativeApi.queryTargetClusterListDetails(
            targetClusterRequestParm);
        if (CollectionUtils.isEmpty(clusterDetailInfos)) {
            log.error("Not find storage by id:{}", storageRepository.getId());
            throw new LegoCheckedException(CommonErrorCode.TARGET_CLUSTER_NOT_EXIST, "Target cluster not exist.");
        }
        storageRepository.getExtendAuth()
            .setAuthPwd(encryptorService.decrypt(clusterDetailInfos.get(0).getStorageSystem().getPassword()));
    }

    @Override
    protected void handleTask(DeleteCopyTask task, CopyInfoBo copy) {
        task.getRepositories().forEach(this::addAuth);
        DwsBuildRepositoryUtil.addRepositoriesAuth(task.getRepositories(), clusterNativeApi, encryptorService,
            deployTypeService, false);
        if (!super.isResourceExists(task)) {
            log.warn("Gaussdb dws copy resource or root resource is not exist.");
            return;
        }
        gaussDBBaseService.modifyAdvanceParams(task.getAdvanceParams(), task.getProtectObject().getUuid());
        ProtectedResource resource = gaussDBBaseService.getResourceById(task.getProtectObject().getUuid());
        ProtectedEnvironment environment = gaussDBBaseService.getEnvironmentById(resource.getRootUuid());
        TaskEnvironment protectEnv = task.getProtectEnv();
        // 用户名("DwsUser":"omm")、环境变量路径("envFile":"/home/env")放在protectEnv->extendInfo里;
        DwsTaskEnvironmentUtil.initProtectEnvOfEnvFile(protectEnv,
            environment.getExtendInfo().get(DwsConstant.EXTEND_INFO_KEY_ENV_FILE));
        DwsTaskEnvironmentUtil.initProtectEnvOfDwsUser(protectEnv, environment.getAuth().getAuthKey());
        protectEnv.getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        task.setProtectEnv(protectEnv);
        task.getProtectEnv().setNodes(gaussDBBaseService.supplyNodes(resource.getRootUuid()));
        task.setIsForceDeleted(true);
    }

    @Override
    protected void supplyAgent(DeleteCopyTask task, CopyInfoBo copy) {
        ProtectedResource resource = gaussDBBaseService.getResourceById(task.getProtectObject().getUuid());
        ProtectedResource rootResource = gaussDBBaseService.getResourceById(resource.getRootUuid());
        // 删除副本任务只能下发到内置agent上
        List<Endpoint> endpoints = OpServiceUtil.isHcsService()
            ? getHcsEndpoints(resource.getRootUuid(), rootResource)
            : getEndpoints(resource.getRootUuid(), rootResource);
        task.setAgents(endpoints);
    }

    private List<Endpoint> getEndpoints(String rootUuid, ProtectedResource rootResource) {
        ProtectedResourceChecker checker = providerManager.findProviderOrDefault(ProtectedResourceChecker.class,
            rootResource, this.protectedResourceChecker);
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap = checker.collectConnectableResources(
            rootResource);
        ProtectedEnvironment environment = gaussDBBaseService.getEnvironmentById(rootUuid);
        if (environment == null) {
            return Collections.emptyList();
        }
        List<String> clusterIps = Arrays.asList(
            environment.getExtendInfo().get(DwsConstant.DWS_CLUSTER_AGENT).split(","));
        return protectedResourceMap.values()
            .stream()
            .flatMap(List::stream)
            .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort()))
            .filter(endpoint -> clusterIps.contains(endpoint.getIp()))
            .collect(Collectors.toList());
    }

    private List<Endpoint> getHcsEndpoints(String rootUuid, ProtectedResource rootResource) {
        ProtectedResourceChecker checker = providerManager.findProviderOrDefault(ProtectedResourceChecker.class,
            rootResource, this.protectedResourceChecker);
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap = checker.collectConnectableResources(
            rootResource);
        ProtectedEnvironment environment = gaussDBBaseService.getEnvironmentById(rootUuid);
        if (environment == null) {
            return Collections.emptyList();
        }
        List<String> endpointIps = environment.getDependencies()
            .get(DwsConstant.DWS_CLUSTER_AGENT)
            .stream()
            .map(ProtectedResource::getEndpoint)
            .collect(Collectors.toList());
        return protectedResourceMap.values()
            .stream()
            .flatMap(List::stream)
            .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort()))
            .filter(endpoint -> endpointIps.contains(endpoint.getIp()))
            .collect(Collectors.toList());
    }
}
