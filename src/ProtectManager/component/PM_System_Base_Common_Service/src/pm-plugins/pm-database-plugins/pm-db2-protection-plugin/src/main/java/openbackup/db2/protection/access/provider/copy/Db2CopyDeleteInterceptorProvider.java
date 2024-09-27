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
package openbackup.db2.protection.access.provider.copy;

import openbackup.data.access.client.sdk.api.framework.dme.AvailableTimeRanges;
import openbackup.data.access.framework.copy.mng.service.CopyService;
import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.access.framework.core.common.enums.DmeJobStatusEnum;
import openbackup.data.access.framework.servitization.entity.VpcInfoEntity;
import openbackup.data.access.framework.servitization.service.IVpcService;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupChangeCauseEnum;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupModifyReq;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.db2.protection.access.service.Db2Service;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.OpServiceUtil;

import com.alibaba.fastjson.JSON;
import com.google.common.collect.ImmutableList;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * db2副本删除provider
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-01-13
 */
@Component
@Slf4j
public class Db2CopyDeleteInterceptorProvider extends AbstractDbCopyDeleteInterceptor {
    private final Db2Service db2Service;

    private final InstanceResourceService instanceResourceService;

    private final ResourceService resourceService;

    private CopyService copyService;

    private IVpcService vpcService;

    /**
     * 构造函数
     *
     * @param copyRestApi 副本api
     * @param resourceService 资源服务
     * @param db2Service db2服务
     * @param instanceResourceService 实例资源服务
     * @param resourceService 保护对象服务
     */
    public Db2CopyDeleteInterceptorProvider(CopyRestApi copyRestApi, ResourceService resourceService,
        Db2Service db2Service, InstanceResourceService instanceResourceService) {
        super(copyRestApi, resourceService);
        this.db2Service = db2Service;
        this.instanceResourceService = instanceResourceService;
        this.resourceService = resourceService;
    }

    @Autowired
    public void setCopyService(CopyService copyService) {
        this.copyService = copyService;
    }

    @Autowired
    public void setVpcService(IVpcService vpcService) {
        this.vpcService = vpcService;
    }

    /**
     * 删除全量副本时，要删除此副本到下一个全量副本之间的副本
     *
     * @param copies 本个副本之后的所有备份副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    @Override
    protected List<String> getCopiesCopyTypeIsFull(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        List<Copy> associatedCopies = CopyUtil.getCopiesBetweenTwoCopy(copies, thisCopy, nextFullCopy);
        Copy firstLogCopy = copies.stream()
            .filter(copy -> copy.getBackupType() == BackupTypeConstants.LOG.getAbBackupType())
            .findFirst()
            .orElse(null);
        List<String> copyUuids = associatedCopies.stream().map(Copy::getUuid).collect(Collectors.toList());
        if (firstLogCopy == null) {
            return copyUuids;
        }
        String thisCopyBakTime = JSONObject.fromObject(thisCopy.getProperties())
            .getString(DatabaseConstants.COPY_BACKUP_TIME_KEY);
        JSONObject firstLogCopyPropertyJson = JSONObject.fromObject(firstLogCopy.getProperties());
        String firstLogCopyStartTime = firstLogCopyPropertyJson.getString(DatabaseConstants.LOG_COPY_BEGIN_TIME_KEY);
        if (Objects.equals(firstLogCopyStartTime, thisCopyBakTime)) {
            return copyUuids;
        }
        long firLogCopyStartTimestamp = Long.parseLong(firstLogCopyStartTime);
        if (firLogCopyStartTimestamp > Long.parseLong(thisCopyBakTime)) {
            return copyUuids;
        }
        PageListResponse<AvailableTimeRanges> timeRangesResp;
        try {
            timeRangesResp = copyService.listAvailableTimeRanges(thisCopy.getResourceId(), firLogCopyStartTimestamp,
                firLogCopyStartTimestamp + 1, 100, 0);
        } catch (LegoCheckedException e) {
            log.warn("Call list available time ranges interface of dme failed.", e);
            return Collections.emptyList();
        }
        if (timeRangesResp.getTotalCount() == 0) {
            return copyUuids;
        }
        return associatedCopies.stream()
            .filter(copy -> copy.getBackupType() != BackupTypeConstants.LOG.getAbBackupType())
            .map(Copy::getUuid)
            .collect(Collectors.toList());
    }

    /**
     * 删除增量副本时，要删除此副本到下一个全量副本之间的所有增量副本
     *
     * @param copies 本个副本之后的所有备份副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    @Override
    protected List<String> getCopiesCopyTypeIsDifferenceIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        return getAssociatedTypeCopiesByBackup(copies, thisCopy, nextFullCopy,
            ImmutableList.of(BackupTypeConstants.DIFFERENCE_INCREMENT));
    }

    @Override
    protected void supplyAgent(DeleteCopyTask task, CopyInfoBo copy) {
        task.setAgents(
            db2Service.getAgentsByInstanceResource(instanceResourceService.getResourceById(copy.getResourceId())));
        fillAdvanceParams(task);
    }

    private void fillAdvanceParams(DeleteCopyTask task) {
        if (!OpServiceUtil.isHcsService()) {
            return;
        }
        log.info("This is a hcs deploy environment, need set vpc info. task id: {}", task.getTaskId());
        Set<String> vpcIds = getVpcIds(task.getAgents());
        if (CollectionUtils.isNotEmpty(vpcIds)) {
            log.info("vpcIds = {}", StringUtils.join(vpcIds, ","));
            List<VpcInfoEntity> vpcInfoEntities = vpcService.getVpcByVpcIds(vpcIds);
            Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams())
                .orElse(new HashMap<>());
            advanceParams.put(DatabaseConstants.VPC_INFO_KEY, JSON.toJSONString(vpcInfoEntities));
            task.setAdvanceParams(advanceParams);
        }
    }

    private Set<String> getVpcIds(List<Endpoint> agents) {
        if (CollectionUtils.isEmpty(agents)) {
            return new HashSet<>();
        }
        return agents.stream()
            .map(Endpoint::getId)
            .map(resUuid -> resourceService.getBasicResourceById(false, resUuid))
            .map(protectedResource -> protectedResource.get().getExtendInfoByKey(ResourceConstants.VPC_ID))
            .filter(StringUtils::isNotBlank)
            .collect(Collectors.toSet());
    }

    @Override
    protected void handleTask(DeleteCopyTask task, CopyInfoBo copy) {
        task.setIsForceDeleted(true);
        if (!super.isResourceExists(task)) {
            return;
        }
        setProtectEnvExtendInfo(task);
        setProtectEnvNodes(task);
    }

    private void setProtectEnvExtendInfo(DeleteCopyTask task) {
        Map<String, String> envExtendInfo = Optional.ofNullable(task.getProtectEnv().getExtendInfo())
            .orElseGet(HashMap::new);
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, Db2ClusterTypeEnum.getDeployType(task.getProtectEnv()
            .getExtendInfo()
            .getOrDefault(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.SINGLE.getType())));
        task.getProtectEnv().setExtendInfo(envExtendInfo);
    }

    private void setProtectEnvNodes(DeleteCopyTask task) {
        ProtectedResource resource = instanceResourceService.getResourceById(task.getProtectObject().getUuid());
        List<TaskEnvironment> nodes = db2Service.getEnvNodesByInstanceResource(resource);
        task.getProtectEnv().setNodes(nodes);
    }

    @Override
    public void finalize(Copy copy, TaskCompleteMessageBo taskMessage) {
        int jobStatus = taskMessage.getJobStatus();
        String requestId = taskMessage.getJobRequestId();
        log.info("Db2 copy delete post process. job status is {}, request id: {}", jobStatus, requestId);
        if (!Objects.equals(DmeJobStatusEnum.fromStatus(jobStatus), DmeJobStatusEnum.SUCCESS)) {
            return;
        }
        List<String> relatedResource = addResourcesSetNextFull(copy, requestId);
        if (VerifyUtil.isEmpty(relatedResource)) {
            log.info("Db2 related resource is empty, request id: {}, copy id: {}.", requestId, copy.getUuid());
            return;
        }
        log.info("Db2 related resource is: {}, request id: {}, copy id: {}.", copy.getResourceId(), requestId,
            copy.getUuid());
        setNextBackupToFull(relatedResource);
    }

    private List<String> addResourcesSetNextFull(Copy copy, String requestId) {
        if (!Objects.equals(CopyGeneratedByEnum.BY_BACKUP.value(), copy.getGeneratedBy())) {
            return Collections.emptyList();
        }
        Map<String, Object> copyFilter = new HashMap<>();
        copyFilter.put(DatabaseConstants.COPY_BACKUP_TYPE_KEY, BackupTypeEnum.FULL.getAbbreviation());
        Copy latestFullCopy = copyRestApi.queryLatestBackupCopy(copy.getResourceId(), null, copyFilter);
        if (VerifyUtil.isEmpty(latestFullCopy)) {
            log.info("Db2 latest full copy is empty. request id: {}", requestId);
            return Collections.emptyList();
        }
        if (latestFullCopy.getGn() > copy.getGn()) {
            return Collections.emptyList();
        }
        switch (BackupTypeConstants.getBackupTypeByAbBackupType(copy.getBackupType())) {
            case FULL:
            case DIFFERENCE_INCREMENT:
                return Collections.singletonList(copy.getResourceId());
            default:
                return Collections.emptyList();
        }
    }

    private void setNextBackupToFull(List<String> relatedResource) {
        NextBackupModifyReq nextBackupModifyReq = NextBackupModifyReq.build(relatedResource,
                NextBackupChangeCauseEnum.LATEST_FULL_OR_INCREMENTAL_COPY_DELETE_SUCCESS_TO_FULL_LABEL);
        resourceService.modifyNextBackup(nextBackupModifyReq, false);
    }

    @Override
    public boolean applicable(String subType) {
        return Arrays.asList(ResourceSubTypeEnum.DB2_DATABASE.getType(), ResourceSubTypeEnum.DB2_TABLESPACE.getType())
            .contains(subType);
    }
}
