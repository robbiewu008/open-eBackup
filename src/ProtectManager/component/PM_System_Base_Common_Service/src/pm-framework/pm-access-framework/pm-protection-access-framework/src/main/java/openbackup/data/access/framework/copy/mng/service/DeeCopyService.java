/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.copy.mng.service;

import openbackup.data.access.client.sdk.api.framework.dee.DeeCopiesManagementRestApi;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.copy.mng.constant.CopyResourcePropertiesConstant;
import openbackup.data.access.framework.core.common.constants.CopyConstants;
import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.DeeCopiesRelatedTask;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.enums.RetentionTypeEnum;
import openbackup.system.base.common.enums.TimeUnitEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.util.JobUpdateUtil;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.RetentionBo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.Calendar;
import java.util.Date;
import java.util.Optional;

/**
 * DEE快照相关操作实现
 *
 * @author q00654632
 * @since 2023-08-05
 */
@Service
@Slf4j
public class DeeCopyService {
    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private JobService jobService;

    @Autowired
    private DeeCopiesManagementRestApi deeCopiesManagementRestApi;

    /**
     * 计算保留天数
     *
     * @param retention 保留时间数据类
     * @return 保留天数
     */
    public static long getRetentionDay(RetentionBo retention) {
        long retentionDay = 0L;
        Calendar calendar = Calendar.getInstance();
        Date currentDate = calendar.getTime();
        switch (TimeUnitEnum.getByUnit(retention.getDurationUnit())) {
            case DAYS:
                retentionDay = retention.getRetentionDuration();
                break;
            case WEEKS:
                retentionDay = retention.getRetentionDuration() * 7L;
                break;
            case MONTHS:
                calendar.add(Calendar.MONTH, retention.getRetentionDuration());
                Date futureDate = calendar.getTime();
                retentionDay = (futureDate.getTime() - currentDate.getTime()) / (1000 * 60 * 60 * 24);
                break;
            case YEARS:
                calendar.add(Calendar.YEAR, retention.getRetentionDuration());
                retentionDay = (calendar.getTime().getTime() - currentDate.getTime()) / (1000 * 60 * 60 * 24);
                break;
            default:
        }
        return retentionDay;
    }

    /**
     * 通过dee进行文件系统快照备份
     *
     * @param backupObject backupObject
     * @return BackupTask
     */
    public BackupTask hyperDetectBackup(BackupObject backupObject) {
        RMap<String, String> redisValue = redissonClient.getMap(backupObject.getRequestId(), StringCodec.INSTANCE);
        DeeCopiesRelatedTask deeCopiesRelatedTask = getDeeCopiesRelatedTask(backupObject, redisValue);
        deeCopiesManagementRestApi.createBackupFsSnapshot(deeCopiesRelatedTask);
        log.info("Send backup task to data mover engine successful! requestId is {}",
            deeCopiesRelatedTask.getRequestId());
        jobService.updateJob(backupObject.getRequestId(), JobUpdateUtil.getDeliverReq());
        return new BackupTask();
    }

    private DeeCopiesRelatedTask getDeeCopiesRelatedTask(BackupObject backupObject, RMap<String, String> redisValue) {
        DeeCopiesRelatedTask deeCopiesRelatedTask = new DeeCopiesRelatedTask();
        deeCopiesRelatedTask.setTaskId(backupObject.getTaskId());
        deeCopiesRelatedTask.setRequestId(backupObject.getRequestId());
        ProtectedResource protectedResource = getProtectedResource(backupObject.getProtectedObject().getResourceId());
        deeCopiesRelatedTask.setDeviceId(protectedResource.getRootUuid());
        deeCopiesRelatedTask
            .setVstoreId(protectedResource.getExtendInfoByKey(CopyPropertiesKeyConstant.KEY_PROPERTIES_TENANT_ID));
        deeCopiesRelatedTask.setFilesystemName(protectedResource.getName());
        deeCopiesRelatedTask.setResourceId(protectedResource.getUuid());
        if (ResourceSubTypeEnum.LUN.getType().equals(protectedResource.getSubType())) {
            deeCopiesRelatedTask.setFilesystemId(
                protectedResource.getExtendInfoByKey(CopyResourcePropertiesConstant.PROTECTED_RESOURCE_LUN_ID));
        } else {
            deeCopiesRelatedTask.setFilesystemId(
                protectedResource.getExtendInfoByKey(CopyResourcePropertiesConstant.PROTECTED_RESOURCE_FILESYSTEM_ID));
        }

        deeCopiesRelatedTask.setSnapshotName(redisValue.get(CopyConstants.COPY_NAME));
        deeCopiesRelatedTask
            .setVstoreName(protectedResource.getExtendInfoByKey(CopyPropertiesKeyConstant.KEY_PROPERTIES_TENANT_NAME));
        deeCopiesRelatedTask.setSubType(protectedResource.getSubType());

        String policy = redisValue.get(CopyConstants.POLICY);
        PolicyBo policyBo = JSONObject.fromObject(policy).toBean(PolicyBo.class);
        if (policyBo != null && policyBo.getRetention() != null
            && !policyBo.getRetention().getRetentionType().equals(RetentionTypeEnum.PERMANENT.getType())) {
            RetentionBo retention = policyBo.getRetention();
            long retentionDay = getRetentionDay(retention);
            deeCopiesRelatedTask.setRetentionDay(retentionDay);
        }
        return deeCopiesRelatedTask;
    }

    private ProtectedResource getProtectedResource(String resourceId) {
        Optional<ProtectedResource> resOptional = resourceService.getResourceById(resourceId);
        if (resOptional.isPresent()) {
            return resOptional.get();
        }
        throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected Resource is not exists!");
    }
}
