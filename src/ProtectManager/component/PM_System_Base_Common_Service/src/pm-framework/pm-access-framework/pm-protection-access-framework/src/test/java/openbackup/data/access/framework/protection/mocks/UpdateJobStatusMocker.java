/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.data.access.framework.protection.mocks;

import openbackup.data.access.framework.protection.controller.req.UpdateJobLogRequest;
import openbackup.data.access.framework.protection.controller.req.UpdateJobStatusRequest;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

/**
 * 更新JobStatus信息Mock类，用于mock更新job状态各种场景的对象
 *
 * @author w00616953
 * @since 2021-12-10
 */
public class UpdateJobStatusMocker {

    /**
     * 构造DME上报任务状态的请求对象
     *
     * @return dme上报任务状态请求对象
     */
    public static UpdateJobStatusRequest mockUpdateJobStatusRequest() {
        UpdateJobStatusRequest jobStatusRequest = new UpdateJobStatusRequest();
        Map<String, String> extendField = new HashMap<>();
        extendField.put("key", "value");

        jobStatusRequest.setJobRequestId(UUID.randomUUID().toString());
        jobStatusRequest.setStatus(13);
        jobStatusRequest.setProgress(1);
        jobStatusRequest.setSpeed(123L);
        jobStatusRequest.setAdditionalStatus("additional status");
        jobStatusRequest.setExtendField(extendField);
        jobStatusRequest.setTaskId(UUID.randomUUID().toString());
        return jobStatusRequest;
    }

    /**
     * 构造DME上报任务日志的请求对象
     *
     * @return dme上报任务状态日志对象
     */
    public static UpdateJobLogRequest mockUpdateJobLogRequest() {
        UpdateJobLogRequest jobLogRequest = new UpdateJobLogRequest();

        jobLogRequest.setLogTimestamp(1634006791713L);
        jobLogRequest.setLogLevel(1);
        jobLogRequest.setLogInfo("dme_vmware_create_vm_snapshot_label");
        jobLogRequest.setLogInfoParam(Arrays.asList("Backup_snapshot_2021-Oct-12 10:46:31.700294"));

        return jobLogRequest;
    }
}
