/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.oceanbase.common.util;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.oceanbase.common.constants.OBConstants;
import openbackup.oceanbase.common.constants.OBErrorCodeConstants;
import openbackup.oceanbase.common.dto.OBAgentInfo;
import openbackup.oceanbase.common.dto.OBClusterInfo;
import openbackup.oceanbase.service.OceanBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.util.BeanTools;

import com.fasterxml.jackson.core.type.TypeReference;
import com.google.common.collect.Lists;
import com.google.common.collect.Sets;

import lombok.extern.slf4j.Slf4j;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-06
 */
@Slf4j
public class OceanBaseUtils {
    private static final long SUCCESS_CODE = 0L;

    private static final String ONLINE = LinkStatusEnum.ONLINE.getStatus().toString();

    private static final String OFFLINE = LinkStatusEnum.OFFLINE.getStatus().toString();

    /**
     * 获取extendInfo扩展信息中的内容
     *
     * @param environment 受保护环境
     * @return 集群信息
     */
    public static OBClusterInfo readExtendClusterInfo(ProtectedResource environment) {
        String clusterInfos = Optional.ofNullable(environment)
            .map(ProtectedResource::getExtendInfo)
            .map(item -> item.get(OBConstants.KEY_CLUSTER_INFO))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Cluster info is empty!"));

        return JsonUtil.read(clusterInfos, OBClusterInfo.class);
    }

    /**
     * 获取extendInfo扩展信息中的内容
     *
     * @param task 受保护环境
     * @return 集群信息
     */
    public static OBClusterInfo readExtendClusterInfo(TaskEnvironment task) {
        String clusterInfos = Optional.ofNullable(task)
            .map(TaskEnvironment::getExtendInfo)
            .map(item -> item.get(OBConstants.KEY_CLUSTER_INFO))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Cluster info is empty!"));

        return JsonUtil.read(clusterInfos, OBClusterInfo.class);
    }

    /**
     * 生成ListResourceV2接口的请求参数
     *
     * @param environment 受保护环境
     * @param conditions conditions
     * @return 接口的请求参数
     */
    public static ListResourceV2Req generateListResourceV2Req(ProtectedEnvironment environment, String conditions) {
        ListResourceV2Req req = new ListResourceV2Req();
        req.setPageNo(0);
        req.setPageSize(10);
        req.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        req.setApplications(Collections.singletonList(BeanTools.copy(environment, Application::new)));
        req.setConditions(conditions);
        return req;
    }

    /**
     * 获取备份任务执行的agent
     *
     * @param backupTask 备份任务
     * @param oceanBaseService oceanBaseService
     * @return 备份任务执行的agent
     */
    public static List<Endpoint> supplyAgent(BackupTask backupTask, OceanBaseService oceanBaseService) {
        TaskEnvironment protectEnv = backupTask.getProtectEnv();
        OBClusterInfo obClusterInfo = OceanBaseUtils.readExtendClusterInfo(protectEnv);
        List<Endpoint> endpointList = new ArrayList<>();
        obClusterInfo.getObClientAgents()
            .stream()
            .filter(item -> Objects.equals(item.getLinkStatus(), ONLINE))
            .forEach(clientAgent -> {
                Endpoint endpoint = getEndpoint(oceanBaseService, clientAgent);
                endpointList.add(endpoint);
            });

        obClusterInfo.getObServerAgents().forEach(clientAgent -> {
            Endpoint endpoint = getEndpoint(oceanBaseService, clientAgent);
            endpointList.add(endpoint);
        });
        return endpointList;
    }

    /**
     * 获取备份任务执行的agent
     *
     * @param clusterEnv 集群谢谢
     * @param oceanBaseService oceanBaseService
     * @return 备份任务执行的agent
     */
    public static List<Endpoint> supplyAgent(ProtectedEnvironment clusterEnv, OceanBaseService oceanBaseService) {
        OBClusterInfo obClusterInfo = OceanBaseUtils.readExtendClusterInfo(clusterEnv);
        List<Endpoint> endpointList = new ArrayList<>();
        obClusterInfo.getObClientAgents()
            .stream()
            .filter(item -> Objects.equals(item.getLinkStatus(), ONLINE))
            .forEach(clientAgent -> {
                Endpoint endpoint = getEndpoint(oceanBaseService, clientAgent);
                endpointList.add(endpoint);
            });

        obClusterInfo.getObServerAgents().forEach(clientAgent -> {
            Endpoint endpoint = getEndpoint(oceanBaseService, clientAgent);
            endpointList.add(endpoint);
        });
        return endpointList;
    }

    /**
     * 获取备份任务执行的agent
     *
     * @param clusterEnv 集群谢谢
     * @param oceanBaseService oceanBaseService
     * @return 备份任务执行的agent
     */
    public static List<Endpoint> supplyAgentWithSingleClient(ProtectedEnvironment clusterEnv,
        OceanBaseService oceanBaseService) {
        OBClusterInfo obClusterInfo = OceanBaseUtils.readExtendClusterInfo(clusterEnv);
        List<Endpoint> endpointList = new ArrayList<>();
        OBAgentInfo clientAgentInfo = obClusterInfo.getObClientAgents()
            .stream()
            .filter(item -> Objects.equals(item.getLinkStatus(), ONLINE))
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.HOST_OFFLINE, "agent is offline."));
        endpointList.add(getEndpoint(oceanBaseService, clientAgentInfo));

        obClusterInfo.getObServerAgents().forEach(clientAgent -> {
            Endpoint endpoint = getEndpoint(oceanBaseService, clientAgent);
            endpointList.add(endpoint);
        });
        return endpointList;
    }

    private static Endpoint getEndpoint(OceanBaseService oceanBaseService, OBAgentInfo clientAgent) {
        ProtectedEnvironment agentEnv = oceanBaseService.getEnvironmentById(clientAgent.getParentUuid());
        Endpoint endpoint = new Endpoint();
        endpoint.setId(agentEnv.getUuid());
        endpoint.setIp(agentEnv.getEndpoint());
        endpoint.setPort(agentEnv.getPort());
        return endpoint;
    }

    /**
     * 有任一OBClient在线， OBServer全部在线，则集群在线
     *
     * @param clusterInfo 集群信息
     * @return true：在线， false：离线
     */
    public static boolean isClusterOnline(OBClusterInfo clusterInfo) {
        for (OBAgentInfo info : clusterInfo.getObServerAgents()) {
            if (Objects.equals(info.getLinkStatus(), LinkStatusEnum.OFFLINE.getStatus().toString())) {
                return false;
            }
        }
        for (OBAgentInfo info : clusterInfo.getObClientAgents()) {
            // 任一OBClient在线 则满足要求
            if (Objects.equals(info.getLinkStatus(), LinkStatusEnum.ONLINE.getStatus().toString())) {
                return true;
            }
        }
        return false;
    }

    /**
     * 根据检查结果设置linkStatus状态
     *
     * @param checkReportList 连通检查结果
     * @param copyEnv 受保护环境
     */
    public static void setLinkStatusBaseCheckResult(List<CheckReport<Object>> checkReportList,
        ProtectedEnvironment copyEnv) {
        OBClusterInfo clusterInfo = OceanBaseUtils.readExtendClusterInfo(copyEnv);
        log.info("Start set link status base OceanBase connect result. cluster[uuid: {}]", copyEnv.getUuid());
        for (CheckResult<Object> checkResult : checkReportList.get(0).getResults()) {
            ProtectedEnvironment agentEnv = checkResult.getEnvironment();
            String checkType = agentEnv.getExtendInfoByKey(OBConstants.KEY_CHECK_TYPE);
            ActionResult actionResult = checkResult.getResults();

            if (isObServerCheck(checkType) && actionResult.getCode() != SUCCESS_CODE) {
                // 更新OBServer为OFFLINE
                clusterInfo.getObServerAgents()
                    .stream()
                    .filter(agentInfo -> Objects.equals(agentEnv.getUuid(), agentInfo.getParentUuid()))
                    .forEach(agentInfo -> agentInfo.setLinkStatus(OFFLINE));
            }

            if (isObClientCheck(checkType)) {
                setOBClientStatus(clusterInfo, actionResult, agentEnv);
            }
            log.info("the result of check OceanBase agent, type is {}, uuid is {}, result:{}", checkType,
                agentEnv.getUuid(), JsonUtil.json(actionResult));
        }

        if (!OceanBaseUtils.isClusterOnline(clusterInfo)) {
            // 有任一OBClient在线，且OBServer全部在线，则集群在线
            copyEnv.setLinkStatus(OFFLINE);
        }

        copyEnv.setExtendInfoByKey(OBConstants.KEY_CLUSTER_INFO, JsonUtil.json(clusterInfo));
        log.info("End set link status base OceanBase connect result. cluster[uuid: {}] link status is {}",
            copyEnv.getUuid(), copyEnv.getLinkStatus());
    }

    private static boolean isObClientCheck(String checkType) {
        return Objects.equals(checkType, OBConstants.CHECK_OBCLIENT);
    }

    private static boolean isObServerCheck(String checkType) {
        return Objects.equals(checkType, OBConstants.CHECK_OBSERVER);
    }

    /**
     * 检查连通性性获取检查结果
     *
     * @param resourceCheckContext 资源检查上下文
     * @return AppEnvResponse
     */
    public static List<CheckReport<Object>> getContextCheckReport(ResourceCheckContext resourceCheckContext) {
        Map<String, Object> context = Optional.ofNullable(resourceCheckContext.getContext()).orElse(new HashMap<>());
        Object obj = context.get(OBConstants.CONTENT_KEY_CONNECT_RESULT);

        if (Objects.isNull(obj)) {
            log.error("failed to query OpenGauss cluster nodes.");
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "OceanBase connect check report is empty.");
        }
        return JsonUtil.read(obj.toString(), new TypeReference<List<CheckReport<Object>>>() {
        });
    }

    private static void changeOBServerOffLine(OBClusterInfo clusterInfo) {
        for (OBAgentInfo agentInfo : clusterInfo.getObServerAgents()) {
            agentInfo.setLinkStatus(OFFLINE);
        }
    }

    private static void setOBClientStatus(OBClusterInfo clusterInfo, ActionResult actionResult,
        ProtectedEnvironment agentEnv) {
        // OBClient节点， 根据检查的错误码进行细分
        long errorCode = actionResult.getCode();

        // OBClient返回认证错误或observer链接异常， 则将返回的有异常的OBServer节点状态设置为OFFLINE
        if (errorCode == OBErrorCodeConstants.AUTH_ERROR || errorCode == OBErrorCodeConstants.OBSERVER_CONNECT_ERROR
            || errorCode == OBErrorCodeConstants.OBSERVER_IP_NOT_MATCH) {
            // 更新对应OBServer为OFFLINE
            Set<String> ips = Sets.newHashSet(actionResult.getDetailParams());
            changeOBServerOfflineByIp(clusterInfo, ips);
        } else if (errorCode == OBErrorCodeConstants.CLUSTER_NODE_COUNT_NOT_SAME_ERROR
            || errorCode == OBErrorCodeConstants.OBSERVER_IS_NOT_ONE_CLUSTER_ERROR
            || errorCode == OBErrorCodeConstants.CLUSTER_STATUS_INVALID_ERROR) {
            // 将OBServer的所有节点状态设置为OFFLINE
            changeOBServerOffLine(clusterInfo);
        } else if (errorCode != SUCCESS_CODE) {
            // 其他错误场景，将OBClient设为OFFLINE
            changeOBClientOfflineById(clusterInfo, agentEnv);
        } else {
            log.info("result is success");
        }
    }

    private static void changeOBClientOfflineById(OBClusterInfo clusterInfo, ProtectedEnvironment agentEnv) {
        clusterInfo.getObClientAgents()
            .stream()
            .filter(obAgentInfo -> Objects.equals(obAgentInfo.getParentUuid(), agentEnv.getUuid()))
            .forEach(obAgentInfo -> obAgentInfo.setLinkStatus(OFFLINE));
    }

    private static void changeOBServerOfflineByIp(OBClusterInfo clusterInfo, Set<String> ips) {
        clusterInfo.getObServerAgents()
            .stream()
            .filter(agentInfo -> ips.contains(agentInfo.getIp()))
            .forEach(agentInfo -> agentInfo.setLinkStatus(OFFLINE));
    }

    /**
     * 将集群中所有节点状态设置为ONLINE
     *
     * @param clusterInfo clusterInfo
     */
    public static void updateAllLinkStatusOnline(OBClusterInfo clusterInfo) {
        for (OBAgentInfo agentInfo : clusterInfo.getObServerAgents()) {
            agentInfo.setLinkStatus(ONLINE);
        }
        for (OBAgentInfo agentInfo : clusterInfo.getObClientAgents()) {
            agentInfo.setLinkStatus(ONLINE);
        }
    }

    /**
     * 清除不需要入库的extendInfo
     *
     * @param resource resource
     */
    public static void clearExtendInfo(ProtectedResource resource) {
        getWantDeleteKey().forEach(key -> resource.getExtendInfo().remove(key));
    }

    private static List<String> getWantDeleteKey() {
        return Lists.newArrayList(OBConstants.KEY_AGENT_IP, OBConstants.KEY_CHECK_TYPE, OBConstants.KEY_CHECK_SCENE);
    }
}
