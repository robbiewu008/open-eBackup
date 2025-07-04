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
package openbackup.db2.protection.access.service.impl;

import com.google.common.collect.Lists;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.FinalizeClearReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.DatabaseRestoreService;
import openbackup.database.base.plugin.service.InstanceProtectionService;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.db2.protection.access.constant.Db2Constants;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.db2.protection.access.service.Db2InstanceService;
import openbackup.db2.protection.access.service.Db2Service;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.StreamUtil;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * db2服务
 *
 */
@Service
@Slf4j
public class Db2ServiceImpl implements Db2Service {
    private final InstanceResourceService instanceResourceService;

    private final InstanceProtectionService instanceProtectionService;

    private final DatabaseRestoreService databaseRestoreService;

    private final CopyRestApi copyRestApi;

    private final ResourceService resourceService;

    private AgentUnifiedService agentUnifiedService;

    private Db2InstanceService db2instanceService;

    public Db2ServiceImpl(InstanceResourceService instanceResourceService,
        InstanceProtectionService instanceProtectionService, DatabaseRestoreService databaseRestoreService,
        CopyRestApi copyRestApi, ResourceService resourceService) {
        this.instanceResourceService = instanceResourceService;
        this.instanceProtectionService = instanceProtectionService;
        this.databaseRestoreService = databaseRestoreService;
        this.copyRestApi = copyRestApi;
        this.resourceService = resourceService;
    }

    @Autowired
    public void setAgentUnifiedService(AgentUnifiedService agentUnifiedService) {
        this.agentUnifiedService = agentUnifiedService;
    }

    @Autowired
    public void setDb2instanceService(Db2InstanceService db2instanceService) {
        this.db2instanceService = db2instanceService;
    }

    @Override
    public List<Endpoint> getAgentsByInstanceResource(ProtectedResource resource) {
        List<TaskEnvironment> nodeList = getEnvNodesByInstanceResource(resource);
        return nodeList.stream()
            .map(node -> new Endpoint(node.getUuid(), node.getEndpoint(), node.getPort()))
            .collect(Collectors.toList());
    }

    @Override
    public List<TaskEnvironment> getEnvNodesByInstanceResource(ProtectedResource resource) {
        if (ResourceSubTypeEnum.DB2_DATABASE.equalsSubType(resource.getSubType())) {
            return extractEnvNodesByInstance(resource.getParentUuid());
        }
        return extractEnvNodesByInstance(resource.getExtendInfoByKey(DatabaseConstants.INSTANCE_UUID_KEY));
    }

    private List<TaskEnvironment> extractEnvNodesByInstance(String instanceId) {
        ProtectedResource instance = instanceResourceService.getResourceById(instanceId);
        if (ResourceSubTypeEnum.DB2_INSTANCE.equalsSubType(instance.getSubType())) {
            return instanceProtectionService.extractEnvNodesBySingleInstance(instance);
        }
        return instanceProtectionService.extractEnvNodesByClusterInstance(instance);
    }

    @Override
    public void checkSupportRestore(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        JSONObject resource = JSONObject.fromObject(copy.getResourceProperties());
        JSONObject resourceExtendInfo = resource.getJSONObject(DatabaseConstants.EXTEND_INFO);
        Map<String, String> targetObjectExtendInfo = task.getTargetObject().getExtendInfo();
        databaseRestoreService.checkDeployOperatingSystem(resourceExtendInfo.getString(DatabaseConstants.DEPLOY_OS_KEY),
            targetObjectExtendInfo.get(DatabaseConstants.DEPLOY_OS_KEY));
        databaseRestoreService.checkResourceSubType(copy.getResourceSubType(), task.getTargetObject().getSubType());
        databaseRestoreService.checkClusterType(resourceExtendInfo.getString(DatabaseConstants.CLUSTER_TYPE),
            targetObjectExtendInfo.get(DatabaseConstants.CLUSTER_TYPE));
        databaseRestoreService.checkVersion(resource.getString(DatabaseConstants.VERSION),
            instanceResourceService.getResourceById(task.getTargetObject().getUuid()).getVersion());
        checkTablespaceRestore(task, copy);
        checkTargetInstanceName(resource.getString(DatabaseConstants.COPY_PARENT_NAME_KEY), task, copy);
        checkTargetClusterNodes(copy, task);
        checkNewLocationRestoreTargetDatabase(copy, task);
        checkHadrTargetDatabase(copy, task, resourceExtendInfo.getString(DatabaseConstants.CLUSTER_TYPE));
    }

    private void checkTablespaceRestore(RestoreTask task, Copy copy) {
        if (!ResourceSubTypeEnum.DB2_TABLESPACE.equalsSubType(copy.getResourceSubType())) {
            return;
        }
        String location = task.getTargetLocation().getLocation();
        if (RestoreLocationEnum.NEW.getLocation().equals(location)) {
            log.error("Db2 tablespace do not support new location restore. task id: {}.", task.getTaskId());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "Db2 tablespace do not support new location restore.");
        }
        Map<String, String> advanceParams = task.getAdvanceParams();
        if (!VerifyUtil.isEmpty(advanceParams) && advanceParams.containsKey(DatabaseConstants.RESTORE_TIME_STAMP_KEY)) {
            log.error("Db2 tablespace do not support point in time restore. task id: {}.", task.getTaskId());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "Db2 tablespace do not support point in time restore.");
        }
    }

    private void checkTargetInstanceName(String sourceInstance, RestoreTask task, Copy copy) {
        if (!ResourceSubTypeEnum.DB2_DATABASE.equalsSubType(copy.getResourceSubType())) {
            return;
        }
        if (!Objects.equals(sourceInstance, task.getTargetObject().getParentName())) {
            log.error("The db2 database restore target instance is inconsistent. task id: {}.", task.getTaskId());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "The db2 database restore target instance is inconsistent.");
        }
    }

    private void checkTargetClusterNodes(Copy copy, RestoreTask task) {
        int sourceNodes = copy.getResourceEnvironmentIp().split(DatabaseConstants.SPLIT_CHAR).length;
        int targetNodes = task.getTargetEnv().getEndpoint().split(DatabaseConstants.SPLIT_CHAR).length;
        if (!Objects.equals(sourceNodes, targetNodes)) {
            log.error("The db2 database restore target cluster nodes is inconsistent. task id: {}.", task.getTaskId());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "The db2 database restore target cluster nodes is inconsistent.");
        }
    }

    private void checkNewLocationRestoreTargetDatabase(Copy copy, RestoreTask task) {
        if (!RestoreLocationEnum.NEW.getLocation().equals(task.getTargetLocation().getLocation())) {
            return;
        }
        if (Objects.equals(copy.getResourceId(), task.getTargetObject().getUuid())) {
            log.error("The db2 database restore target database is consistent. task id: {}.", task.getTaskId());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "The db2 database restore target database is consistent.");
        }
    }

    private void checkHadrTargetDatabase(Copy copy, RestoreTask task, String clusterType) {
        if (!Db2ClusterTypeEnum.HADR.getType().equals(clusterType) || !ResourceSubTypeEnum.DB2_DATABASE.equalsSubType(
            copy.getResourceSubType())) {
            return;
        }
        if (!Objects.equals(copy.getResourceName(), task.getTargetObject().getName())) {
            log.error("The db2 hadr database restore target database name is inconsistent. task id: {}.",
                task.getTaskId());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "The db2 hadr database restore target database name is inconsistent.");
        }
    }

    @Override
    public void updateHadrDatabaseStatus(TaskResource protectObject, String resourceStatus) {
        String clusterType = protectObject.getExtendInfo().get(DatabaseConstants.CLUSTER_TYPE);
        if (Db2ClusterTypeEnum.HADR.getType().equals(clusterType)) {
            ProtectedResource hadrDatabase = new ProtectedResource();
            hadrDatabase.setUuid(protectObject.getUuid());
            hadrDatabase.setExtendInfoByKey(DatabaseConstants.STATUS, resourceStatus);
            resourceService.updateSourceDirectly(Stream.of(hadrDatabase).collect(Collectors.toList()));
        }
    }

    @Override
    public String queryDatabaseSize(ProtectedResource resource) {
        if (ResourceSubTypeEnum.DB2_DATABASE.equalsSubType(resource.getSubType())) {
            return queryDatabaseSizeByAgent(resource.getUuid());
        }
        return queryDatabaseSizeByAgent(resource.getParentUuid());
    }

    private String queryDatabaseSizeByAgent(String databaseId) {
        ListResourceV2Req listResourceV2Req = buildListResourceReq(databaseId);
        PageListResponse<ProtectedResource> pageListResponse;
        try {
            pageListResponse = agentUnifiedService.getDetailPageList(
                ResourceSubTypeEnum.DB2.getType(), listResourceV2Req.getAppEnv().getEndpoint(),
                listResourceV2Req.getAppEnv().getPort(), listResourceV2Req);
        } catch (LegoCheckedException | LegoUncheckedException | FeignException e) {
            log.error("Db2 query database size is failed.", ExceptionUtil.getErrorMessage(e));
            return StringUtils.EMPTY;
        }
        if (VerifyUtil.isEmpty(pageListResponse.getRecords())) {
            log.error("Db2 query database size is empty.");
            return StringUtils.EMPTY;
        }
        return pageListResponse.getRecords().get(IsmNumberConstant.ZERO).getExtendInfoByKey(Db2Constants.DATA_SIZE_KEY);
    }

    private ListResourceV2Req buildListResourceReq(String databaseId) {
        ListResourceV2Req req = new ListResourceV2Req();
        req.setAppEnv(buildAppEnv(databaseId));
        req.setApplications(Lists.newArrayList(buildApp(databaseId)));
        return req;
    }

    private AppEnv buildAppEnv(String databaseId) {
        ProtectedResource database = instanceResourceService.getResourceById(databaseId);
        ProtectedResource instance = instanceResourceService.getResourceById(database.getParentUuid());
        if (ResourceSubTypeEnum.DB2_INSTANCE.equalsSubType(instance.getSubType())) {
            return BeanTools.copy(extractEnvironmentByInstance(instance), AppEnv::new);
        }
        db2instanceService.filterClusterInstance(instance);
        ProtectedResource subInstance = extractSubInstance(instance);
        return BeanTools.copy(extractEnvironmentByInstance(subInstance), AppEnv::new);
    }

    private ProtectedResource extractSubInstance(ProtectedResource clusterInstance) {
        return clusterInstance.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException("Don't have sub instance."));
    }

    private ProtectedEnvironment extractEnvironmentByInstance(ProtectedResource subInstance) {
        return subInstance.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException("This instance environment is empty."));
    }

    private Application buildApp(String databaseId) {
        ProtectedResource database = instanceResourceService.getResourceById(databaseId);
        Application application = new Application();
        application.setName(database.getName());
        application.setUuid(database.getUuid());
        application.setSubType(ResourceSubTypeEnum.DB2.getType());
        return application;
    }

    /**
     * 获取Agent环境信息
     *
     * @param envId 环境uuid
     * @return Agent环境信息
     */
    public ProtectedEnvironment getEnvironmentById(String envId) {
        return resourceService.getResourceById(envId)
                .filter(env -> env instanceof ProtectedEnvironment)
                .map(env -> (ProtectedEnvironment) env)
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.RESOURCE_IS_NOT_EXIST,
                        "Protected environment is not exists!"));
    }

    @Override
    public void deleteLogFromProductEnv(ProtectedResource resource,
                                        HashMap<String, String> extendInfo, String agent_id) {
        log.info("deleteLogCopyFromProductEnv resource is {}", resource.getUuid());
        if (StringUtils.equals(resource.getSubType(), ResourceSubTypeEnum.DB2_DATABASE.getType())) {
            deleteLogClusterInstance(resource, extendInfo, agent_id);
        }
    }

    private void deleteLogClusterInstance(ProtectedResource resource,
                                          HashMap<String, String> extendInfo, String agent_id) {
        // 根据id获取agent资源
        ProtectedEnvironment agentEnvironment = getEnvironmentById(agent_id);
        log.info("finalize deleteLogClusterInstance, agentEnvironment is {}", agentEnvironment);
        if (agentEnvironment == null || StringUtils.equals(agentEnvironment.getLinkStatus(),
                LinkStatusEnum.OFFLINE.getStatus().toString())) {
            log.warn("agent {} not exist or is offline", resource.getUuid());
            return;
        }
        Application application = new Application();
        application.setName(resource.getName());
        application.setUuid(resource.getUuid());
        application.setSubType(ResourceSubTypeEnum.DB2_DATABASE.getType());
        HashMap<String, String> appExtendInfo = new HashMap<>();
        application.setExtendInfo(appExtendInfo);
        AppEnv appEnv = new AppEnv();
        FinalizeClearReq finalizeClearReq = new FinalizeClearReq(appEnv, application, extendInfo);
        agentUnifiedService.finalizeClear(agentEnvironment, resource.getSubType(), finalizeClearReq);
    }
}
