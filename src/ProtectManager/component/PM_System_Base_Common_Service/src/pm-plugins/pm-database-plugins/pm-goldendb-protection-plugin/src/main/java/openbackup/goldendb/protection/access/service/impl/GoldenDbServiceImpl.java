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
package openbackup.goldendb.protection.access.service.impl;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CheckAppReq;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.goldendb.protection.access.constant.GoldenDbConstant;
import openbackup.goldendb.protection.access.dto.cluster.GoldenCluster;
import openbackup.goldendb.protection.access.dto.cluster.Node;
import openbackup.goldendb.protection.access.dto.instance.GoldenInstance;
import openbackup.goldendb.protection.access.dto.instance.Group;
import openbackup.goldendb.protection.access.dto.instance.Gtm;
import openbackup.goldendb.protection.access.dto.instance.MysqlNode;
import openbackup.goldendb.protection.access.service.GoldenDbService;
import com.huawei.oceanprotect.job.sdk.JobService;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.model.PagingParamRequest;
import openbackup.system.base.common.model.SortingParamRequest;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.model.job.request.QueryJobRequest;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

import java.lang.reflect.Field;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述 GoldenDbServiceImpl
 *
 * @author s30036254
 * @since 2023-02-13
 */
@Slf4j
@Service
public class GoldenDbServiceImpl implements GoldenDbService {
    private final ResourceService resourceService;

    private AgentUnifiedService agentUnifiedService;

    private final JobService jobService;

    /**
     * 构造器注入
     *
     * @param resourceService resourceService
     * @param agentUnifiedService agentUnifiedService
     * @param jobService jobService
     */
    public GoldenDbServiceImpl(ResourceService resourceService, AgentUnifiedService agentUnifiedService,
        JobService jobService) {
        this.resourceService = resourceService;
        this.agentUnifiedService = agentUnifiedService;
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
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Protected environment is not exists!"));
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
     * 获取单个节点的连通性
     *
     * @param mysqlNode mysql节点
     * @param environment 受保护环境
     * @return 是否连接
     */
    @Override
    public boolean singleConnectCheck(MysqlNode mysqlNode, ProtectedEnvironment environment) {
        // 通过agentId得到具体要使用的agentEnvironment来获得ip+port
        ProtectedEnvironment agentEnvironment = getEnvironmentById(mysqlNode.getParentUuid());
        AgentBaseDto agentBaseDto;
        try {
            agentBaseDto = agentUnifiedService.check("GoldenDB-cluster", agentEnvironment,
                getCheckReq(environment, mysqlNode));
            return agentBaseDto.isAgentBaseDtoReturnSuccess();
        } catch (LegoCheckedException e) {
            long errorCode = CommonErrorCode.AGENT_NETWORK_ERROR;
            ActionResult actionResult = new ActionResult();
            if (!VerifyUtil.isEmpty(e.getMessage())) {
                actionResult = JsonUtil.read(e.getMessage(), ActionResult.class);
                errorCode = Long.parseLong(actionResult.getBodyErr());
            }
            if (errorCode == GoldenDbConstant.MISS_COMPONENT) {
                String message = actionResult.getMessage();
                log.error("Get error information from plugin is {}", message);
                HashMap hash = JsonUtil.read(message, HashMap.class);
                List<String> pluginParameter = (List<String>) hash.get(GoldenDbConstant.PARAMETERS);
                throw new LegoCheckedException(errorCode, new String[] {pluginParameter.get(0)},
                    "The goldenDB environment is missing some components.");
            }
            if (errorCode == GoldenDbConstant.NODE_TYPE_MISMATCH) {
                String message = actionResult.getMessage();
                log.error("Get error information from plugin is {}", message);

                // 参数填充
                String osUser = mysqlNode.getOsUser();
                String nodeType = mysqlNode.getNodeType();
                String parentUuid = mysqlNode.getParentUuid();
                String endpoint = getEnvironmentById(parentUuid).getEndpoint();
                String nodeInfo = splitNodeType(nodeType);
                String[] parameters = {nodeInfo, endpoint, osUser, nodeType};
                log.error("nodeInfo is {},endpoint is {},osUser is {},nodeType is {}", nodeInfo, endpoint, osUser,
                    nodeType);
                throw new LegoCheckedException(errorCode, parameters, "node type mismatch");
            }
            log.error("GoldenDb check fail. ip: {}", mysqlNode.getIp());
            throw new LegoCheckedException(errorCode, "GoldenDb check fail.");
        } catch (LegoUncheckedException | FeignException e) {
            log.error("GoldenDb check fail. ip: {}", mysqlNode.getIp());
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "GoldenDb check fail.");
        }
    }

    private String splitNodeType(String nodeType) {
        return nodeType.substring(0, nodeType.length() - GoldenDbConstant.SUB_NODE_LENGTH).toUpperCase(Locale.ENGLISH);
    }

    /**
     * 获取单个节点的连通性
     *
     * @param mysqlNode mysql节点
     * @param environment 受保护环境
     * @return 只返回结果，不抛出异常
     */
    @Override
    public boolean singleHealthCheck(MysqlNode mysqlNode, ProtectedEnvironment environment) {
        try {
            return singleConnectCheck(BeanTools.copy(mysqlNode, MysqlNode::new), environment);
        } catch (LegoCheckedException | LegoUncheckedException | FeignException exception) {
            log.warn("fail to verify the manageDbNode,osUser is {}", mysqlNode.getOsUser());
            return false;
        }
    }

    /**
     * 获取计算节点对应的mysql
     *
     * @param environment 受保护环境
     * @return 计算节点
     */
    @Override
    public List<MysqlNode> getComputeNode(ProtectedEnvironment environment) {
        LinkedList<MysqlNode> mysqlNodes = new LinkedList<>();
        String clusterInfo = environment.getExtendInfo().get(GoldenDbConstant.CLUSTER_INFO);
        List<Group> groups = JsonUtil.read(clusterInfo, GoldenInstance.class).getGroup();
        groups.stream().forEach(group -> {
            group.getMysqlNodes().stream().forEach(mysqlNode -> {
                mysqlNodes.add(mysqlNode);
            });
        });
        return mysqlNodes;
    }

    /**
     * 获取管理数据库节点对应的mysql
     *
     * @param environment 受保护环境
     * @return 管理数据库节点
     */
    @Override
    public List<Node> getManageDbNode(ProtectedEnvironment environment) {
        String nodes = environment.getExtendInfo().get(GoldenDbConstant.GOLDEN_CLUSTER);
        return JsonUtil.read(nodes, GoldenCluster.class).getNodes();
    }

    /**
     * 获取管理数据库节点对应的mysql
     *
     * @param environment 受保护环境
     * @return 管理数据库节点
     */
    @Override
    public List<Gtm> getGtmNode(ProtectedEnvironment environment) {
        LinkedList<Gtm> gtmNodes = new LinkedList<>();
        String clusterInfo = environment.getExtendInfo().get(GoldenDbConstant.CLUSTER_INFO);
        List<Gtm> gtmList = JsonUtil.read(clusterInfo, GoldenInstance.class).getGtm();
        gtmList.stream().forEach(gtm -> {
            gtmNodes.add(gtm);
        });
        return gtmNodes;
    }

    /**
     * 获取所有的GoldenDb环境
     *
     * @param updateUuid updateUuid
     * @return 受保护环境
     */
    @Override
    public List<ProtectedResource> getGoldenDbEnv(String updateUuid) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("type", ResourceTypeEnum.DATABASE.getType());
        conditions.put("subType", ResourceSubTypeEnum.GOLDENDB_CLUSTER.getType());
        LinkedList<ProtectedResource> result = new LinkedList<>();
        List<ProtectedResource> records =
            resourceService.query(LegoNumberConstant.ZERO, LegoNumberConstant.HUNDRED, conditions).getRecords();
        records.forEach(resource -> {
            if (resource.getUuid() != null && !resource.getUuid().equals(updateUuid)) {
                result.add(resource);
            }
        });
        return result;
    }

    /**
     * 更新资源的状态
     *
     * @param resourceId resourceId
     * @param status status
     */
    @Override
    public void updateResourceLinkStatus(String resourceId, String status) {
        ProtectedResource updateResource = new ProtectedResource();
        updateResource.setUuid(resourceId);
        updateResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, status);
        resourceService.updateSourceDirectly(Collections.singletonList(updateResource));
    }

    /**
     * 得到环境下面的所有子实例
     *
     * @param parentUuid 环境uuid
     * @return 子资源
     */
    @Override
    public List<ProtectedResource> getChildren(String parentUuid) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(GoldenDbConstant.SUBTYPE, ResourceSubTypeEnum.GOLDENDB_CLUSETER_INSTANCE.getType());
        conditions.put(GoldenDbConstant.PARENT_UUID, parentUuid);
        return resourceService.query(LegoNumberConstant.ZERO, LegoNumberConstant.NUM_EXPORT_NUM, conditions)
            .getRecords();
    }

    private CheckAppReq getCheckReq(ProtectedEnvironment environment, MysqlNode mysqlNode) {
        Application application = new Application();
        application.setSubType(ResourceSubTypeEnum.GOLDENDB_CLUSTER.getType());
        application.setType(ResourceTypeEnum.DATABASE.getType());
        application.setAuth(environment.getAuth());
        HashMap<String, String> extendInfo = new HashMap<>();

        // 将mysql对象中的所有属性添加到extentInfo
        for (Field field : mysqlNode.getClass().getDeclaredFields()) {
            field.setAccessible(true);
            try {
                if (field.get(mysqlNode) != null) {
                    extendInfo.put(field.getName(), field.get(mysqlNode).toString());
                }
            } catch (IllegalAccessException e) {
                log.error("can not get param");
            }
        }
        application.setExtendInfo(extendInfo);
        AppEnv appEnv = BeanTools.copy(environment, AppEnv::new);
        return new CheckAppReq(appEnv, application);
    }

    /**
     * 查询当前最新的任务
     *
     * @param instanceId 实例id
     * @param type 备份/恢复
     * @return 任务状况
     */
    @Override
    public Optional<JobBo> queryLatestJob(String instanceId, String type) {
        // 过滤条件
        QueryJobRequest conditions = new QueryJobRequest();
        conditions.setSourceId(instanceId);
        conditions.setTypes(Collections.singletonList(type));

        // 排序条件
        SortingParamRequest sortParam = new SortingParamRequest();
        sortParam.setOrderBy(GoldenDbConstant.START_TIME);
        sortParam.setOrderType(GoldenDbConstant.DESC_ORDER_TYPE);

        // 分页大小
        PagingParamRequest pageParam = new PagingParamRequest();
        pageParam.setPageSize(1);
        PageListResponse<JobBo> jobBoPageListResponse = jobService.queryJobs(conditions, pageParam, sortParam, null);

        List<JobBo> jobs = jobService.queryJobs(conditions, pageParam, sortParam, null).getRecords();
        return jobs.stream().findFirst();
    }
}
