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
package openbackup.tdsql.resources.access.provider;

import com.alibaba.fastjson.JSON;
import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.util.ResourceCheckContextUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.tdsql.resources.access.constant.TdsqlConstant;
import openbackup.tdsql.resources.access.dto.cluster.OssNode;
import openbackup.tdsql.resources.access.dto.cluster.TdsqlCluster;
import openbackup.tdsql.resources.access.dto.instance.TdsqlGroup;
import openbackup.tdsql.resources.access.service.TdsqlService;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.stream.Collectors;

/**
 * 功能描述 分布式实例节点连通性检查
 *
 */
@Component
@Slf4j
public class TdsqlClusterGroupConnectionChecker extends UnifiedResourceConnectionChecker {
    private final TdsqlService tdsqlService;

    private final ResourceService resourceService;

    /**
     * provider管控
     */
    private final ProviderManager providerManager;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param tdsqlService tdsqlService
     * @param resourceService resourceService
     * @param providerManager providerManager
     */
    public TdsqlClusterGroupConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        AgentUnifiedService agentUnifiedService, TdsqlService tdsqlService, ResourceService resourceService,
        ProviderManager providerManager) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.tdsqlService = tdsqlService;
        this.resourceService = resourceService;
        this.providerManager = providerManager;
    }

    /**
     * 连通性子类过滤接口
     *
     * @param object 受保护资源
     * @return boolean 是否使用该类进行连通性校验
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType()
            .equals(object.getSubType());
    }

    /**
     * 获取集群节点及其对应的主机
     *
     * @param instanceResource 实例
     * @return key为集群节点，value为集群节点对应的主机列表
     */
    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(
        ProtectedResource instanceResource) {
        log.info("start tdsql cluster group {} collectConnectableResources", instanceResource.getUuid());

        // 先检查下集群的连通性
        ProtectedEnvironment cluster = tdsqlService.getEnvironmentById(instanceResource.getParentUuid());
        // 使用checkContent方法，集群检查失败， 直接抛出异常，结束流程
        ResourceCheckContext checkContext = providerManager.findProvider(ResourceConnectionCheckProvider.class, cluster)
            .tryCheckConnection(cluster);

        List<ActionResult> results = Optional.ofNullable(checkContext.getActionResults())
            .orElseGet(Collections::emptyList)
            .stream()
            .filter(result -> result.getCode() != ActionResult.SUCCESS_CODE)
            .collect(Collectors.toList());
        if (!results.isEmpty()) {
            log.error("end tdsql cluster group {} collectConnectableResources, but tdsql cluster[{}] is offline",
                instanceResource.getUuid(), cluster.getUuid());
            throw cast(results.get(0));
        }

        log.info("tdsql cluster[{}] connect check success, begin check date node of tdsql instance", cluster.getUuid());
        // 如果集群连通性正常， 再检查实例的连通性
        // 下发给oss节点校验分布式实例连通性
        Map<ProtectedResource, List<ProtectedEnvironment>> dataNodeHostMap = collectDataNodeResource(instanceResource,
            cluster);
        log.info("end tdsql cluster group {} collectConnectableResources", instanceResource.getUuid());
        return dataNodeHostMap;
    }

    private Map<ProtectedResource, List<ProtectedEnvironment>> collectDataNodeResource(
        ProtectedResource instanceResource, ProtectedEnvironment clusterEnv) {
        Map<ProtectedResource, List<ProtectedEnvironment>> nodeHostMap = new HashMap<>();
        TdsqlCluster tdsqlCluster = JsonUtil.read(clusterEnv.getExtendInfo().get(TdsqlConstant.CLUSTER_INFO),
            TdsqlCluster.class);
        OssNode ossNode = tdsqlCluster.getOssNodes().get(0);
        ProtectedResource protectedResource = getProtectedResource(instanceResource, clusterEnv, ossNode,
            ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType());
        ProtectedEnvironment agentEnvironment = tdsqlService.getEnvironmentById(ossNode.getParentUuid());
        log.info("[TDSQL-clusterGroup]connection check agent_id: {},ip is {}", ossNode.getParentUuid(),
            agentEnvironment.getEndpoint());
        nodeHostMap.put(protectedResource, Lists.newArrayList(agentEnvironment));
        return nodeHostMap;
    }

    /**
     * getProtectedResource
     *
     * @param resource 资源
     * @param clusterEnv 集群资源
     * @param ossNode oss节点
     * @param subType subType
     * @return ProtectedResource
     */
    public static ProtectedResource getProtectedResource(ProtectedResource resource, ProtectedEnvironment clusterEnv,
        OssNode ossNode, String subType) {
        String clusterGroupInfo = resource.getExtendInfoByKey(TdsqlConstant.CLUSTER_GROUP_INFO);
        TdsqlGroup tdsqlGroup = JsonUtil.read(clusterGroupInfo, TdsqlGroup.class);
        HashMap<String, String> extendInfo = new HashMap<>();
        // 跟据关联的集群，获取任意一个oss节点，http://oss业务ip:port/tdsql
        String requestUrl = "http://" + ossNode.getIp() + ":" + ossNode.getPort() + "/tdsql";
        extendInfo.put(TdsqlConstant.REQUESTURL, requestUrl);
        extendInfo.put(TdsqlConstant.ID, tdsqlGroup.getGroup().getGroupId());
        extendInfo.put(TdsqlConstant.CHECK_TYPE, TdsqlConstant.CHECK_GROUP_INFO);
        extendInfo.put(TdsqlConstant.GROUP, JSON.toJSONString(tdsqlGroup.getGroup()));

        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(resource.getUuid());
        protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
        protectedResource.setSubType(subType);
        protectedResource.setExtendInfo(extendInfo);
        protectedResource.setAuth(clusterEnv.getAuth());
        return protectedResource;
    }

    private LegoCheckedException cast(ActionResult result) {
        String[] params = Optional.ofNullable(result.getDetailParams())
            .map(e -> e.toArray(new String[0]))
            .orElse(new String[0]);
        return new LegoCheckedException(result.getCode(), params, result.getMessage());
    }

    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource resource) {
        ProtectedEnvironment agentEnv = resource.getEnvironment();
        log.info("Start tdsql cluster group connect check. uuid: {}, " + "check agent uuid is {}",
            resource.getUuid(), agentEnv.getUuid());
        CheckResult<Object> checkResult;
        ActionResult actionResult;

        if (EnvironmentLinkStatusHelper.isOnlineAdaptMultiCluster(agentEnv)) {
            // 检查连通性
            checkResult = super.generateCheckResult(resource);
            actionResult = Optional.ofNullable(checkResult).map(CheckResult::getResults).orElse(new ActionResult());
        } else {
            actionResult = new ActionResult();
            actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
            actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            actionResult.setMessage("agent network error");

            checkResult = new CheckResult<>();
            checkResult.setEnvironment(agentEnv);
            checkResult.setResults(actionResult);
        }

        log.info(
            "end tdsql cluster group connect check. uuid: {}, " + "check agent uuid is {}, check result error: {}",
            resource.getUuid(), resource.getEnvironment().getUuid(), JsonUtil.json(actionResult));

        return checkResult;
    }

    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReport, Map<String, Object> context) {
        log.info("To deal with TDSQL collectActionResults");
        return super.collectActionResults(updateLinkStatus(checkReport), context);
    }

    private List<CheckReport<Object>> updateLinkStatus(List<CheckReport<Object>> checkReportList) {
        String instanceUuid = Optional.ofNullable(checkReportList.get(0))
            .map(CheckReport::getResource)
            .map(ProtectedResource::getUuid)
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "not find tdsql cluster group resource"));
        log.info("start update tdsql cluster group [{}] linkStatus, checkReport length is {}", instanceUuid,
            checkReportList.size());

        ProtectedResource resource = tdsqlService.getResourceById(instanceUuid);
        Map<String, String> extendInfo = resource.getExtendInfo();
        TdsqlGroup tdsqlGroup = JsonUtil.read(extendInfo.get(TdsqlConstant.CLUSTER_GROUP_INFO),
            TdsqlGroup.class);
        String mysqlVersion = extendInfo.get(TdsqlConstant.MYSQL_VERSION);

        AtomicBoolean isInstanceOnline = new AtomicBoolean(true);
        if (!getCheckResult(checkReportList)) {
            log.warn("the connection check failed, tdsql cluster group [{}] is OFFLINE", instanceUuid);
            isInstanceOnline.set(false);
        }

        ProtectedResource updateResource = new ProtectedResource();
        updateResource.setUuid(instanceUuid);
        updateResource.setExtendInfoByKey(TdsqlConstant.LINKSTATUS, isInstanceOnline.get()
            ? LinkStatusEnum.ONLINE.getStatus().toString()
            : LinkStatusEnum.OFFLINE.getStatus().toString());
        updateResource.setExtendInfoByKey(TdsqlConstant.CLUSTER_GROUP_INFO, JsonUtil.json(tdsqlGroup));
        updateResource.setExtendInfoByKey(TdsqlConstant.MYSQL_VERSION, mysqlVersion);

        resourceService.updateSourceDirectly(Lists.newArrayList(updateResource));
        log.info("end update tdsql cluster group [{}] linkStatus", instanceUuid);

        return checkReportList;
    }

    private boolean getCheckResult(List<CheckReport<Object>> checkReportList) {
        List<ActionResult> actionResultList = checkReportList.stream()
            .flatMap(item -> item.getResults().stream())
            .map(CheckResult::getResults)
            .collect(Collectors.toList());
        return ResourceCheckContextUtil.isSuccess(actionResultList);
    }
}
