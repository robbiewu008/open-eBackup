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
package openbackup.data.access.framework.restore.converter;

import openbackup.data.access.framework.protection.common.converters.TaskResourceFilterConverter;
import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;

import org.apache.commons.collections.CollectionUtils;
import org.springframework.beans.BeanUtils;

import java.util.List;
import java.util.stream.Collectors;

/**
 * RestoreTaskConvert
 *
 * @description: RestoreTask对象转换器，用于跟controller层对象转换
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/2
 **/
public class RestoreTaskConverter {
    /**
     * 将CreateRestoreTaskRequest对象转换为RestoreTask对象
     *
     * @param request 创建恢复任务请求对象
     * @return 恢复任务下发对象 {@code RestoreTask}
     */
    public static RestoreTask convertToRestoreTask(CreateRestoreTaskRequest request) {
        RestoreTask restoreTask = new RestoreTask();
        BeanUtils.copyProperties(
                request,
                restoreTask,
                "restoreType",
                "filters",
                "targetEnv",
                "targetObject",
                "subObjects",
                "agents",
                "advanceParams");
        restoreTask.setTargetEnv(new TaskEnvironment());
        restoreTask.setRestoreType(request.getRestoreType().getType());
        if (!CollectionUtils.isEmpty(request.getFilters())) {
            restoreTask.setFilters(
                    request.getFilters().stream()
                            .map(TaskResourceFilterConverter::covertToTaskResourceFilter)
                            .collect(Collectors.toList()));
        }
        if (!CollectionUtils.isEmpty(request.getSubObjects())) {
            final List<TaskResource> subObjects =
                    request.getSubObjects().stream()
                            .map(
                                    item -> {
                                        TaskResource taskResource = new TaskResource();
                                        taskResource.setName(item);
                                        return taskResource;
                                    })
                            .collect(Collectors.toList());
            restoreTask.setSubObjects(subObjects);
        }
        return restoreTask;
    }
}
