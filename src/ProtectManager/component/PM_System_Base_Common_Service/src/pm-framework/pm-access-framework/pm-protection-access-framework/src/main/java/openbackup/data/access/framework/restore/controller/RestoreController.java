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
package openbackup.data.access.framework.restore.controller;

import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.access.framework.restore.service.RestoreTaskManager;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.controller.BaseValidatorController;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.user.enums.OperationTypeEnum;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.security.journal.Logging;
import openbackup.system.base.security.permission.Permission;

import lombok.extern.slf4j.Slf4j;

import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import javax.validation.Valid;

/**
 * 统一备份框架中副本恢复的统一Rest接口
 * <p>
 * 框架中接口为V2版本，V1.0.0中已接入的资源依然走V1版本的接口
 * </p>
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/11/30
 **/
@Slf4j
@RestController
@RequestMapping("/v2")
public class RestoreController extends BaseValidatorController {
    private final RestoreTaskManager restoreTaskManager;

    /**
     * 构造函数
     *
     * @param restoreTaskManager 恢复任务管理器bean
     */
    public RestoreController(RestoreTaskManager restoreTaskManager) {
        this.restoreTaskManager = restoreTaskManager;
    }

    /**
     * 创建恢复任务，所有恢复任务执行的统一入口
     *
     * @param restoreTaskRequest 创建恢复任务请求
     * @return 创建恢复任务响应 {@code CreateRestoreTaskResponse}
     */
    @ExterAttack
    @PostMapping("/restore/jobs")
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
            resources = {"copy:$1?.copyId", "resource:$1?.targetObject"},
        resourceSetType = ResourceSetTypeEnum.COPY, operation = OperationTypeEnum.QUERY,
        target = "#restoreTaskRequest.copyId")
    @Logging(
            name = "0x206403350005",
            target = "Restore",
            details = {"$1?.copy?.resourceName", "$1?.copy?.displayTimestamp", "$return?.uuid"})
    public UuidObject createRestoreTask(@Valid @RequestBody CreateRestoreTaskRequest restoreTaskRequest) {
        log.info("create restore job.");
        return new UuidObject(restoreTaskManager.init(restoreTaskRequest));
    }
}