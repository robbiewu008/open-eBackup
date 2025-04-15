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
package openbackup.tdsql.resources.access.service.impl;

import com.huawei.oceanprotect.job.sdk.JobService;

import com.alibaba.fastjson.JSON;
import com.google.common.collect.Lists;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CheckAppReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.FinalizeClearReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.RequestUriUtil;
import openbackup.tdsql.resources.access.constant.TdsqlConstant;
import openbackup.tdsql.resources.access.dto.cluster.BaseNode;
import openbackup.tdsql.resources.access.dto.cluster.OssNode;
import openbackup.tdsql.resources.access.dto.cluster.SchedulerNode;
import openbackup.tdsql.resources.access.dto.cluster.TdsqlCluster;
import openbackup.tdsql.resources.access.dto.instance.DataNode;
import openbackup.tdsql.resources.access.dto.instance.Group;
import openbackup.tdsql.resources.access.dto.instance.GroupInfo;
import openbackup.tdsql.resources.access.dto.instance.TdsqlGroup;
import openbackup.tdsql.resources.access.dto.instance.TdsqlInstance;
import openbackup.tdsql.resources.access.service.TdsqlService;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Service;

import java.lang.reflect.Field;
import java.net.URI;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicReference;

/**
 * 功能描述 TdsqlServiceImpl
 *
 */
@Slf4j
@Service
public class TdsqlServiceImpl implements TdsqlService {
    private final ResourceService resourceService;

    private final AgentUnifiedService agentUnifiedService;

    private final DmeUnifiedRestApi dmeUnifiedRestApi;

    private final JobService jobService;

    /**
     * TdsqlServiceImpl
     *
     * @param resourceService resourceService
     * @param agentUnifiedService agentUnifiedService
     * @param dmeUnifiedRestApi dmeUnifiedRestApi
     * @param jobService jobService
     */
    public TdsqlServiceImpl(ResourceService resourceService, AgentUnifiedService agentUnifiedService,
        DmeUnifiedRestApi dmeUnifiedRestApi, JobService jobService) {
        this.resourceService = resourceService;
        this.agentUnifiedService = agentUnifiedService;
        this.dmeUnifiedRestApi = dmeUnifiedRestApi;
        this.jobService = jobService;
    }

    /**
     * 获取Agent环境信息
     *
     * @param envId 环境uuid
     * @return Agent环境信息
     */
    @Override
    public ProtectedEnvironment getEnvironmentById(String envId) {
        return resourceService.getResourceById(envId)
            .filter(env -> env instanceof ProtectedEnvironment)
            .map(env -> (ProtectedEnvironment) env)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.RESOURCE_IS_NOT_EXIST,
                "Protected environment is not exists!"));
    }

    /**
     * 根据资源uuid，获取应该存在的资源信息
     *
     * @param uuid 资源uuid
     * @return ProtectedResource 资源信息
     */
    @Override
    public ProtectedResource getResourceById(String uuid) {
        return resourceService.getResourceById(uuid)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));
    }

    /**
     * 获取oss节点
     *
     * @param environment 受保护环境
     * @return 管理数据库节点
     */
    @Override
    public List<OssNode> getOssNode(ProtectedEnvironment environment) {
        String tdsqlNodes = environment.getExtendInfo().get(TdsqlConstant.CLUSTER_INFO);
        return JsonUtil.read(tdsqlNodes, TdsqlCluster.class).getOssNodes();
    }

    /**
     * 获取scheduler节点
     *
     * @param environment 受保护环境
     * @return 管理数据库节点
     */
    @Override
    public List<SchedulerNode> getSchedulerNode(ProtectedEnvironment environment) {
        String tdsqlNodes = environment.getExtendInfo().get(TdsqlConstant.CLUSTER_INFO);
        return JsonUtil.read(tdsqlNodes, TdsqlCluster.class).getSchedulerNodes();
    }

    /**
     * 获取单个Oss节点的连通性
     *
     * @param ossNode oss节点
     * @param environment 受保护环境
     * @return 是否连接
     */
    @Override
    public boolean singleOssNodeConnectCheck(OssNode ossNode, ProtectedEnvironment environment) {
        // 通过agentId得到具体要使用的agentEnvironment来获得ip+port
        ProtectedEnvironment agentEnvironment = getEnvironmentById(ossNode.getParentUuid());
        URI uri = RequestUriUtil.getRequestUri(agentEnvironment.getEndpoint(), agentEnvironment.getPort());
        CheckAppReq ossCheckReq = getOssCheckReq(environment, ossNode);
        try {
            AgentBaseDto agentBaseDto = agentUnifiedService.check(TdsqlConstant.TDSQL_CLUSTER, agentEnvironment,
                ossCheckReq);
            return agentBaseDto.isAgentBaseDtoReturnSuccess();
        } catch (LegoCheckedException e) {
            String ip = ossNode.getIp();
            throw convertException(e, ip);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("TDSQL check is failed, ip: {}", ossNode.getIp());
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "TDSQL check failed!");
        }
    }

    /**
     * 获取单个Scheduler节点的连通性
     *
     * @param schedulerNode scheduler节点
     * @param environment 受保护环境
     * @return 是否连接
     */
    @Override
    public boolean singleSchedulerNodeConnectCheck(SchedulerNode schedulerNode, ProtectedEnvironment environment) {
        // 通过agentId得到具体要使用的agentEnvironment来获得ip+port
        ProtectedEnvironment agentEnvironment = getEnvironmentById(schedulerNode.getParentUuid());
        AgentBaseDto agentBaseDto;
        try {
            agentBaseDto = agentUnifiedService.check(TdsqlConstant.TDSQL_CLUSTER, agentEnvironment,
                getSchedulerCheckReq(environment, schedulerNode));
            return agentBaseDto.isAgentBaseDtoReturnSuccess();
        } catch (LegoCheckedException e) {
            String ip = schedulerNode.getIp();
            throw convertException(e, ip);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("TDSQL check failed. ip: {}", schedulerNode.getIp());
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "TDSQL check failed.");
        }
    }

    private CheckAppReq getOssCheckReq(ProtectedEnvironment environment, OssNode ossNode) {
        return getCheckAppReq(environment, ossNode);
    }

    private CheckAppReq getSchedulerCheckReq(ProtectedEnvironment environment, SchedulerNode schedulerNode) {
        return getCheckAppReq(environment, schedulerNode);
    }

    private static CheckAppReq getCheckAppReq(ProtectedEnvironment environment, BaseNode node) {
        node.setLinkStatus(environment.getLinkStatus());
        Application application = new Application();
        application.setSubType(ResourceSubTypeEnum.TDSQL_CLUSTER.getType());
        application.setAuth(environment.getAuth());
        HashMap<String, String> extendInfo = new HashMap<>();
        extendInfo.put(TdsqlConstant.CHECK_TYPE, TdsqlConstant.CHECK_NODE);
        extendInfo.put(TdsqlConstant.SINGLENODE, JSON.toJSONString(node));
        application.setExtendInfo(extendInfo);

        AppEnv appEnv = BeanTools.copy(environment, AppEnv::new);
        return new CheckAppReq(appEnv, application);
    }

    /**
     * 获取单个dataNode节点的连通性
     *
     * @param dataNode instance data节点
     * @param environment 受保护环境
     * @return 是否连接
     */
    @Override
    public boolean singleDataNodeConnectCheck(DataNode dataNode, ProtectedEnvironment environment) {
        // 通过agentId得到具体要使用的agentEnvironment来获得ip+port
        ProtectedEnvironment agentEnvironment = getEnvironmentById(dataNode.getParentUuid());
        AgentBaseDto agentBaseDto;
        try {
            agentBaseDto = agentUnifiedService.check(TdsqlConstant.TDSQL_CLUSTERINSTACE, agentEnvironment,
                getDataNodeCheckReq(environment, dataNode));
            return agentBaseDto.isAgentBaseDtoReturnSuccess();
        } catch (LegoCheckedException e) {
            String ip = dataNode.getIp();
            throw convertException(e, ip);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("TDSQL check fail. ip: {}", dataNode.getIp());
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "TDSQL check fail.");
        }
    }

    @Override
    public boolean checkDataNodeIsMatchAgent(DataNode dataNode, ProtectedEnvironment environment) {
        // 通过agentId得到具体要使用的agentEnvironment来获得ip+port
        ProtectedEnvironment agentEnvironment = getEnvironmentById(dataNode.getParentUuid());
        AgentBaseDto agentBaseDto;
        try {
            agentBaseDto = agentUnifiedService.check(TdsqlConstant.TDSQL_CLUSTER_GROUP, agentEnvironment,
                getGroupDataNodeCheckReq(environment, dataNode));
            return agentBaseDto.isAgentBaseDtoReturnSuccess();
        } catch (LegoCheckedException e) {
            String ip = dataNode.getIp();
            throw convertException(e, ip);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("TDSQL check fail. ip: {}", dataNode.getIp());
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "TDSQL check fail.");
        }
    }

    /**
     * 校验实例信息（下发到数据节点执行OSS查询实例数据和入参进行比较）
     *
     * @param tdsqlGroup Group实例数据
     * @param environment 受保护环境
     * @return 实例信息是否一致
     */
    @Override
    public boolean checkGroupInfo(TdsqlGroup tdsqlGroup, ProtectedEnvironment environment) {
        GroupInfo groupInfo = tdsqlGroup.getGroup();
        // 通过agentId得到具体要使用的agentEnvironment来获得ip+port
        ProtectedEnvironment agentEnvironment = getEnvironmentById(groupInfo.getDataNodes().get(0).getParentUuid());
        AgentBaseDto agentBaseDto;
        try {
            agentBaseDto = agentUnifiedService.check(TdsqlConstant.TDSQL_CLUSTER_GROUP, agentEnvironment,
                getGroupInfoCheckReq(environment, tdsqlGroup));
            return agentBaseDto.isAgentBaseDtoReturnSuccess();
        } catch (LegoCheckedException e) {
            String ip = groupInfo.getDataNodes().get(0).getIp();
            throw convertException(e, ip);
        } catch (LegoUncheckedException | FeignException e) {
            log.error("TDSQL groupInfoCheck fail. ip: {}", groupInfo.getDataNodes().get(0).getIp());
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "TDSQL groupInfoCheck fail.");
        }
    }

    private static LegoCheckedException convertException(LegoCheckedException e, String ip) {
        long errorCode = CommonErrorCode.AGENT_NETWORK_ERROR;
        ActionResult actionResult = new ActionResult();
        if (!VerifyUtil.isEmpty(e.getMessage())) {
            actionResult = JsonUtil.read(e.getMessage(), ActionResult.class);
            errorCode = Long.parseLong(actionResult.getBodyErr());
        }
        if (errorCode == TdsqlConstant.MISS_COMPONENT) {
            String message = actionResult.getMessage();
            log.error("Get error information from plugin is {}", message);
            return new LegoCheckedException(TdsqlConstant.CHECK_CLUSTER_FAILED,
                "The TDSQL environment is missing some components.");
        }
        log.error("TDSQL check is failed. ip: {}", ip);
        if (actionResult.getDetailParams() != null && actionResult.getDetailParams().size() > 0) {
            return new LegoCheckedException(errorCode, actionResult.getDetailParams().toArray(new String[] {}),
                "TDSQL check fail.");
        } else {
            return new LegoCheckedException(errorCode, "TDSQL check fail.");
        }
    }

    private CheckAppReq getGroupDataNodeCheckReq(ProtectedEnvironment environment, DataNode dataNode) {
        Application application = new Application();
        application.setSubType(ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType());
        application.setAuth(environment.getAuth());
        HashMap<String, String> extendInfo = new HashMap<>();
        extendInfo.put(TdsqlConstant.CHECK_TYPE, TdsqlConstant.CHECK_GROUP_DATA_NODE);
        extendInfo.put(TdsqlConstant.DATANODE, JSON.toJSONString(dataNode));
        application.setExtendInfo(extendInfo);
        AppEnv appEnv = BeanTools.copy(environment, AppEnv::new);
        return new CheckAppReq(appEnv, application);
    }

    private CheckAppReq getDataNodeCheckReq(ProtectedEnvironment environment, DataNode dataNode) {
        Application application = new Application();
        application.setSubType(ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType());
        application.setAuth(environment.getAuth());
        HashMap<String, String> extendInfo = new HashMap<>();
        extendInfo.put(TdsqlConstant.CHECK_TYPE, TdsqlConstant.CHECK_NODE);
        HashMap<String, String> singleNode = new HashMap<>();
        singleNode.put(TdsqlConstant.LINKSTATUS, environment.getLinkStatus());

        // 将dataNode对象中的所有属性添加到singleNode
        for (Field field : dataNode.getClass().getDeclaredFields()) {
            field.setAccessible(true);
            try {
                if (field.get(dataNode) != null) {
                    singleNode.put(field.getName(), field.get(dataNode).toString());
                }
            } catch (IllegalAccessException e) {
                log.error("Can not get param");
            }
        }
        extendInfo.put(TdsqlConstant.SINGLENODE, JSON.toJSONString(singleNode));
        application.setExtendInfo(extendInfo);
        AppEnv appEnv = BeanTools.copy(environment, AppEnv::new);
        return new CheckAppReq(appEnv, application);
    }

    private CheckAppReq getGroupInfoCheckReq(ProtectedEnvironment environment, TdsqlGroup tdsqlGroup) {
        // 获取集群信息
        ProtectedEnvironment parentEnvironment = getEnvironmentById(tdsqlGroup.getCluster());
        TdsqlCluster tdsqlCluster = JsonUtil.read(parentEnvironment.getExtendInfo().get(TdsqlConstant.CLUSTER_INFO),
            TdsqlCluster.class);

        Application application = new Application();
        application.setSubType(ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType());
        application.setAuth(parentEnvironment.getAuth());
        HashMap<String, String> extendInfo = new HashMap<>();
        // 跟据关联的集群，获取任意一个oss节点，http://oss业务ip:port/tdsql
        OssNode ossNode = tdsqlCluster.getOssNodes().get(0);
        String requestUrl = "http://" + ossNode.getIp() + ":" + ossNode.getPort() + "/tdsql";
        extendInfo.put(TdsqlConstant.REQUESTURL, requestUrl);
        extendInfo.put(TdsqlConstant.ID, tdsqlGroup.getGroup().getGroupId());
        extendInfo.put(TdsqlConstant.CHECK_TYPE, TdsqlConstant.CHECK_GROUP_INFO);
        extendInfo.put(TdsqlConstant.GROUP, JSON.toJSONString(tdsqlGroup.getGroup()));
        application.setExtendInfo(extendInfo);
        AppEnv appEnv = BeanTools.copy(environment, AppEnv::new);
        return new CheckAppReq(appEnv, application);
    }

    /**
     * 得到环境下面的所有子实例
     *
     * @param parentUuid 环境uuid
     * @param subType subType
     * @return 子资源
     */
    @Override
    public List<ProtectedResource> getChildren(String parentUuid, String subType) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(TdsqlConstant.SUBTYPE, subType);
        conditions.put(TdsqlConstant.PARENT_UUID, parentUuid);
        return resourceService.query(LegoNumberConstant.ZERO, LegoNumberConstant.NUM_EXPORT_NUM, conditions)
            .getRecords();
    }

    /**
     * 更新实例及其数据节点的linkStatus
     *
     * @param updateResource updateResource
     * @param linkStatus status
     */
    @Override
    public void updateInstanceLinkStatus(ProtectedResource updateResource, String linkStatus) {
        log.info("begin update tdsql instance link status, resourceId: {}, linkStatus: {}", updateResource.getUuid(),
            linkStatus);
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(updateResource.getUuid());
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, linkStatus);

        String clusterInstanceInfo = updateResource.getExtendInfoByKey(TdsqlConstant.CLUSTER_INSTANCE_INFO);

        TdsqlInstance tdsqlInstance = JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class);
        for (Group group : tdsqlInstance.getGroups()) {
            for (DataNode dataNode : group.getDataNodes()) {
                dataNode.setLinkStatus(linkStatus);
            }
        }
        resource.getExtendInfo().put(TdsqlConstant.CLUSTER_INSTANCE_INFO, JsonUtil.json(tdsqlInstance));

        resourceService.updateSourceDirectly(Collections.singletonList(resource));
        log.info("End update tdsql instance link status");
    }

    /**
     * updateClusterGroupLinkStatus 更新分布式实例状态
     *
     * @param updateResource updateResource
     * @param linkStatus status
     */
    @Override
    public void updateClusterGroupLinkStatus(ProtectedResource updateResource, String linkStatus) {
        log.info("begin update tdsql cluster group link status, resourceId: {}, linkStatus: {}",
            updateResource.getUuid(), linkStatus);
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(updateResource.getUuid());
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, linkStatus);
        resourceService.updateSourceDirectly(Collections.singletonList(resource));
        log.info("End update tdsql cluster group link status");
    }

    /**
     * 获取计算节点对应的dataNode
     *
     * @param instanceResource 实例资源
     * @return 计算节点
     */
    @Override
    public List<DataNode> getInstanceDataNodes(ProtectedResource instanceResource) {
        LinkedList<DataNode> dataNodeList = new LinkedList<>();
        String clusterInstanceInfo = instanceResource.getExtendInfo().get(TdsqlConstant.CLUSTER_INSTANCE_INFO);
        List<Group> groups = JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class).getGroups();
        groups.forEach(group -> {
            dataNodeList.addAll(group.getDataNodes());
        });
        return dataNodeList;
    }

    /**
     * 获取oss env
     *
     * @param environment 受保护环境
     * @return 计算节点
     */
    public ProtectedEnvironment getOssEnv(ProtectedEnvironment environment) {
        String clusterInfo = environment.getExtendInfo().get(TdsqlConstant.CLUSTER_INFO);
        List<OssNode> ossNodeList = JsonUtil.read(clusterInfo, TdsqlCluster.class).getOssNodes();
        String uuid = ossNodeList.get(0).getParentUuid();
        return getEnvironmentById(uuid);
    }

    /**
     * 获取clusterInfo
     *
     * @param resource 受保护环境
     * @return 计算节点
     */
    @Override
    public ProtectedEnvironment getClusterEnv(ProtectedResource resource) {
        String clusterInstanceInfo = resource.getExtendInfo().get(TdsqlConstant.CLUSTER_INSTANCE_INFO);
        String clusterUuid = JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class).getCluster();
        return getEnvironmentById(clusterUuid);
    }

    @Override
    public PageListResponse<ProtectedResource> getBrowseResult(
        BrowseEnvironmentResourceConditions environmentConditions, ProtectedEnvironment environment) {
        AtomicReference<PageListResponse<ProtectedResource>> result = new AtomicReference<>(new PageListResponse<>());
        ProtectedEnvironment agentEnv = getOssEnv(environment);
        log.info("start to browse TDSQL dataNode,ip is {},resource type is {}, agentPort is {}", agentEnv.getEndpoint(),
            environmentConditions.getResourceType(), agentEnv.getPort());
        result.set(agentUnifiedService.getDetailPageListNoRetry(environmentConditions.getResourceType(),
            agentEnv.getEndpoint(), agentEnv.getPort(), getListResourceReq(environment, environmentConditions), false));
        return result.get();
    }

    @Override
    public PageListResponse<ProtectedResource> getClusterHosts(
        BrowseEnvironmentResourceConditions environmentConditions, ProtectedEnvironment environment, String queryType) {
        AtomicReference<PageListResponse<ProtectedResource>> result = new AtomicReference<>(new PageListResponse<>());
        ProtectedEnvironment agentEnv = getOssEnv(environment);
        log.info("start to browse TDSQL dataNode,ip is {},resource type is {}, agentPort is {}", agentEnv.getEndpoint(),
            environmentConditions.getResourceType(), agentEnv.getPort());
        result.set(agentUnifiedService.getDetailPageListNoRetry(environmentConditions.getResourceType(),
            agentEnv.getEndpoint(), agentEnv.getPort(),
            getClusterHostReq(environment, environmentConditions, queryType), false));

        return result.get();
    }

    private ListResourceV2Req getClusterHostReq(ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions, String queryType) {
        // getResources的接口参数
        OssNode ossNode = getOssNode(environment).get(0);

        Application application = BeanTools.copy(environment, Application::new);
        application.setSubType(ResourceSubTypeEnum.TDSQL_CLUSTER.getType());
        application.setAuth(environment.getAuth());
        HashMap<String, String> extendInfo = new HashMap<>();

        // 跟据关联的集群，获取任意一个oss节点，http://oss业务ip:port/tdsql
        String requestUrl = "http://" + ossNode.getIp() + ":" + ossNode.getPort() + "/tdsql";
        extendInfo.put(TdsqlConstant.REQUESTURL, requestUrl);
        extendInfo.put(TdsqlConstant.QUERY_TYPE_KEY, queryType);
        application.setExtendInfo(extendInfo);
        ListResourceV2Req req = new ListResourceV2Req();
        req.setPageNo(environmentConditions.getPageNo());
        req.setPageSize(environmentConditions.getPageSize());
        req.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        req.setApplications(Lists.newArrayList(application));
        return req;
    }

    private ListResourceV2Req getListResourceReq(ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions) {
        // getResources的接口参数
        OssNode ossNode = getOssNode(environment).get(0);

        Application application = BeanTools.copy(environment, Application::new);
        application.setSubType(ResourceSubTypeEnum.TDSQL_CLUSTER.getType());
        application.setAuth(environment.getAuth());
        HashMap<String, String> extendInfo = new HashMap<>();

        // 跟据关联的集群，获取任意一个oss节点，http://oss业务ip:port/tdsql
        TdsqlInstance param = JsonUtil.read(environmentConditions.getConditions(), TdsqlInstance.class);
        String requestUrl = "http://" + ossNode.getIp() + ":" + ossNode.getPort() + "/tdsql";
        extendInfo.put(TdsqlConstant.REQUESTURL, requestUrl);
        extendInfo.put(TdsqlConstant.INSTANCE_TYPE, param.getType());
        extendInfo.put(TdsqlConstant.ID, param.getId());
        application.setExtendInfo(extendInfo);
        ListResourceV2Req req = new ListResourceV2Req();
        req.setPageNo(environmentConditions.getPageNo());
        req.setPageSize(environmentConditions.getPageSize());
        req.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        req.setApplications(Lists.newArrayList(application));
        return req;
    }

    @Override
    public AppEnvResponse queryClusterInfo(ProtectedEnvironment environment, String agentId) {
        // 通过agentId得到具体要使用的agentEnvironment来获得ip+port
        ProtectedEnvironment agentEnvironment = getEnvironmentById(agentId);
        return agentUnifiedService.getClusterInfoNoRetry(environment, agentEnvironment);
    }

    @Override
    public boolean singleDataNodeHealthCheck(DataNode dataNode, ProtectedEnvironment environment) {
        try {
            return singleDataNodeConnectCheck(BeanTools.copy(dataNode, DataNode::new), environment);
        } catch (LegoCheckedException | LegoUncheckedException | FeignException exception) {
            log.warn("fail to verify the node, IP is {}", dataNode.getIp());
            return false;
        }
    }

    /**
     * 根据Agent主机信息，获取Agent主机的Endpoint
     *
     * @param env Agent环境信息
     * @return Agent对应的Endpoint信息
     */
    @Override
    public Endpoint getAgentEndpoint(ProtectedEnvironment env) {
        if (VerifyUtil.isEmpty(env.getUuid()) || VerifyUtil.isEmpty(env.getEndpoint()) || VerifyUtil.isEmpty(
            env.getPort())) {
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_IS_NOT_EXIST, "TDSQL agent env lack require msg.");
        }
        return new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort());
    }

    /**
     * 检验subtype是否一致
     *
     * @param copy 源
     * @param targetResource 目标资源
     */
    @Override
    public void checkSubType(Copy copy, TaskResource targetResource) {
        log.info("start to check subType.");
        if (!copy.getResourceSubType().equals(targetResource.getSubType())) {
            throw new LegoCheckedException(TdsqlConstant.CHECK_TDSQL_DEPLOYMENT_MODEL_FAILED, "Inconsistent subType!");
        }
    }

    @Override
    public void removeDataRepoWhiteListOfResource(String resourceId) {
        Map<String, Object> paramsMap = new HashMap<>();
        ArrayList<String> actionList = new ArrayList<>();
        actionList.add("removeDataRepoWhiteList");
        Integer count = jobService.getJobCount(Lists.newArrayList(JobTypeEnum.BACKUP.getValue()),
            Lists.newArrayList(JobStatusEnum.SUCCESS.name()), Lists.newArrayList(resourceId));
        if (count > 0) {
            actionList.add("removeRepository");
        }
        paramsMap.put("resourceId", resourceId);
        paramsMap.put("actions", actionList);
        log.info("Removing data repository white list of resource(uuid={}), body: {}.", resourceId, paramsMap);
        dmeUnifiedRestApi.removeRepoWhiteListOfResource(paramsMap);
    }

    @Override
    public void umountDataRepo(TdsqlGroup tdsqlGroup, ProtectedResource resource) {
        GroupInfo groupInfo = tdsqlGroup.getGroup();
        // 通过agentId得到具体要使用的agentEnvironment来获得ip+port
        log.info("umount data repository");
        for (DataNode dataNode : groupInfo.getDataNodes()) {
            ProtectedEnvironment agentEnvironment = getEnvironmentById(dataNode.getParentUuid());
            if (agentEnvironment == null || StringUtils.equals(agentEnvironment.getLinkStatus(),
                LinkStatusEnum.OFFLINE.getStatus().toString())) {
                log.warn("agent {} not exist or is offline", dataNode.getParentUuid());
                continue;
            }
            CheckAppReq reqBody = buildCheckAppReq(tdsqlGroup, resource);
            agentUnifiedService.removeProtectUnmountRepoNoRetry(agentEnvironment.getEndpoint(),
                agentEnvironment.getPort(), TdsqlConstant.TDSQL_CLUSTER_GROUP,
                JSONObject.fromObject(reqBody).toString());
        }
    }

    private CheckAppReq buildCheckAppReq(TdsqlGroup tdsqlGroup, ProtectedResource resource) {
        Application application = new Application();
        application.setSubType(ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType());
        application.setAuth(resource.getAuth());
        HashMap<String, String> extendInfo = new HashMap<>();
        extendInfo.put(TdsqlConstant.CLUSTER_GROUP_INFO, JsonUtil.json(tdsqlGroup));
        application.setExtendInfo(extendInfo);

        AppEnv appEnv = BeanTools.copy(resource, AppEnv::new);
        return new CheckAppReq(appEnv, application);
    }

    @Override
    public void deleteLogFromProductEnv(ProtectedResource resource, HashMap<String, String> extendInfo) {
        log.info("deleteLogCopyFromProductEnv resource {}", resource.getUuid());
        if (StringUtils.equals(resource.getSubType(), ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType())) {
            deleteLogClusterInstance(resource, extendInfo);
        }
    }

    private void deleteLogClusterInstance(ProtectedResource resource, HashMap<String, String> extendInfo) {
        // 非分布式实例清理生产环境日志
        String clusterInstanceInfo = resource.getExtendInfo().get(TdsqlConstant.CLUSTER_INSTANCE_INFO);
        log.info("finalize deleteLogClusterInstance, clusterInstanceInfo is {}", clusterInstanceInfo);
        ProtectedEnvironment environment = BeanTools.copy(resource, ProtectedEnvironment::new);
        List<Group> groups = JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class).getGroups();
        for (Group group : groups) {
            for (DataNode dataNode : group.getDataNodes()) {
                ProtectedEnvironment agentEnvironment = getEnvironmentById(dataNode.getParentUuid());
                if (agentEnvironment == null || StringUtils.equals(agentEnvironment.getLinkStatus(),
                    LinkStatusEnum.OFFLINE.getStatus().toString())) {
                    log.warn("agent {} not exist or is offline", dataNode.getParentUuid());
                    return;
                }
                Application application = new Application();
                application.setSubType(ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType());
                application.setAuth(environment.getAuth());
                HashMap<String, String> appExtendInfo = new HashMap<>();
                appExtendInfo.put(TdsqlConstant.DATANODE, JSON.toJSONString(dataNode));
                application.setExtendInfo(appExtendInfo);
                AppEnv appEnv = BeanTools.copy(environment, AppEnv::new);
                FinalizeClearReq finalizeClearReq = new FinalizeClearReq(appEnv, application, extendInfo);
                agentUnifiedService.finalizeClear(agentEnvironment, resource.getSubType(), finalizeClearReq);
            }
        }
    }
}




