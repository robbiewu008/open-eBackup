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
package openbackup.tdsql.resources.access.interceptor;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.common.collect.Maps;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tdsql.resources.access.provider.TdsqlAgentProvider;
import openbackup.tdsql.resources.access.service.TdsqlService;

import org.apache.commons.collections4.CollectionUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 功能描述 恢复
 *
 */
@Slf4j
@Component
public class TdsqlClusterGroupRestoreInterceptor extends AbstractDbRestoreInterceptorProvider {
    private final TdsqlService tdsqlService;

    private final CopyRestApi copyRestApi;

    private final TdsqlAgentProvider tdsqlAgentProvider;

    private final List<String> copyGeneratedByEnumList = Arrays.asList(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value(),
        CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value());

    /**
     * 构造器
     *
     * @param tdsqlService tdsqlService
     * @param copyRestApi copyRestApi
     * @param tdsqlAgentProvider tdsqlAgentProvider
     */
    public TdsqlClusterGroupRestoreInterceptor(TdsqlService tdsqlService, CopyRestApi copyRestApi,
        TdsqlAgentProvider tdsqlAgentProvider) {
        this.tdsqlService = tdsqlService;
        this.copyRestApi = copyRestApi;
        this.tdsqlAgentProvider = tdsqlAgentProvider;
    }

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType().equals(object);
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        // 添加恢复的agents到任务中
        log.info("begin TDSQL group restoreTask intercept");
        List<Endpoint> endpointList = buildRestoreAgents(task);
        task.setAgents(endpointList);

        ArrayList<TaskEnvironment> taskEnvironments = new ArrayList<>();
        // 设置nodes信息
        endpointList.stream().map(Endpoint::getId).forEach(it -> {
            taskEnvironments.add(getNode(it));
        });

        // 设置部署类型
        task.getTargetEnv()
            .getExtendInfo()
            .put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.DISTRIBUTED.getType());
        task.getTargetEnv().setNodes(taskEnvironments);

        // 设置速度统计方式为UBC
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);

        // 后置任务所有节点都执行
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElse(Maps.newHashMap());
        advanceParams.put(DatabaseConstants.MULTI_POST_JOB, Boolean.TRUE.toString());
        task.setAdvanceParams(advanceParams);

        // 设置恢复副本类型
        buildRestoreMode(task);
        log.info("end TDSQL group restoreTask intercept");
        return task;
    }

    private TaskEnvironment getNode(String agentUuid) {
        ProtectedEnvironment agent = tdsqlService.getEnvironmentById(agentUuid);
        TaskEnvironment taskEnv = new TaskEnvironment();
        taskEnv.setName(agent.getName());
        taskEnv.setUuid(agent.getUuid());
        taskEnv.setType(agent.getType());
        taskEnv.setSubType(agent.getSubType());
        taskEnv.setEndpoint(agent.getEndpoint());
        taskEnv.setExtendInfo(agent.getExtendInfo());
        return taskEnv;
    }

    /**
     * 设置恢复模式
     *
     * @param task 恢复任务
     */
    public void buildRestoreMode(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        if (copyGeneratedByEnumList.contains(copy.getGeneratedBy())) {
            task.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
        } else {
            task.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }
        log.info("build TDSQL group copy restore mode. copy id: {}, mode: {}", task.getCopyId(), task.getRestoreMode());
    }


    private List<Endpoint> buildRestoreAgents(RestoreTask task) {
        // 从extend_info里面获取恢复的数据节点信息
        Map<String, String> advanceParams = task.getAdvanceParams();
        log.info("restoreHosts is {}", advanceParams.get("restoreHosts"));
        List<Map<String, String>> restoreHosts;
        try {
            ObjectMapper objectMapper = new ObjectMapper();
            restoreHosts = objectMapper.readValue(advanceParams.get("restoreHosts"),
                new TypeReference<List<Map<String, String>>>() {});
        } catch (JsonProcessingException e) {
            log.error("parse restore hosts info error");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "restore hosts info invalid!");
        }

        if (CollectionUtils.isEmpty(restoreHosts)) {
            log.error("restore hosts info is empty");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "restore hosts info empty!");
        }
        List<String> agentUuids = restoreHosts.stream().map(item -> item.get("parentUuid")).collect(
            Collectors.toList());
        List<Endpoint> endpointList = new ArrayList<>();
        for (String agentUuid : agentUuids) {
            ProtectedEnvironment agentEnv = tdsqlService.getEnvironmentById(agentUuid);
            Endpoint endpoint = new Endpoint();
            endpoint.setId(agentEnv.getUuid());
            endpoint.setIp(agentEnv.getEndpoint());
            endpoint.setPort(agentEnv.getPort());
            endpointList.add(endpoint);
        }
        return endpointList;
    }
}
