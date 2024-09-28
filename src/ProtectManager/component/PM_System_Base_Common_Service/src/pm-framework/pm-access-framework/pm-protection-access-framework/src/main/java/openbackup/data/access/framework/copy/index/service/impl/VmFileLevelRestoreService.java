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
package openbackup.data.access.framework.copy.index.service.impl;

import openbackup.access.framework.resource.service.ProtectedResourceEvent;
import openbackup.access.framework.resource.service.ProtectedResourceMonitorService;
import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;
import openbackup.data.access.framework.copy.index.service.IvmFileLevelRestoreService;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.VmRestoreTypeEnum;
import openbackup.data.access.framework.core.common.model.RestoreStorageInfo;
import openbackup.data.access.framework.core.common.model.VmFlrDestInfo;
import openbackup.data.access.framework.core.common.model.VmFlrRequest;
import openbackup.data.protection.access.provider.sdk.base.Parameter;
import openbackup.data.protection.access.provider.sdk.job.Task;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.RestoreObject;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.kmc.EncryptorRestApi;
import openbackup.system.base.sdk.kmc.model.PlaintextVo;
import openbackup.system.base.sdk.storage.StorageRestClient;

import lombok.extern.slf4j.Slf4j;

import org.apache.logging.log4j.util.Strings;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.core.KafkaTemplate;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 虚拟化应用文件集恢复服务
 *
 */
@Component
@Slf4j
public class VmFileLevelRestoreService implements IvmFileLevelRestoreService {
    private static final String OLD_LOCATION = "O";

    private static final String DEFAULT_STORAGE_POOL = "0";

    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private StorageRestClient storageRestClient;

    @Autowired
    private KafkaTemplate kafka;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private JobCenterRestApi jobCenterRestApi;

    @Autowired
    private EncryptorRestApi encryptorRestApi;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private ProtectedResourceMonitorService protectedResourceMonitorService;

    @Autowired
    private StorageUnitService storageUnitService;

    @Override
    public Task fileLevelRestore(RestoreObject restoreObject, String snapMetaData) {
        // 1.向dee-parser发送恢复请求
        sendFlrRequestMsg(restoreObject, snapMetaData);

        // 2.更新副本状态恢复中，由于支持恢复并发，去掉该更新
        // 3.更新任务状态
        updateJobRunning(restoreObject);

        return newTaskFromRequestId(restoreObject.getRequestId());
    }

    private void sendFlrRequestMsg(RestoreObject restoreObject, String snapMetaData) {
        checkOs(restoreObject);
        VmFlrRequest vmFlrRequest = createVmFlrRequest(restoreObject, snapMetaData);
        kafka.send(TopicConstants.FLR_TOPIC, JSONObject.fromObject(vmFlrRequest).toString());
        log.info("VM application flr send success, requestId : {}", restoreObject.getRequestId());
    }

    private void updateJobRunning(RestoreObject restoreObject) {
        RMap<String, String> map = redissonClient.getMap(restoreObject.getRequestId(), StringCodec.INSTANCE);
        // 设置副本id,作用：RestoreTaskCompleteHandler处理时可以更新副本状态
        map.put("restore_copy_id", restoreObject.getCopyId());
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        updateJobRequest.setStatus(JobStatusEnum.RUNNING);
        jobCenterRestApi.updateJob(map.get(ContextConstants.JOB_ID), updateJobRequest);
    }

    private Task newTaskFromRequestId(String requestId) {
        Task task = new Task();
        task.setUuid(requestId);
        return task;
    }

    private VmFlrRequest createVmFlrRequest(RestoreObject restoreObject, String snapMetaData) {
        VmFlrRequest request = new VmFlrRequest();
        request.setJobId(getFlrJobId(restoreObject));
        request.setRequestId(restoreObject.getRequestId());
        request.setStorageInfo(getStorageInfo());
        request.setPaths(restoreObject.getRestoreObjects());
        request.setDestInfo(getFlrDestInfoFromParams(restoreObject.getParameters()));
        request.setReplaceMode(getReplaceModeFromParams(restoreObject.getParameters()));

        // 设置快照信息, 加载副本信息
        Copy copy = copyRestApi.queryCopyByID(restoreObject.getCopyId());
        request.setSnapId(copy.getUuid());
        request.setSnapType(copy.getGeneratedBy());
        request.setSnapMetadata(snapMetaData);
        request.setResourceSubType(copy.getResourceSubType());
        // 下发DEE参数，适配软硬解耦
        Optional<StorageUnitVo> storageUnitVoOptional = storageUnitService.getStorageUnitById(copy.getStorageUnitId());
        if (storageUnitVoOptional.isPresent()) {
            StorageUnitVo storageUnitVo = storageUnitVoOptional.get();
            request.setStorageId(storageUnitVo.getPoolId());
            request.setDeviceId(storageUnitVo.getDeviceId());
        } else {
            // 取默认值
            request.setStorageId(DEFAULT_STORAGE_POOL);
            request.setDeviceId(copy.getDeviceEsn());
        }
        return request;
    }

    private String getReplaceModeFromParams(List<Parameter> parameters) {
        Map<String, String> parameterMap = parameters.stream()
            .collect(Collectors.toMap(Parameter::getKey, Parameter::getValue, (k1, k2) -> k1));
        return VmRestoreTypeEnum.valueOf(parameterMap.get("FILE_REPLACE_STRATEGY"))
            .name()
            .toLowerCase(Locale.getDefault());
    }

    private String getFlrJobId(RestoreObject restoreObject) {
        Map<String, String> jobMap = redissonClient.getMap(restoreObject.getRequestId(), StringCodec.INSTANCE);
        return jobMap.get(ContextConstants.JOB_ID);
    }

    private VmFlrDestInfo getFlrDestInfoFromParams(List<Parameter> parameters) {
        Map<String, String> parameterMap = parameters.stream()
            .collect(Collectors.toMap(Parameter::getKey, Parameter::getValue, (k1, k2) -> k1));
        VmFlrDestInfo destInfo = new VmFlrDestInfo();
        destInfo.setUsername(parameterMap.get("USER_NAME"));
        destInfo.setVmIp(parameterMap.get("VM_IP"));
        destInfo.setPassword(parameterMap.get("PASSWORD"));
        return destInfo;
    }

    private RestoreStorageInfo getStorageInfo() {
        RestoreStorageInfo flrStorage = new RestoreStorageInfo();
        flrStorage.setStorageType("DORADO");
        flrStorage.setProtocol("NAS");
        flrStorage.setPassword(encryptedPassword(flrStorage.getPassword()));
        return flrStorage;
    }

    private String encryptedPassword(String flrStorage) {
        PlaintextVo plaintextVo = new PlaintextVo();
        plaintextVo.setPlaintext(flrStorage);
        return encryptorRestApi.encrypt(plaintextVo).getCiphertext();
    }

    private Map<String, String> parseResourcePropertiesExtendInfoOfCopy(Copy copy) {
        JSONObject copyResourceProperties = JSONObject.fromObject(copy.getResourceProperties());
        return Optional.ofNullable(copyResourceProperties
                .getJSONObject("extendInfo").toMap(String.class))
                .orElse(Collections.emptyMap());
    }

    private String obtainCurrentProtectedResourceOsType(Copy copy) {
        // 分别尝试从数据库的resource和copy的resource中获得os_type
        log.info("Start query resource id, resource id: {}", copy.getResourceId());
        Optional<ProtectedResource> resourceOpt = resourceService.getResourceById(false, copy.getResourceId());
        if (resourceOpt.isPresent()) {
            ProtectedResource resource = resourceOpt.get();
            protectedResourceMonitorService.invoke("desensitize", resource, ProtectedResourceEvent::getResource);
            return resource.getExtendInfo().get("os_type");
        }
        log.info("Not get copy resource id by query, resource id: {}", copy.getResourceId());
        String copyOsType = parseResourcePropertiesExtendInfoOfCopy(copy).get("os_type");
        log.info("Get copy resource id by parse, resource id: {}, os_type: {}", copy.getResourceId(), copyOsType);
        if (!Strings.isEmpty(copyOsType)) {
            return copyOsType;
        }
        throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "not get current os type");
    }

    private void checkOs(RestoreObject restoreObject) {
        // 原位置恢复直接返回
        if (OLD_LOCATION.equals(restoreObject.getRestoreLocation())) {
            return;
        }

        // 新位置恢复检查,副本虚拟机资源和目标资源操作系统是否一致
        Copy copy = copyRestApi.queryCopyByID(restoreObject.getCopyId());
        String currentOsType = obtainCurrentProtectedResourceOsType(copy);
        ProtectedResource targetResource = obtainProtectedResource(restoreObject.getTarget().getEnvId());
        String targetOsType = targetResource.getExtendInfo().get("os_type");
        log.info("FLR restore osType check. currentOsType:{}, targetOsType:{}, request id:{}",
                currentOsType, targetOsType, restoreObject.getRequestId());

        if (!Objects.equals(currentOsType, targetOsType)) {
            updateJobOsDiffFail(restoreObject, currentOsType, targetOsType);
            log.error("OS type is different");
            throw new LegoCheckedException("file restore fail, reason : OS is different");
        }
    }

    private void updateJobOsDiffFail(RestoreObject restoreObject, String currentOsType, String targetOsType) {
        JobLogBo jobLogBo = new JobLogBo();
        jobLogBo.setJobId(getFlrJobId(restoreObject));
        jobLogBo.setLevel(JobLogLevelEnum.ERROR.getValue());
        jobLogBo.setStartTime(System.currentTimeMillis());
        jobLogBo.setLogInfo("job_file_level_restore_os_diff_label");
        jobLogBo.setLogDetailParam(Arrays.asList(targetOsType, currentOsType));
        jobLogBo.setLogDetail(String.valueOf(CommonErrorCode.FILE_RESTORE_OS_DIFF));
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        updateJobRequest.setJobLogs(Collections.singletonList(jobLogBo));
        updateJobRequest.setStatus(JobStatusEnum.FAIL);
        jobCenterRestApi.updateJob(getFlrJobId(restoreObject), updateJobRequest);
    }

    private ProtectedResource obtainProtectedResource(String resourceId) {
        ProtectedResource resource = resourceService.getResourceById(false, resourceId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "resource not exist"));
        protectedResourceMonitorService.invoke("desensitize", resource, ProtectedResourceEvent::getResource);
        return resource;
    }

    @Override
    public Task download(RestoreObject restoreObject, String snapMetaData) {
        VmFlrRequest vmDownloadRequest = createVmDownloadRequest(restoreObject, snapMetaData);
        kafka.send(TopicConstants.FLR_TOPIC, JSONObject.fromObject(vmDownloadRequest).toString());
        log.info("FLR send success ,requestId : {}", restoreObject.getRequestId());
        Task task = new Task();
        task.setUuid(restoreObject.getRequestId());
        return task;
    }

    private VmFlrRequest createVmDownloadRequest(RestoreObject restoreObject, String snapMetaData) {
        Copy copy = copyRestApi.queryCopyByID(restoreObject.getCopyId());
        VmFlrRequest request = new VmFlrRequest();
        request.setRequestId(restoreObject.getRequestId());
        request.setRecordId(restoreObject.getRecordId());
        request.setResourceSubType(copy.getResourceSubType());
        request.setStorageInfo(getStorageInfo());
        request.setReplaceMode(VmRestoreTypeEnum.DOWNLOAD.name().toLowerCase(Locale.getDefault()));
        request.setSnapId(restoreObject.getCopyId());
        request.setSnapType(copy.getGeneratedBy());
        request.setSnapMetadata(snapMetaData);
        // 下发DEE参数，适配软硬解耦
        Optional<StorageUnitVo> storageUnitVoOptional = storageUnitService.getStorageUnitById(copy.getStorageUnitId());
        if (storageUnitVoOptional.isPresent()) {
            StorageUnitVo storageUnitVo = storageUnitVoOptional.get();
            request.setStorageId(storageUnitVo.getPoolId());
            request.setDeviceId(storageUnitVo.getDeviceId());
        } else {
            // 取默认值
            request.setStorageId(DEFAULT_STORAGE_POOL);
            request.setDeviceId(copy.getDeviceEsn());
        }
        // 文件级恢复，导入文件
        Map<String, String> map = redissonClient.getMap(restoreObject.getRequestId(), StringCodec.INSTANCE);
        List<String> paths = JSONArray.toCollection(JSONArray.fromObject(map.get("paths")), String.class);
        request.setPaths(paths);
        request.setUserId(map.get("user_id"));
        return request;
    }
}
