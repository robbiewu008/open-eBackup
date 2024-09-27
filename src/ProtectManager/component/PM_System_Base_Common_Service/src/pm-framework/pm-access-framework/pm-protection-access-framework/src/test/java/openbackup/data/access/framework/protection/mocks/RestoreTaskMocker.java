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
