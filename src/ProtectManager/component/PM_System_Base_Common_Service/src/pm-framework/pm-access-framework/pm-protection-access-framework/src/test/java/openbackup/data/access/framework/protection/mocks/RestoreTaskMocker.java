/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.mocks;

import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.access.framework.restore.converter.RestoreTaskConverter;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;

import org.springframework.beans.BeanUtils;

/**
 * 功能描述
 *
 * @description:
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/15
 **/
public class RestoreTaskMocker {
    public static RestoreTask mockRestoreTask(String requestId) {
        final CreateRestoreTaskRequest mockRequest = CreateRestoreTaskRequestMocker.mockSuccessRequest();
        final RestoreTask mockRestoreTask = RestoreTaskConverter.convertToRestoreTask(mockRequest);
        final TaskResource taskResource = TaskResourceMocker.mockFullInfo();
        final ProtectedEnvironment protectedEnvironment = ProtectedResourceMocker.mockTaskEnv();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        BeanUtils.copyProperties(protectedEnvironment, taskEnvironment);
        mockRestoreTask.setTargetObject(taskResource);
        mockRestoreTask.setRequestId(requestId);
        mockRestoreTask.setTargetEnv(taskEnvironment);
        return mockRestoreTask;
    }
}
