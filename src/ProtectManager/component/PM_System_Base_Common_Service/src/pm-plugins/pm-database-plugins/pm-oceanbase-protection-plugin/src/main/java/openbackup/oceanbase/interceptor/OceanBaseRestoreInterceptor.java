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
package openbackup.oceanbase.interceptor;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.oceanbase.common.constants.OBConstants;
import openbackup.oceanbase.provider.OceanBaseAgentProvider;
import openbackup.oceanbase.service.OceanBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.BeanTools;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.google.common.collect.Maps;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.regex.Pattern;

/**
 * OceanBase恢复拦截器
 *
 * @author x30028756
 * @version [OceanProtect X8000 1.5.0]
 * @since 2023-07-13
 */
@Slf4j
@Component
public class OceanBaseRestoreInterceptor extends AbstractDbRestoreInterceptorProvider {
    private final ResourceService resourceService;

    private final CopyRestApi copyRestApi;

    private final OceanBaseAgentProvider oceanBaseAgentProvider;

    private final OceanBaseService oceanBaseService;

    private final DeployTypeService deployTypeService;

    /**
     * 构造方法
     *
     * @param resourceService resourceService
     * @param copyRestApi copyRestApi
     * @param oceanBaseAgentProvider oceanBaseAgentProvider
     * @param oceanBaseService oceanBaseService
     * @param deployTypeService deployTypeService
     */
    public OceanBaseRestoreInterceptor(ResourceService resourceService, CopyRestApi copyRestApi,
        OceanBaseAgentProvider oceanBaseAgentProvider, OceanBaseService oceanBaseService,
        DeployTypeService deployTypeService) {
        this.resourceService = resourceService;
        this.copyRestApi = copyRestApi;
        this.oceanBaseAgentProvider = oceanBaseAgentProvider;
        this.oceanBaseService = oceanBaseService;
        this.deployTypeService = deployTypeService;
    }

    @Override
    public boolean applicable(String subType) {
        return StringUtils.equals(subType, ResourceSubTypeEnum.OCEAN_BASE_TENANT.getType()) || StringUtils.equals(
            subType, ResourceSubTypeEnum.OCEAN_BASE_CLUSTER.getType());
    }

    @Override
    public void restoreTaskCreationPreCheck(RestoreTask task) {
        if (deployTypeService.isE1000()) {
            oceanBaseService.checkSupportNFSV41Dependent(task.getRepositories());
        } else {
            oceanBaseService.checkSupportNFSV41();
        }
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElse(Maps.newHashMap());
        String tenantInfosStr = advanceParams.get("tenantInfos");
        if (StringUtils.isEmpty(tenantInfosStr)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "param illegal.");
        }
        JSONArray tenantArray = JSONArray.parseArray(tenantInfosStr);
        if (tenantArray.size() == 0) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "param illegal.");
        } else {
            for (int i = 0; i < tenantArray.size(); i++) {
                // 遍历 jsonarray 数组，把每一个对象转成 json 对象
                JSONObject tenant = tenantArray.getJSONObject(i);
                // 得到 每个对象中的属性值
                tenantNameRuleCheck(tenant, "originalName");
                tenantNameRuleCheck(tenant, "targetName");
            }
        }
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElse(Maps.newHashMap());

        advanceParams.put(DatabaseConstants.MULTI_POST_JOB, Boolean.FALSE.toString());
        task.setAdvanceParams(advanceParams);
        // 设置速度统计方式为UBC
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);
        ProtectedResource targetResource = resourceService.getResourceById(false, task.getTargetEnv().getUuid())
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "protectedResource is not exist"));
        // 设置agent
        buildRestoreAgents(task, targetResource);
        // 填充节点信息
        supplyNodes(task, targetResource);
        // 填充目标环境扩展字段
        supplyTargetEnvExtendInfo(task);
        setRestoreParam(task);
        return task;
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        return Collections.singletonList(new LockResourceBo(task.getTargetObject().getUuid(), LockType.WRITE));
    }

    private void buildRestoreAgents(RestoreTask task, ProtectedResource targetResource) {
        log.info("OceanBaseRestore build restore agents of task: {}", task.getTaskId());
        ProtectedResource resource = BeanTools.copy(targetResource, ProtectedResource::new);
        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.RESTORE.getValue())
            .build();

        List<Endpoint> endpointList = oceanBaseAgentProvider.getSelectedAgents(agentSelectParam);

        Map<String, String> envExtendInfo = task.getTargetEnv().getExtendInfo();
        envExtendInfo.put(OBConstants.EXEC_NODE_ID, endpointList.get(0).getId());
        task.getTargetEnv().setExtendInfo(envExtendInfo);
        task.setAgents(endpointList);
    }

    private void supplyNodes(RestoreTask task, ProtectedResource targetResource) {
        log.info("OceanBaseRestore supply nodes of task: {}", task.getTaskId());
        // 获取集群下节点
        List<ProtectedResource> serverAgents = targetResource.getDependencies().get(OBConstants.SERVER_AGENTS);
        List<ProtectedResource> clientAgents = targetResource.getDependencies().get(OBConstants.CLIENT_AGENTS);
        Collection<ProtectedResource> nodes = CollectionUtils.union(serverAgents, clientAgents);
        List<TaskEnvironment> nodeList = new ArrayList<>();
        for (ProtectedResource resource : nodes) {
            if (agentsContains(task, resource.getUuid())) {
                TaskEnvironment read = JsonUtil.read(JsonUtil.json(resource), TaskEnvironment.class);
                nodeList.add(read);
            }
        }
        task.getTargetEnv().setNodes(nodeList);
    }

    private boolean agentsContains(RestoreTask task, String uuid) {
        return task.getAgents().stream().anyMatch(item -> Objects.equals(item.getId(), uuid));
    }

    private void supplyTargetEnvExtendInfo(RestoreTask task) {
        Map<String, String> envExtendInfo = task.getTargetEnv().getExtendInfo();
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.DISTRIBUTED.getType());
        task.getTargetEnv().setExtendInfo(envExtendInfo);
    }

    private void setRestoreParam(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        String generatedBy = copy.getGeneratedBy();
        // 设置恢复模式
        task.setRestoreMode(
            StringUtils.equals(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value(), generatedBy) || StringUtils.equals(
                CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value(), generatedBy)
                ? RestoreModeEnum.DOWNLOAD_RESTORE.getMode()
                : RestoreModeEnum.LOCAL_RESTORE.getMode());
        log.info("OceanBase restore task: {}, copy id: {}, mode: {}", task.getTaskId(), task.getCopyId(),
            task.getRestoreMode());
    }

    /**
     * 1-128位，只能以字母或者_开头，由数字，字母，_组成
     *
     * @param tenant 租户信息
     * @param name 租户名
     */
    private static void tenantNameRuleCheck(JSONObject tenant, String name) {
        Object nameObject = tenant.get(name);
        if (Objects.isNull(nameObject)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "param illegal.");
        }
        String tenantName = nameObject.toString();
        String pattern = "^[A-Za-z_][0-9A-Za-z_]{0,127}";
        boolean isMatch = Pattern.matches(pattern, tenantName);
        if (!isMatch) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "param illegal.");
        }
    }
}