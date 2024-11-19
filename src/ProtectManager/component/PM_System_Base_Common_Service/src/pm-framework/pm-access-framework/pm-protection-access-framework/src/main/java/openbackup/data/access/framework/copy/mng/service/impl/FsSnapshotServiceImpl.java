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
package openbackup.data.access.framework.copy.mng.service.impl;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.job.sdk.JobService;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.dee.DeeFsSnapshotRestApi;
import openbackup.data.access.client.sdk.api.framework.dee.model.CreateFsSnapshotRequest;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.copy.mng.constant.CopyResourcePropertiesConstant;
import openbackup.data.access.framework.copy.mng.service.FsSnapshotService;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.CopyConstants;
import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.job.util.JobUpdateUtil;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.SlaBo;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

/**
 * 安全一体机创建快照实现
 * 支持Dorado/OceanProtect/Pacific设备
 *
 */
@Slf4j
@Service
public class FsSnapshotServiceImpl implements FsSnapshotService {
    @Autowired
    private MemberClusterService memberClusterService;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private DeeFsSnapshotRestApi deeFsSnapshotRestApi;

    @Autowired
    private JobService jobService;

    /**
     * 创建文件系统快照
     *
     * @param backupObject 备份信息
     * @param protectedResource 保护对象信息
     * @return 备份参数对象
     */
    @Override
    public BackupTask oceanCyberDetectBackup(BackupObject backupObject, ProtectedResource protectedResource) {
        CreateFsSnapshotRequest createFsSnapshotRequest = buildCreateFsSnapshotRequest(backupObject, protectedResource);
        deeFsSnapshotRestApi.createFsSnapshot(createFsSnapshotRequest);
        log.info("Send filesystem backup task to dee successful! requestId is {}",
            createFsSnapshotRequest.getRequestId());
        jobService.updateJob(backupObject.getRequestId(), JobUpdateUtil.getDeliverReq());
        return new BackupTask();
    }

    // 获取通过DEE创建文件系统快照请求参数
    private CreateFsSnapshotRequest buildCreateFsSnapshotRequest(BackupObject backupObject,
        ProtectedResource protectedResource) {
        RMap<String, String> redisValue = redissonClient.getMap(backupObject.getRequestId(), StringCodec.INSTANCE);
        CreateFsSnapshotRequest createFsSnapshotRequest = new CreateFsSnapshotRequest();
        createFsSnapshotRequest.setTaskId(backupObject.getTaskId());
        createFsSnapshotRequest.setRequestId(backupObject.getRequestId());
        createFsSnapshotRequest.setSnapshotName(redisValue.get(CopyConstants.COPY_NAME));
        createFsSnapshotRequest.setDeviceId(protectedResource.getRootUuid());
        createFsSnapshotRequest.setFilesystemName(protectedResource.getName());
        createFsSnapshotRequest.setFilesystemId(protectedResource.getExtendInfoByKey(
            CopyResourcePropertiesConstant.PROTECTED_RESOURCE_FILESYSTEM_ID));
        createFsSnapshotRequest.setResourceId(protectedResource.getUuid());
        createFsSnapshotRequest.setVstoreName(protectedResource.getExtendInfoByKey(
            CopyPropertiesKeyConstant.KEY_PROPERTIES_TENANT_NAME));
        String policy = redisValue.get(CopyConstants.POLICY);
        PolicyBo policyBo = JSONObject.fromObject(policy).toBean(PolicyBo.class);
        if (policyBo != null && policyBo.getRetention() != null) {
            createFsSnapshotRequest.setRetentionDuration(policyBo.getRetention().getRetentionDuration());
            createFsSnapshotRequest.setRetentionType(policyBo.getRetention().getRetentionType());
            createFsSnapshotRequest.setDurationUnit(policyBo.getRetention().getDurationUnit());
        }

        String slaString = redisValue.get(CopyConstants.SLA);
        if (!VerifyUtil.isEmpty(slaString)) {
            createFsSnapshotRequest.setSlaString(slaString);
            SlaBo slaBo = JSONObject.toBean(slaString, SlaBo.class);
            createFsSnapshotRequest.setSlaName(slaBo.getName());
        }

        redisValue.put(ContextConstants.COPY_FORMAT, String.valueOf(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat()));
        createFsSnapshotRequest.setCopyId(backupObject.getRequestId()); // 副本ID和requestID保持一致
        String chainId = redisValue.get(CopyPropertiesKeyConstant.KEY_BACKUP_CHAIN_ID);
        createFsSnapshotRequest.setChainId(chainId);
        createFsSnapshotRequest.setDeviceEsn(memberClusterService.getCurrentClusterEsn());
        String vstoreId = protectedResource.getExtendInfoByKey(CopyPropertiesKeyConstant.KEY_PROPERTIES_TENANT_ID);
        createFsSnapshotRequest.setVstoreId(vstoreId);
        return createFsSnapshotRequest;
    }
}
