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
package openbackup.data.access.framework.livemount.provider;

import openbackup.data.access.framework.livemount.common.constants.LiveMountConstants;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountStatus;
import openbackup.data.access.framework.livemount.service.LiveMountService;
import openbackup.data.protection.access.provider.sdk.job.JobCallbackProvider;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.copy.model.CopyStatusUpdateParam;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * 功能描述: LivemountJobCallbackProvider
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-21
 */
@Slf4j
@Component
public class LiveMountJobCallbackProvider implements JobCallbackProvider {
    private static final List<String> LIVE_MOUNT_JOB_LIST = Arrays.asList(
            JobTypeEnum.LIVE_MOUNT.getValue(),
            JobTypeEnum.UNMOUNT.getValue(),
            JobTypeEnum.MIGRATE.getValue());

    private final CopyRestApi copyRestApi;

    private final RedissonClient redissonClient;

    private final LiveMountService liveMountService;

    /**
     * 构造器注入
     *
     *  @param copyRestApi copyRestApi
     * @param redissonClient redissonClient
     * @param liveMountService liveMountService
     */
    public LiveMountJobCallbackProvider(CopyRestApi copyRestApi, RedissonClient redissonClient,
            LiveMountService liveMountService) {
        this.copyRestApi = copyRestApi;
        this.redissonClient = redissonClient;
        this.liveMountService = liveMountService;
    }

    @Override
    public boolean applicable(String jobType) {
        return LIVE_MOUNT_JOB_LIST.contains(jobType);
    }

    @Override
    public void doCallback(JobBo job) {
        log.error("Livemount job (jobId: {}, jobType: {}) over time, copyId: {}.",
                job.getJobId(), job.getType(), job.getCopyId());
        updateLiveMountStatus(job);
        resetCopyStatus(job);
    }

    private void updateLiveMountStatus(JobBo job) {
        JSONObject json = JSONObject.fromObject(job.getMessage());
        JSONObject payload = JSONObject.fromObject(json.get("payload"));
        LiveMountEntity livemount = payload.getBean("live_mount", LiveMountEntity.class);
        if (!VerifyUtil.isEmpty(livemount)) {
            liveMountService.updateLiveMountStatus(livemount, LiveMountStatus.INVALID);
            log.error("Livemount job {} failed, update {} status to invalid.", job.getJobId(), livemount.getId());
        }
    }

    private void resetCopyStatus(JobBo job) {
        if (VerifyUtil.isEmpty(job.getCopyId())) {
            return;
        }
        if (!JobTypeEnum.LIVE_MOUNT.getValue().equals(job.getType())) {
            copyRestApi.updateCopyStatus(job.getCopyId(), buildUpdateParam(CopyStatus.MOUNTED));
            log.error("Livemount reset copy status success, jobId: {}, copyId: {}.", job.getJobId(), job.getCopyId());
            return;
        }
        RMap<Object, Object> map = redissonClient.getMap(job.getJobId(), StringCodec.INSTANCE);
        Object cloneCopyId = map.get(LiveMountConstants.CLONE_COPY_ID);
        String resetCopyId = (!VerifyUtil.isEmpty(cloneCopyId) && cloneCopyId instanceof String)
                ? (String) cloneCopyId : job.getCopyId();
        copyRestApi.updateCopyStatus(resetCopyId, buildUpdateParam(CopyStatus.NORMAL));
        log.error("Livemount reset copy status success, jobId: {}, copyId: {}.", job.getJobId(), resetCopyId);
    }

    private CopyStatusUpdateParam buildUpdateParam(CopyStatus status) {
        CopyStatusUpdateParam updateParam = new CopyStatusUpdateParam();
        updateParam.setStatus(status);
        return updateParam;
    }
}