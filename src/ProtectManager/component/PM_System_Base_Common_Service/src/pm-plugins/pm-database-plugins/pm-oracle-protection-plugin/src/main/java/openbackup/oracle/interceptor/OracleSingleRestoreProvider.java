/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * @version [OceanProtect DataBackup 1.3.0]
 * @author c30038333
 * @since 2023-02-14
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
