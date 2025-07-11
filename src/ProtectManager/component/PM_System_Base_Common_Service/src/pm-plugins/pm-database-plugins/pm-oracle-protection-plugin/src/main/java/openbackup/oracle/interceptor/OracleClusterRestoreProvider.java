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

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.oracle.constants.OracleConstants;
import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.model.Copy;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.stream.Collectors;

/**
 *  oracle集群实例恢复任务下发provider
 *
 */
@Slf4j
@Component
public class OracleClusterRestoreProvider {
    private static final String NODES_INFO = "nodesInfo";

    private static final String MULTI_POST_JOB = "multiPostJob";

    private final OracleBaseService oracleBaseService;

    /**
     * 构造方法
     *
     * @param oracleBaseService oracleBaseService
     */
    public OracleClusterRestoreProvider(OracleBaseService oracleBaseService) {
        this.oracleBaseService = oracleBaseService;
    }

    /**
     * 数据库各自应用信息
     *
     * @param task RestoreTask
     * @param copy 副本信息
     * @return RestoreTask
     */
    public RestoreTask supplyCluster(RestoreTask task, Copy copy) {
        log.info("Oracle RAC restore start, taskId: {}.", task.getTaskId());
        ProtectedEnvironment cluster = oracleBaseService.getEnvironmentById(task.getTargetEnv().getUuid());
        TaskResource targetDatabase = task.getTargetObject();
        oracleBaseService.fillVersionToExtendInfo(cluster.getVersion(), targetDatabase);
        fillDeployType(task);
        fillAgents(task, cluster);
        fillAdvanceParams(task, copy);
        log.info("Oracle RAC restore taskId: {}, targetDatabase: {}.", task.getTaskId(), targetDatabase.getUuid());
        return task;
    }

    private void fillDeployType(RestoreTask task) {
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.AP.getType());
    }

    private void fillAdvanceParams(RestoreTask task, Copy copy) {
        String resourceJsonString = copy.getResourceProperties();
        JSONObject resourceJson = JSONObject.fromObject(resourceJsonString);
        String instances = resourceJson.getJSONObject(DatabaseConstants.EXTEND_INFO)
            .getString(OracleConstants.INSTANCES);
        task.getAdvanceParams().put(NODES_INFO, instances);
        task.getAdvanceParams().put(OracleConstants.TARGET_LOCATION, task.getTargetLocation().getLocation());
        task.getAdvanceParams().put(MULTI_POST_JOB, "true");
    }

    // 从集群dependency里，获取集群下面的所有agent
    private void fillAgents(RestoreTask task, ProtectedEnvironment cluster) {
        List<Endpoint> endpoints = cluster.getDependencies().get(DatabaseConstants.AGENTS).stream()
                .filter(agent -> agent instanceof ProtectedEnvironment)
                .map(agent -> (ProtectedEnvironment) agent)
                .map(agent -> new Endpoint(agent.getUuid(), agent.getEndpoint(), agent.getPort(), agent.getOsType()))
                .collect(Collectors.toList());
        task.setAgents(endpoints);
    }
}
