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
package openbackup.data.access.framework.copy.mng.handler.v1;

import com.huawei.oceanprotect.system.base.label.service.LabelService;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.CopyIndexStatus;
import openbackup.data.access.framework.core.common.enums.DmeJobStatusEnum;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.handler.TaskCompleteHandler;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.index.v1.DeleteIndexProvider;
import openbackup.data.protection.access.provider.sdk.job.ExtendsInfo;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.JobStopParam;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.apache.commons.lang3.StringUtils;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collections;
import java.util.Map;

/**
 * 副本删除任务完成消息监听器
 *
 */
@Slf4j
@Component
public class CopyDeleteCompleteHandler implements TaskCompleteHandler {
    private static final String COPY_DAMAGED = "copy_damaged";

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private NotifyManager notifyManager;

    @Autowired
    private ProviderManager providerManager;

    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private JobCenterRestApi jobCenterRestApi;

    @Autowired
    private UserQuotaManager userQuotaManager;

    @Autowired
    private LabelService labelService;

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(String object) {
        return Arrays.asList(JobTypeEnum.COPY_DELETE.getValue(), JobTypeEnum.COPY_EXPIRE.getValue()).contains(object);
    }

    /**
     * task complete handler
     *
     * @param taskCompleteMessage task complete message
     */
    @ExterAttack
    @Override
    public void onTaskCompleteSuccess(TaskCompleteMessageBo taskCompleteMessage) {
        taskCommonPostProcess(taskCompleteMessage, true);
    }

    private void taskCommonPostProcess(TaskCompleteMessageBo taskCompleteMessage, boolean isSuccess) {
        updateTaskCanStop(taskCompleteMessage);
        String requestId = taskCompleteMessage.getJobRequestId();
        RMap<String, String> context = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        Map extInfo = taskCompleteMessage.getExtendsInfo();
        ExtendsInfo extendsInfo = null;
        if (extInfo != null) {
            extendsInfo = JSONObject.toBean(JSONObject.fromObject(extInfo), ExtendsInfo.class);
        }
        int status = taskCompleteMessage.getJobStatus();
        boolean isCopyDamaged = DmeJobStatusEnum.SUCCESS.equals(DmeJobStatusEnum.fromStatus(status)) || (
            extendsInfo != null && extendsInfo.isCopyDamaged()); // false 副本未删除成功，可用 true 表示副本不可用
        context.put(COPY_DAMAGED, String.valueOf(isCopyDamaged));

        String copyId = context.get(ContextConstants.COPY_ID);
        try {
            Copy copy = copyRestApi.queryCopyByID(copyId, false);
            if (copy != null && CopyIndexStatus.INDEXED.getIndexStaus().equals(copy.getIndexed())) {
                providerManager.findProvider(DeleteIndexProvider.class, copy.getResourceSubType())
                    .deleteIndex(requestId, copyId);
            }
            if (isSuccess) {
                // 副本删除成功，减少用户已使用配额
                userQuotaManager.decreaseUsedQuota(requestId, copy);
                // 删除标签关联关系 (一样的逻辑抽象出来比较好)
                labelService.deleteByResourceObjectIdsAndLabelIds(Collections.singletonList(copyId), StringUtils.EMPTY);
            }
        } catch (LegoCheckedException exception) {
            log.error("Delete copy index failed.", ExceptionUtil.getErrorMessage(exception));
        }

        // 发送副本删除topic
        JSONObject copyDeleteReq = new JSONObject();
        copyDeleteReq.put(ContextConstants.REQUEST_ID, requestId);
        notifyManager.send(TopicConstants.COPY_DELETE_JOB_MONITOR_FINISHED, copyDeleteReq.toString());
    }

    /**
     * task complete handler
     *
     * @param taskCompleteMessage task complete message
     */
    @Override
    public void onTaskCompleteFailed(TaskCompleteMessageBo taskCompleteMessage) {
        // 失败的逻辑在副本模块已做处理，本次仅先做接口适配
        taskCommonPostProcess(taskCompleteMessage, false);
    }

    private void updateTaskCanStop(TaskCompleteMessageBo taskCompleteMessage) {
        JobStopParam jobStopParam = new JobStopParam();
        jobStopParam.setBackupEngineCancelable(true);
        jobStopParam.setEnforceStop(true);
        UpdateJobRequest updateJobRequest = new UpdateJobRequest();
        updateJobRequest.setEnableStop(true);
        jobCenterRestApi.updateJob(taskCompleteMessage.getJobRequestId(), updateJobRequest, jobStopParam);
    }
}
