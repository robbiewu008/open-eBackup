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
package openbackup.data.access.framework.copy.index.provider;

import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.common.collect.ImmutableList;

import openbackup.access.framework.resource.service.ProtectedResourceEvent;
import openbackup.access.framework.resource.service.ProtectedResourceMonitorService;
import openbackup.data.access.framework.copy.index.service.IvmFileLevelRestoreService;
import openbackup.data.access.framework.core.common.constants.CopyIndexConstants;
import openbackup.data.protection.access.provider.sdk.job.Task;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.RestoreObject;
import openbackup.data.protection.access.provider.sdk.restore.RestoreRequest;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.DataMoverCheckedException;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.CreateJobRequest;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.apache.commons.lang3.StringUtils;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 虚拟化应用细粒度恢复provider抽象类
 *
 */
public abstract class AbstractVmFileLevelRestoreProvider {
    private static final int MAX_TARGET_NAME_LENGTH = 80;

    private static final int MIN_TARGET_NAME_LENGTH = 1;

    private static final List<String> SUPPORTED_GENERATE_TYPE = ImmutableList.of(
            CopyGeneratedByEnum.BY_BACKUP.value(),
            CopyGeneratedByEnum.BY_REPLICATED.value(),
            CopyGeneratedByEnum.BY_CASCADED_REPLICATION.value(),
            CopyGeneratedByEnum.BY_REVERSE_REPLICATION.value()
    );

    private final CopyRestApi copyRestApi;

    private final ResourceService resourceService;

    private final ProtectedResourceMonitorService protectedResourceMonitorService;

    private final IvmFileLevelRestoreService flrService;

    private final ResourceSetApi resourceSetApi;

    /**
     * 构造
     *
     * @param copyRestApi copyRestApi
     * @param resourceService resourceService
     * @param protectedResourceMonitorService protectedResourceMonitorService
     * @param flrService flrService
     * @param resourceSetApi resourceSetApi
     */
    public AbstractVmFileLevelRestoreProvider(CopyRestApi copyRestApi, ResourceService resourceService,
        ProtectedResourceMonitorService protectedResourceMonitorService, IvmFileLevelRestoreService flrService,
        ResourceSetApi resourceSetApi) {
        this.copyRestApi = copyRestApi;
        this.resourceService = resourceService;
        this.protectedResourceMonitorService = protectedResourceMonitorService;
        this.flrService = flrService;
        this.resourceSetApi = resourceSetApi;
    }

    /**
     * 检查是否是支持生成方式
     *
     * @param restoreObject 副本信息
     * @return true/false
     */
    protected boolean checkGeneratedType(RestoreObject restoreObject) {
        if (restoreObject == null) {
            return false;
        }
        String generatedBy = restoreObject.getCopyGeneratedBy();
        return SUPPORTED_GENERATE_TYPE.contains(generatedBy);
    }

    /**
     * 生成快照元数据信息
     *
     * @param properties 配置信息
     * @return 元数据信息
     */
    protected abstract String buildSnapMetadata(JSONObject properties);

    /**
     * 执行恢复
     *
     * @param restoreObject 恢复对象
     * @return Task
     */
    protected final Task doRestore(RestoreObject restoreObject) {
        return CopyIndexConstants.FLR_RESTORE_TYPE.equals(restoreObject.getRestoreType())
            ? flrService.fileLevelRestore(restoreObject, createSnapMetaData(restoreObject))
            : flrService.download(restoreObject, createSnapMetaData(restoreObject));
    }

    /**
     * 创建恢复任务
     * {
     * "copy_id": "da52f2e3-682a-44a3-8668-08069b087f55",
     * "object_type": "vim.VirtualMachine",
     * "restore_location": "O",
     * "filters": [],
     * "restore_objects": [
     * "/bin"
     * ],
     * "restore_type": "FLR",
     * "target": {
     * "details": [],
     * "env_id": "639c07b4-c570-58c4-a6be-6ba9feab0b3d",
     * "env_type": "vim.VirtualMachine",
     * "restore_target": ""
     * },
     * "source": {
     * "source_location": "8.40.162.45/liantong_dc/liantong/",
     * "source_name": "centos73_10-对象存储副本恢复"
     * },
     * "ext_parameters": {
     * "vm_name": "centos73_10-对象存储副本恢复",
     * "USER_NAME": "root",
     * "PASSWORD": "Huawei@123",
     * "VM_IP": "192.168.122.1",
     * "FILE_REPLACE_STRATEGY": "OVERWRITING"
     * }
     * }
     *
     * @param restoreRequest restoreRequest
     * @return String
     * @throws JsonProcessingException JsonProcessingException
     */
    protected final String doCreateRestoreTask(RestoreRequest restoreRequest) throws JsonProcessingException {
        Copy copy = copyRestApi.queryCopyByID(restoreRequest.getCopyId());
        if (copy == null) {
            throw new DataMoverCheckedException("no matched service found", CommonErrorCode.ERR_PARAM);
        }
        checkRestoreParameters(restoreRequest);
        return new ObjectMapper().writeValueAsString(createJobRequest(restoreRequest, copy));
    }

    /**
     * 创建snap元数据
     *
     * @param restoreObject 恢复对象
     * @return snap元数据
     */
    protected final String createSnapMetaData(RestoreObject restoreObject) {
        Copy copy = copyRestApi.queryCopyByID(restoreObject.getCopyId());
        JSONObject properties = JSONObject.fromObject(copy.getProperties());
        return buildSnapMetadata(properties);
    }

    private CreateJobRequest createJobRequest(RestoreRequest restoreRequest, Copy copy) {
        CreateJobRequest job = new CreateJobRequest();
        job.setCopyId(restoreRequest.getCopyId());
        job.setCopyTime(Long.parseLong(copy.getTimestamp()));
        job.setRequestId(restoreRequest.getRequestId());
        job.setSourceId(copy.getResourceId());
        job.setSourceName(restoreRequest.getSource().getSourceName());
        job.setSourceType(ResourceTypeEnum.VM.getType());
        job.setSourceSubType(restoreRequest.getObjectType());
        job.setSourceLocation(restoreRequest.getSource().getSourceLocation());
        job.setType(JobTypeEnum.RESTORE.name());
        Map<String, String> parameters = Optional.ofNullable(restoreRequest.getParameters())
            .orElseGet(Collections::emptyMap);
        job.setTargetName(parameters.get("vm_name"));
        job.setTargetLocation("O".equals(restoreRequest.getRestoreLocation())
            ? restoreRequest.getSource().getSourceLocation()
            : restoreRequest.getTarget().getRestoreTarget());
        job.setExtendField(buildRestoreExtendField(restoreRequest, copy.getProperties()));
        job.setDomainIdList(resourceSetApi.getRelatedDomainIdList(restoreRequest.getCopyId()));
        return job;
    }

    private JSONObject buildRestoreExtendField(RestoreRequest restoreRequest, String copyPropertiesStr) {
        JSONObject extendField = new JSONObject();
        extendField.set("restoreType", restoreRequest.getRestoreType());
        JSONObject copyProperties = JSONObject.fromObject(copyPropertiesStr);
        if (StringUtils.isNotEmpty(copyProperties.getString("storage_id"))) {
            extendField.set("storageId", copyProperties.getString("storage_id"));
        }
        return extendField;
    }

    private void checkRestoreParameters(RestoreRequest restoreRequest) {
        checkRestoreVmName(restoreRequest.getParameters().get("vm_name"));
        checkIfCenterOnline(restoreRequest);
    }

    private void checkRestoreVmName(String name) {
        if (!VerifyUtil.isEmpty(name)) {
            if ((name.length() < MIN_TARGET_NAME_LENGTH) || (name.length() > MAX_TARGET_NAME_LENGTH)) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                    "The VM name does not meet the verification rule.");
            }
        }
    }

    private void checkIfCenterOnline(RestoreRequest restoreRequest) {
        if (StringUtils.isBlank(restoreRequest.getTarget().getEnvId())) {
            return;
        }
        ProtectedResource protectedResource = obtainProtectedResource(restoreRequest.getTarget().getEnvId());
        if (protectedResource != null && LinkStatusEnum.getByStatus(
            Integer.valueOf(protectedResource.getEnvironment().getLinkStatus())).equals(LinkStatusEnum.OFFLINE)) {
            throw new DataMoverCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                new String[] {"resource link status is offline"});
        }
    }

    private ProtectedResource obtainProtectedResource(String resourceId) {
        ProtectedResource resource = resourceService.getResourceById(false, resourceId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "resource not exist"));
        protectedResourceMonitorService.invoke("desensitize", resource, ProtectedResourceEvent::getResource);
        return resource;
    }
}
