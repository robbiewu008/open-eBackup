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
package openbackup.openstack.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.backup.v2.PostBackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.common.OpenstackQuotaService;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.openstack.protection.access.dto.VolInfo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.enums.RetentionTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import com.huawei.oceanprotect.system.base.quota.enums.UpdateQuotaType;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.DeleteExcessCopiesRequest;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import com.alibaba.fastjson.JSON;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 功能描述: Openstack备份拦截器
 *
 */
@Slf4j
@Component
public class OpenstackBackupProvider implements BackupInterceptorProvider {
    private final ResourceService resourceService;
    private final OpenstackQuotaService openstackQuotaService;
    private final CopyRestApi copyRestApi;
    private final ClusterNativeApi clusterNativeApi;

    public OpenstackBackupProvider(ResourceService resourceService,
        OpenstackQuotaService openstackQuotaService, CopyRestApi copyRestApi, ClusterNativeApi clusterNativeApi) {
        this.resourceService = resourceService;
        this.openstackQuotaService = openstackQuotaService;
        this.copyRestApi = copyRestApi;
        this.clusterNativeApi = clusterNativeApi;
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.equalsSubType(subType);
    }

    @Override
    public BackupTask initialize(BackupTask backupTask) {
        log.info("Openstack backup intercept start, taskId: {}, backupType: {}, resourceId: {}.",
            backupTask.getTaskId(), backupTask.getBackupType(), backupTask.getProtectObject().getUuid());
        // 云核场景检查
        registerOpenstackCheck(backupTask);
        // 填充磁盘卷信息
        fillSubObjects(backupTask);
        // 填充域认证信息
        fillDomainAuth(backupTask);
        // 填充存储库信息
        fillRepositories(backupTask);
        // 填充存储库esn
        fillEsn(backupTask);
        log.info("Openstack backup intercept finished, taskId: {}, backupType: {}, resourceId: {}.",
            backupTask.getTaskId(), backupTask.getBackupType(), backupTask.getProtectObject().getUuid());
        return backupTask;
    }

    @Override
    public void finalize(PostBackupTask postBackupTask) {
        log.info("openstack backup post process start.");
        // 按数量保留副本时，删除多余副本
        deleteExcessCopies(postBackupTask);
        ProtectedObject protectedObject = postBackupTask.getProtectedObject();
        if (Objects.isNull(protectedObject) || StringUtils.isBlank(protectedObject.getEnvUuid())
            || StringUtils.isBlank(protectedObject.getResourceId()) || Objects.isNull(postBackupTask.getCopyInfo())) {
            log.error("openstack backup post process failed, protectedObject param is blank");
            return;
        }
        resourceService.getResourceById(false, protectedObject.getEnvUuid())
            .filter(openstackQuotaService::isRegisterOpenstack)
            .ifPresent(extend -> {
                ProtectedResource resource = getProtectedResource(protectedObject.getResourceId());
                openstackQuotaService.updateUsedQuota(resource.getParentUuid(), postBackupTask.getCopyInfo(),
                    UpdateQuotaType.INCREASE);
            });
    }

    private void fillEsn(BackupTask backupTask) {
        Map<String, String> advanceParams = backupTask.getAdvanceParams();
        String currentEsn = clusterNativeApi.getCurrentEsn();
        advanceParams.put(OpenstackConstant.ESN, currentEsn);
        backupTask.setAdvanceParams(advanceParams);
        log.info("openstack fill esn, taskId: {}, esn: {}", backupTask.getTaskId(), currentEsn);
    }

    private void fillDomainAuth(BackupTask backupTask) {
        TaskResource protectObject = backupTask.getProtectObject();
        String domainId = protectObject.getExtendInfo().get(OpenstackConstant.DOMAIN_ID_KEY);
        if (Objects.equals(domainId, OpenstackConstant.DEFAULT_DOMAIN_ID)) {
            domainId = backupTask.getProtectEnv().getExtendInfo().get(OpenstackConstant.SERVICE_ID_KEY);
        }
        boolean shouldDecrypt = true;
        resourceService.getResourceById(shouldDecrypt, domainId)
            .map(ProtectedResource::getAuth)
            .ifPresent(protectObject::setAuth);
        log.info("openstack backup fill domain info, taskId: {}, domainId: {}", backupTask.getTaskId(), domainId);
    }

    private void registerOpenstackCheck(BackupTask task) {
        TaskEnvironment protectEnv = task.getProtectEnv();
        Optional.ofNullable(protectEnv)
            .map(this::convertProtectedResource)
            .filter(openstackQuotaService::isRegisterOpenstack)
            .map(ProtectedResource::getExtendInfo)
            .flatMap(extendInfo -> Optional.ofNullable(extendInfo.get(OpenstackConstant.QUOTA_USER_ID)))
            .ifPresent(userId -> {
                openstackQuotaService.checkBackupQuota(userId, task.getProtectObject().getParentUuid());
                task.addParameter(OpenstackConstant.OPEN_CONSISTENT_SNAPSHOTS, Boolean.TRUE.toString());
            });
    }

    private ProtectedResource convertProtectedResource(TaskEnvironment taskEnvironment) {
        ProtectedResource protectedResource = new ProtectedResource();
        BeanTools.copy(taskEnvironment, protectedResource);
        return protectedResource;
    }

    private ProtectedResource getProtectedResource(String resourceId) {
        return resourceService.getResourceById(false, resourceId)
            .orElseThrow(() ->
                new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "openstack resource is not exist"));
    }

    private void fillSubObjects(BackupTask backupTask) {
        Map<String, String> advanceParams = backupTask.getAdvanceParams();
        String allDisk = advanceParams.get(OpenstackConstant.ALL_DISK);
        if (Boolean.TRUE.toString().equals(allDisk)) {
            return;
        }
        TaskResource protectObject = backupTask.getProtectObject();
        String volInfoStr = protectObject.getExtendInfo().get(OpenstackConstant.VOLUME_INFO_KEY);
        List<VolInfo> hostDiskInfos = JSON.parseArray(volInfoStr, VolInfo.class);
        String diskIdStr = advanceParams.get(OpenstackConstant.DISK_IDS).trim().replaceAll(" ", "");
        // List格式的磁盘信息转化时调用了toString导致数组字符串中缺少引号，去掉括号以逗号分割
        List<String> protectedDiskIds = Arrays.asList(diskIdStr.substring(1, diskIdStr.length() - 1).split(","));
        log.info("backup taskId:{}, protectedDiskIds:{}", backupTask.getTaskId(), protectedDiskIds);
        List<TaskResource> protectSubObjects = getProtectSubObjects(protectedDiskIds, hostDiskInfos);
        backupTask.setProtectSubObjects(protectSubObjects);
    }

    private List<TaskResource> getProtectSubObjects(List<String> diskIdList, List<VolInfo> diskInfoList) {
        return diskInfoList.stream()
                .filter(diskInfo -> diskIdList.contains(diskInfo.getId()))
                .map(this::convertToTaskResource)
                .collect(Collectors.toList());
    }

    private TaskResource convertToTaskResource(VolInfo volInfo) {
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid(volInfo.getId());
        taskResource.setName(volInfo.getName());
        return taskResource;
    }

    private void fillRepositories(BackupTask task) {
        List<StorageRepository> repositories = task.getRepositories();
        // 添加存储仓库类型：2-CACHE_REPOSITORY
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);
        task.setRepositories(repositories);
    }

    private void deleteExcessCopies(PostBackupTask postBackupTask) {
        PolicyBo policyBo = postBackupTask.getPolicyBo();

        if (policyBo == null || !RetentionTypeEnum.QUANTITY.getType()
            .equals(policyBo.getRetention().getRetentionType())) {
            return;
        }

        DeleteExcessCopiesRequest request = new DeleteExcessCopiesRequest();
        String resourceId = postBackupTask.getProtectedObject().getResourceId();
        request.setRetentionQuantity(policyBo.getRetention().getRetentionQuantity());
        request.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        request.setUserId(postBackupTask.getUserId());

        try {
            copyRestApi.deleteExcessCopies(resourceId, request);
            log.info("Delete resource: {} excess: {} copies success, user id: {}.", resourceId,
                policyBo.getRetention().getRetentionQuantity(), request.getUserId());
        } catch (Exception exception) {
            log.error("Delete resource: {} excess copies occur an exception.", resourceId,
                ExceptionUtil.getErrorMessage(exception));
        }
    }
}
