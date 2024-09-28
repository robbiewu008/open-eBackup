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
package openbackup.oracle.interceptor;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.oracle.constants.OracleConstants;
import openbackup.oracle.service.OracleBaseService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collections;

/**
 * oracle数据库恢复任务下发provider
 *
 */
@Slf4j
@Component
public class OracleSingleRestoreProvider {
    OracleBaseService oracleBaseService;

    @Autowired
    public void setOracleBaseService(OracleBaseService oracleBaseService) {
        this.oracleBaseService = oracleBaseService;
    }

    /**
     * 数据库各自应用信息
     *
     * @param task RestoreTask
     * @return RestoreTask
     */
    public RestoreTask supplySingle(RestoreTask task) {
        log.info("Oracle single restore start, taskId: {}.", task.getTaskId());
        fillDeployType(task);
        fillAgents(task);
        fillAdvanceParams(task);
        log.info("Oracle single restore finished, taskId: {}.", task.getTaskId());
        return task;
    }

    private void fillDeployType(RestoreTask task) {
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
    }

    private void fillAdvanceParams(RestoreTask task) {
        task.getAdvanceParams().put(OracleConstants.TARGET_LOCATION, task.getTargetLocation().getLocation());
    }

    // 根据Agents，设置node，DME会用于任务拆分
    private void fillAgents(RestoreTask task) {
        TaskEnvironment targetEnv = task.getTargetEnv();
        ProtectedEnvironment environment = oracleBaseService.getEnvironmentById(targetEnv.getUuid());
        String osType = oracleBaseService.getOsType(environment);
        task.setAgents(Collections.singletonList(new Endpoint(environment.getUuid(), environment.getEndpoint(),
                environment.getPort(), osType)));
    }
}
