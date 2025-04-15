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
package openbackup.openstack.adapter.service;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.access.framework.restore.service.RestoreTaskManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.openstack.adapter.dto.OpenStackRestoreJobDto;
import openbackup.openstack.adapter.generator.RestoreGenerator;

import org.springframework.stereotype.Component;

/**
 * OpenStack恢复相关操作管理器
 *
 */
@Slf4j
@Component
public class OpenStackRestoreManager {
    private final RestoreTaskManager restoreTaskManager;

    public OpenStackRestoreManager(RestoreTaskManager restoreTaskManager) {
        this.restoreTaskManager = restoreTaskManager;
    }

    /**
     * 创建恢复任务
     *
     * @param restoreJob {@link OpenStackRestoreJobDto} 北向接口创建恢复任务请求体
     * @param resource 受保护资源
     * @return 恢复任务id
     */
    public String createRestoreTask(OpenStackRestoreJobDto restoreJob, ProtectedResource resource) {
        CreateRestoreTaskRequest request = RestoreGenerator.generateCreateRestoreReq(restoreJob, resource);
        String jobId = restoreTaskManager.init(request);
        log.info("Openstack create restore job: {} of copy: {} success.", jobId, restoreJob.getCopyId());
        return jobId;
    }
}
