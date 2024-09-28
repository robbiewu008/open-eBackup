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
package openbackup.tidb.resources.access.provider;

import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.thread.PushUpdateThreadPoolTool;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.constants.TidbConstants;
import openbackup.tidb.resources.access.model.TidbCondition;
import openbackup.tidb.resources.access.service.TidbService;
import openbackup.tidb.resources.access.util.TidbUtil;

import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.ObjectUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.stream.Collectors;

/**
 * TDSQL Cluster Provider
 *
 */
@Slf4j
@Component
public class TidbClusterProvider extends DatabaseEnvironmentProvider {
    private final TidbService tidbService;

    private final ResourceService resourceService;

    private final JsonSchemaValidator jsonSchemaValidator;

    /**
     * 构造方法
     *
     * @param providerManager provider manager
     * @param pluginConfigManager provider config manager
     * @param tidbService tdsqlService
     * @param resourceService resourceService
     * @param jsonSchemaValidator jsonSchemaValidator
     */
    public TidbClusterProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        TidbService tidbService, ResourceService resourceService, JsonSchemaValidator jsonSchemaValidator) {
        super(providerManager, pluginConfigManager);
        this.tidbService = tidbService;
        this.resourceService = resourceService;
        this.jsonSchemaValidator = jsonSchemaValidator;
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.TIDB_CLUSTER.getType().equals(resourceSubType);
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("start to check tidb cluster: {}.", environment.getName());
        jsonSchemaValidator.doValidate(environment, ResourceSubTypeEnum.TIDB_CLUSTER.getType());

        TidbUtil.checkTidbReqParam(environment);
        checkClusterInfoList(environment);

        List<Endpoint> endpointList = getEndpointFromEnvironment(environment);

        log.info("begin check tidbService.checkClusterInfo");
        tidbService.checkClusterInfo(environment, ResourceSubTypeEnum.TIDB_CLUSTER.getType(), endpointList.get(0));

        Map<String, String> extendInfoTmp = Optional.ofNullable(environment.getExtendInfo()).orElse(new HashMap<>());
        environment.setVersion(extendInfoTmp.get("version"));
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        environment.setUuid(UUIDGenerator.getUUID());
        environment.setEndpoint(endpointList.get(0).getIp());
        environment.setPath(endpointList.get(0).getIp());
    }

    /**
     * 创建/更新Tdsql集群时，校验ClusterInfoList参数
     *
     * @param protectedEnvironment 集群环境
     */
    private void checkClusterInfoList(ProtectedResource protectedEnvironment) {
        Map<String, String> extendInfo = protectedEnvironment.getExtendInfo();
        String clusterInfoList = MapUtils.getString(extendInfo, TidbConstants.CLUSTER_INFO_LIST);
        JSONArray clusterInfoArray = JSONArray.parseArray(clusterInfoList);
        if (Objects.isNull(clusterInfoArray) || clusterInfoArray.size() == 0) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "param illegal.");
        } else {
            for (int i = 0; i < clusterInfoArray.size(); i++) {
                // 遍历 jsonarray 数组，把每一个对象转成 json 对象
                JSONObject node = clusterInfoArray.getJSONObject(i);
                // 得到 每个对象中的属性值
                nodeCheck(node, TidbConstants.ID, TidbConstants.ROLE, TidbConstants.HOST, TidbConstants.STATUS,
                    TidbConstants.HOST_MANAGER_IP, TidbConstants.HOST_MANAGER_RESOURCE_UUID);
                // 1、校验agent id是否存在
                // 2、校验几个IP是否对应
                String hostManagerResourceUuid = node.get(TidbConstants.HOST_MANAGER_RESOURCE_UUID).toString();
                Optional<ProtectedResource> agentResourceOptional = resourceService.getResourceById(
                    hostManagerResourceUuid);
                if (!agentResourceOptional.isPresent()) {
                    log.error("TDSQL agent id {} is error.", hostManagerResourceUuid);
                    throw new LegoCheckedException(CommonErrorCode.AGENT_NOT_EXIST, "agent not exist");
                }
                ProtectedResource agent = agentResourceOptional.get();
                Map<String, String> agentExtendInfo = agent.getExtendInfo();
                String agentIpList = MapUtils.getString(agentExtendInfo, ResourceConstants.AGENT_IP_LIST);
                String endpoint = agent.getEndpoint();
                log.info("agentIpList is {}, endpoint is {}", agentIpList, endpoint);
                if (!agentIpList.contains(node.get(TidbConstants.HOST).toString()) || !StringUtils.equals(endpoint,
                    node.get(TidbConstants.HOST_MANAGER_IP).toString())) {
                    log.error("TDSQL agent id {} is error.", hostManagerResourceUuid);
                    throw new LegoCheckedException(CommonErrorCode.AGENT_MISMATCH_NODE,
                        new String[] {agent.getEndpoint(), node.get(TidbConstants.ID).toString()},
                        "The node information does not match.");
                }
                log.info("check agent ip success");
            }
        }
    }

    private void nodeCheck(JSONObject node, String... keys) {
        for (String key : keys) {
            Object nameObject = node.get(key);
            if (Objects.isNull(nameObject) || StringUtils.isEmpty(nameObject.toString())) {
                log.error("TDSQL cluster extendInfo {} is null.", key);
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "param illegal.");
            }
        }
    }

    private void checkClusterDuplic(ProtectedEnvironment environment, List<ProtectedResource> clusterList) {
        List<Map<String, String>> extendInfoList = clusterList.stream()
            .map(ProtectedResource::getExtendInfo)
            .collect(Collectors.toList());

        // 修改时跳过检查
        // save_type 0:注册/1:修改
        String saveType = environment.getExtendInfo().get(TidbConstants.SAVE_TYPE);
        if (StringUtils.equalsIgnoreCase(saveType, TidbConstants.SAVE_TYPE_UPDATE)) {
            return;
        }

        for (Map<String, String> extendInfo : extendInfoList) {
            if (StringUtils.equals(extendInfo.get(TidbConstants.CLUSTER_NAME),
                environment.getExtendInfo().get(TidbConstants.CLUSTER_NAME))) {
                log.error("The tidb cluster exists .");
                throw new LegoCheckedException(CommonErrorCode.RESOURCE_IS_REGISTERED, "The tidb cluster exists");
            }
        }
    }

    private List<Endpoint> getEndpointFromEnvironment(ProtectedEnvironment environment) {
        List<ProtectedResource> agentsResourceList = environment.getDependencies().get(DatabaseConstants.AGENTS);
        if (ObjectUtils.isEmpty(agentsResourceList)) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Environment dependencies agent is empty.");
        }
        List<Endpoint> endpointList = new ArrayList<>();
        for (ProtectedResource agentResource : agentsResourceList) {
            Endpoint endpoint = getEndpoint(agentResource.getUuid()).orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Agent is empty"));
            endpointList.add(endpoint);
        }
        return endpointList;
    }

    private Optional<Endpoint> getEndpoint(String agentId) {
        Optional<ProtectedResource> optResource = resourceService.getResourceById(agentId);
        return optResource.filter(resource -> resource instanceof ProtectedEnvironment)
            .map(resource -> (ProtectedEnvironment) resource)
            .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort()));
    }

    @Override
    public PageListResponse<ProtectedResource> browse(ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions) {
        log.info("browser cluster list", JsonUtil.json(environmentConditions));
        TidbCondition tidbCondition = JsonUtil.read(environmentConditions.getConditions(), TidbCondition.class);
        // 如果actiontype是check_tiup
        if (StringUtils.equals(TidbConstants.ACTION_TYPE_CHECK_TIUP, tidbCondition.getActionType())) {
            List<String> agentIds = tidbCondition.getAgentIds();
            List<PageListResponse<ProtectedResource>> clusters = new ArrayList<>();
            // 多个节点去查询获取到的集群信息
            for (String agentId : agentIds) {
                ProtectedResource endpointResource = tidbService.getResourceByCondition(agentId);
                Future<PageListResponse<ProtectedResource>> futureResult = PushUpdateThreadPoolTool.getPool()
                    .submit(() -> tidbService.getBrowseResult(environmentConditions, endpointResource,
                        tidbCondition.isCluster()));
                try {
                    clusters.add(futureResult.get());
                } catch (InterruptedException | ExecutionException | LegoUncheckedException | FeignException ex) {
                    log.error("Get browse result failed: %s", ExceptionUtil.getErrorMessage(ex));
                    throw new LegoCheckedException(CommonErrorCode.AGENT_NOT_BELONG_TO_SAME_CLUSTER,
                        "The selected management nodes do not belong to the same cluster.");
                }
            }
            // 如果返回为空，说明tiup节点有问题
            clusters.stream().forEach(item -> {
                if (CollectionUtils.size(item.getRecords()) == 0) {
                    log.error("Get no record.");
                    throw new LegoCheckedException(CommonErrorCode.AGENT_NOT_BELONG_TO_SAME_CLUSTER,
                        "The selected management nodes do not belong to the same cluster.");
                }
            });
            // 比较获取的集群信息是否一致
            Set<String> clusterSet = clusters.stream()
                .map(cluster -> MapUtils.getString(cluster.getRecords().get(0).getExtendInfo(), "cluster_list"))
                .collect(Collectors.toSet());
            // 大于1说明不一致
            if (CollectionUtils.size(clusterSet) > 1) {
                log.error("Tiups have different clusters.");
                throw new LegoCheckedException(CommonErrorCode.AGENT_NOT_BELONG_TO_SAME_CLUSTER,
                    "The selected management nodes do not belong to the same cluster.");
            }
            return clusters.get(0);
        } else {
            ProtectedResource endpointResource = tidbService.getResourceByCondition(environmentConditions.getEnvId());
            return tidbService.getBrowseResult(environmentConditions, endpointResource, tidbCondition.isCluster());
        }
    }

    /**
     * health check, 定时任务调用这个。
     *
     * @param environment environment
     */
    @Override
    public void validate(ProtectedEnvironment environment) {
        log.info("tidb cluster begin health check .");
        ProtectedResource clusterResource = tidbService.getResourceByCondition(environment.getUuid());
        List<ProtectedResource> resourceList = clusterResource.getDependencies().get(DatabaseConstants.AGENTS);
        String status = LinkStatusEnum.OFFLINE.getStatus().toString();
        for (ProtectedResource agentResource : resourceList) {
            // 检查集群
            try {
                tidbService.checkHealth(clusterResource, agentResource, ResourceSubTypeEnum.TIDB_CLUSTER.getType(),
                    TidbConstants.CHECK_CLUSTER);
                status = LinkStatusEnum.ONLINE.getStatus().toString();
                // 检查数据库，检查表
                checkDbTable(clusterResource, agentResource);
                break;
            } catch (LegoCheckedException e) {
                log.error("cluster health check failed code: {}, message: {}", e.getErrorCode(), e.getMessage());
            }
        }
        updateResourceStatus(clusterResource, status);
        if (StringUtils.equals(LinkStatusEnum.OFFLINE.getStatus().toString(), status)) {
            ProtectedResource agentResource = tidbService.getAgentResource(clusterResource);
            checkDbTable(clusterResource, agentResource);
        }
        log.info("tidb cluster end health check .");
    }

    private void checkDbTable(ProtectedResource clusterResource, ProtectedResource agentResource) {
        log.info("tidb db begin health check .");
        List<ProtectedResource> databaseResources = getSubResource(clusterResource);
        if (CollectionUtils.isNotEmpty(databaseResources)) {
            for (ProtectedResource databaseResource : databaseResources) {
                TidbUtil.wrapExtendInfo2Add(databaseResource, clusterResource);

                databaseResource.setAuth(clusterResource.getAuth());
                try {
                    tidbService.checkHealth(databaseResource, agentResource,
                        ResourceSubTypeEnum.TIDB_DATABASE.getType(), TidbConstants.CHECK_DB);

                    // 检查ok，则设置为正常状态
                    tidbService.updateResourceLinkStatus(Arrays.asList(databaseResource),
                        LinkStatusEnum.ONLINE.getStatus().toString());
                } catch (LegoCheckedException e) {
                    log.error("resource tidb-db health check error. cluster resource id is {}. ",
                        databaseResource.getUuid());
                    tidbService.updateResourceLinkStatus(Arrays.asList(databaseResource),
                        LinkStatusEnum.OFFLINE.getStatus().toString());
                }

                // 检查表
                checkTableHealth(clusterResource, agentResource, databaseResource);
            }
        }
    }

    private void checkTableHealth(ProtectedResource clusterResource, ProtectedResource agentResource,
        ProtectedResource databaseResource) {
        log.info("tidb table begin health check .");
        List<ProtectedResource> tableResourceList = getSubResource(databaseResource);
        if (CollectionUtils.isNotEmpty(tableResourceList)) {
            for (ProtectedResource tableResource : tableResourceList) {
                TidbUtil.wrapExtendInfo2Add(tableResource, clusterResource);
                String realTableNames = tableResource.getExtendInfo().get(TidbConstants.TABLE_NAME);
                tableResource.getExtendInfo().put(TidbConstants.TABLE_NAME_LIST, realTableNames);
                tableResource.setAuth(clusterResource.getAuth());

                try {
                    tidbService.checkHealth(tableResource, agentResource, ResourceSubTypeEnum.TIDB_TABLE.getType(),
                        TidbConstants.CHECK_TABLE);
                    tidbService.updateResourceLinkStatus(Arrays.asList(tableResource),
                        LinkStatusEnum.ONLINE.getStatus().toString());
                } catch (LegoCheckedException e) {
                    log.error("resource tidb-table health check error. cluster resource id is {}. ",
                        tableResource.getUuid());
                    tidbService.updateResourceLinkStatus(Arrays.asList(tableResource),
                        LinkStatusEnum.OFFLINE.getStatus().toString());
                }
            }
        }
    }

    private void updateResourceStatus(ProtectedResource protectedResource, String status) {
        // 查询集群下的所有数据库 & 表，设置为离线
        List<ProtectedResource> databaseResources = getSubResource(protectedResource);
        if (CollectionUtils.isEmpty(databaseResources)) {
            log.info("database num is 0, cluster resource id is {}", protectedResource.getUuid());
            return;
        }

        // 批量设置状态
        tidbService.updateResourceLinkStatus(databaseResources, status);

        // 获取所有的表，设置为离线
        for (ProtectedResource database : databaseResources) {
            List<ProtectedResource> subResource = getSubResource(database);
            if (CollectionUtils.isEmpty(subResource)) {
                return;
            }
            tidbService.updateResourceLinkStatus(subResource, status);
        }
    }

    private List<ProtectedResource> getSubResource(ProtectedResource protectedResource) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(TidbConstants.PARENT_UUID, protectedResource.getUuid());
        PageListResponse<ProtectedResource> resourceByCondition = tidbService.getResourceByCondition(conditions);
        return resourceByCondition.getRecords();
    }
}
