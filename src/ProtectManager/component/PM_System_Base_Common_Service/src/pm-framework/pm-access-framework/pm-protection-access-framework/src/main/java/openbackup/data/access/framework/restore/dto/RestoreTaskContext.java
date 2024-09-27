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
package openbackup.data.access.framework.restore.dto;

import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.system.base.sdk.copy.model.Copy;

/**
 * RestoreTaskContext
 *
 * @description: 恢复任务管理的上下文信息，存储中间过程的数据，减少函数中参数数量
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/2
 **/
public class RestoreTaskContext {
    /**
     * 本次任务的请求id
     */
    private String requestId;

    /**
     * 副本信息
     */
    private Copy copy;

    /**
     * 恢复任务请求
     */
    private CreateRestoreTaskRequest taskRequest;

    /**
     * 恢复任务下发信息
     */
    private RestoreTask restoreTask;

    /**
     * 恢复任务拦截器
     */
    private RestoreInterceptorProvider interceptorProvider;

    public Copy getCopy() {
        return copy;
    }

    public void setCopy(Copy copy) {
        this.copy = copy;
    }

    public CreateRestoreTaskRequest getTaskRequest() {
        return taskRequest;
    }

    public void setTaskRequest(CreateRestoreTaskRequest taskRequest) {
        this.taskRequest = taskRequest;
    }

    public RestoreTask getRestoreTask() {
        return restoreTask;
    }

    public void setRestoreTask(RestoreTask restoreTask) {
        this.restoreTask = restoreTask;
    }

    public RestoreInterceptorProvider getInterceptorProvider() {
        return interceptorProvider;
    }

    public void setInterceptorProvider(RestoreInterceptorProvider interceptorProvider) {
        this.interceptorProvider = interceptorProvider;
    }

    public String getRequestId() {
        return requestId;
    }

    public void setRequestId(String requestId) {
        this.requestId = requestId;
    }
}
