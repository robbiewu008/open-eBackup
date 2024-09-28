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
package openbackup.obs.plugin.provider;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.obs.plugin.common.constants.EnvironmentConstant;
import openbackup.obs.plugin.service.ObjectStorageAgentService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.service.DeployTypeService;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

/**
 * 对象备份恢复任务拦截器
 *
 */
@Component
@Slf4j
public class ObjectStorageRestoreProvider implements RestoreInterceptorProvider {
    private static final String KEY_NAME = "name";

    private static final String KEY_BUCKET_NAME = "bucketName";

    private static final String KEY_EXTEND_INFO = "extendInfo";

    private static final String IS_OCEAN_SMALL_FILES = "is_ocean_small_files";

    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private EncryptorService encryptorService;

    @Autowired
    private ObjectStorageAgentService agentService;

    @Autowired
    private DeployTypeService deployTypeService;

    @Override
    public boolean applicable(String subType) {
        // 恢复新位置时，参数中targetObject值为对象存储id
        return StringUtils.equals(subType, ResourceSubTypeEnum.OBJECT_SET.getType())
            || StringUtils.equals(subType, ResourceSubTypeEnum.OBJECT_STORAGE.getType());
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        if (task.getTargetEnv() == null) {
            log.error("TargetEnv is empty.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "TargetEnv is illegal.");
        }

        if (task.getTargetObject() == null) {
            log.error("TargetObject is empty.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "TargetObject is illegal.");
        }

        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);

        // 校验原始资源是否存在
        checkOriginalResource(task);

        // 设置storageType
        checkStorageType(task);

        // 解密SK 和 代理密码
        decryptData(task);

        checkAgents(task);
        if (deployTypeService.isPacific()) {
            task.getAdvanceParams().put(IS_OCEAN_SMALL_FILES, String.valueOf(true));
        }

        // 设置恢复模式
        setRestoreMode(task);
        log.info("object restore success. taskId: {}", task.getTaskId());
        return task;
    }

    private void checkOriginalResource(RestoreTask task) {
        if (task.getTargetLocation() == RestoreLocationEnum.ORIGINAL) {
            String uuid = task.getTargetObject().getUuid();
            Optional<ProtectedResource> resourceById = resourceService.getResourceById(uuid);
            if (!resourceById.isPresent()) {
                throw new LegoCheckedException(CommonErrorCode.RESOURCE_IS_NOT_EXIST, "original resource is not exist");
            }
        }
    }

    private void checkStorageType(RestoreTask task) {
        String storageType = task.getTargetEnv().getExtendInfo().get(EnvironmentConstant.KEY_STORAGE_TYPE);
        if (StringUtils.isEmpty(storageType)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "storageType is not exist");
        }
        if (task.getTargetEnv().getAuth() == null) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "TargetEnv auth is not exist");
        }
        task.getTargetEnv().getAuth().getExtendInfo().put(EnvironmentConstant.KEY_STORAGE_TYPE, storageType);
    }

    private void decryptData(RestoreTask task) {
        // 解密sk
        String decryptedSk = encryptorService.decrypt(
            task.getTargetEnv().getAuth().getExtendInfo().get(EnvironmentConstant.KEY_SK));
        task.getTargetEnv().getAuth().getExtendInfo().put(EnvironmentConstant.KEY_SK, decryptedSk);

        // 开启代理需要解密代理密码
        if (isEnableProxy(task)) {
            String decryptedPwd = encryptorService.decrypt(
                task.getTargetEnv().getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_USER_PWD));
            task.getTargetEnv().getAuth().getExtendInfo().put(EnvironmentConstant.KEY_PROXY_USER_PWD, decryptedPwd);
        }
    }

    private void setRestoreMode(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        String generatedBy = copy.getGeneratedBy();
        if (CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value().equals(generatedBy)) {
            task.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
        } else if (CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value().equals(generatedBy)) {
            task.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
        } else {
            task.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }
        log.info("redis restore copy id: {}, mode: {}", task.getCopyId(), task.getRestoreMode());
    }

    private void checkAgents(RestoreTask task) {
        String agents = task.getTargetEnv().getExtendInfo().get(EnvironmentConstant.KEY_AGENTS);
        List<Endpoint> objectStorageEndpoint = agentService.getObjectStorageEndpoint(agents);
        if (CollectionUtils.isNotEmpty(objectStorageEndpoint)) {
            task.setAgents(objectStorageEndpoint);
        }
    }

    private boolean isEnableProxy(RestoreTask task) {
        String proxyEnable = task.getTargetEnv().getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_ENABLE);
        return StringUtils.equals(proxyEnable, EnvironmentConstant.PROXY_ENABLE_VALUE);
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        log.info("start restore lock bucket resources, task id = {}", task.getTaskId());
        String envId = Optional.ofNullable(task.getTargetEnv()).map(item -> item.getUuid()).orElse("");
        String buck = Optional.ofNullable(task.getAdvanceParams()).map(item -> item.get(KEY_BUCKET_NAME)).orElse("");

        // 如果advanceParams中没有bucketName，说明没有指定恢复桶，则取用原集合中的桶名。
        List<String> buckListArray = new ArrayList<>();
        if (StringUtils.isEmpty(buck)) {
            buckListArray = getBuckListByCopy(task, envId);
        } else {
            buckListArray.add(buck + "_" + envId);
        }
        List<LockResourceBo> lockList = new ArrayList<>();
        for (String buckId : buckListArray) {
            lockList.add(new LockResourceBo(buckId, LockType.WRITE));
            log.info("add write lock to bucket[{}] in restore, task id = {}", buckId, task.getTaskId());
        }
        return lockList;
    }

    private List<String> getBuckListByCopy(RestoreTask task, String envId) {
        String copyId = task.getCopyId();
        if (copyId == null) {
            log.error("not find copyId, task id is {}", task.getTaskId());
            return new ArrayList<>();
        }
        Copy copy = copyRestApi.queryCopyByID(copyId);
        if (copy == null) {
            log.error("not find copy by copy id, task id is {}", task.getTaskId());
            return new ArrayList<>();
        }

        JSONArray bucketIdList = Optional.ofNullable(JSONObject.toBean(copy.getResourceProperties(), JSONObject.class))
            .map(item -> item.getJSONObject(KEY_EXTEND_INFO))
            .map(extendInfo -> extendInfo.getJSONArray(EnvironmentConstant.KEY_BUCKETLIST))
            .orElse(new JSONArray());

        return genBuckName(bucketIdList, envId);
    }

    private static List<String> genBuckName(JSONArray buckListjsonArray, String envId) {
        List<String> buckListArray = new ArrayList<>();
        for (Object buckId : buckListjsonArray) {
            buckListArray.add((JSONObject.cast(buckId, JSONObject.class)).get(KEY_NAME) + "_" + envId);
        }
        return buckListArray;
    }
}
