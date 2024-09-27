package openbackup.tdsql.resources.access.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.util.ResourceCheckContextUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tdsql.resources.access.constant.TdsqlConstant;
import openbackup.tdsql.resources.access.dto.cluster.BaseNode;
import openbackup.tdsql.resources.access.dto.cluster.OssNode;
import openbackup.tdsql.resources.access.dto.cluster.TdsqlCluster;
import openbackup.tdsql.resources.access.service.TdsqlService;
import openbackup.tdsql.resources.access.util.TdsqlUtils;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.stream.Collectors;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-05-30
 */
@Component
@Slf4j
public class TdsqlClusterConnectionChecker extends UnifiedResourceConnectionChecker {
    private final TdsqlService tdsqlService;

    private final ResourceService resourceService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param tdsqlService tdsqlService
     * @param resourceService resourceService
     */
    public TdsqlClusterConnectionChecker(final ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        final AgentUnifiedService agentUnifiedService, TdsqlService tdsqlService, ResourceService resourceService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.tdsqlService = tdsqlService;
        this.resourceService = resourceService;
    }

    /**
     * 连通性子类过滤接口
     *
     * @param object 受保护资源
     * @return boolean 是否使用该类进行连通性校验
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.TDSQL_CLUSTER.getType().equals(object.getSubType());
    }

    /**
     * 获取集群节点及其对应的主机
     *
     * @param environment 集群
     * @return key为集群节点，value为集群节点对应的主机列表
     */
    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(
        ProtectedResource environment) {
        log.info("start tdsql cluster[{}] collectConnectableResources", environment.getUuid());
        TdsqlCluster tdsqlCluster = JsonUtil.read(environment.getExtendInfo().get(TdsqlConstant.CLUSTER_INFO),
            TdsqlCluster.class);

        Map<ProtectedResource, List<ProtectedEnvironment>> nodeHostMap = new LinkedHashMap<>();
        tdsqlCluster.getOssNodes().forEach(ossNode -> collectHostMap(environment, ossNode, nodeHostMap));
        tdsqlCluster.getSchedulerNodes()
            .forEach(schedulerNode -> collectHostMap(environment, schedulerNode, nodeHostMap));

        log.info("end tdsql cluster[{}] collectConnectableResources", environment.getUuid());
        return nodeHostMap;
    }

    private void collectHostMap(ProtectedResource resource, BaseNode node,
        Map<ProtectedResource, List<ProtectedEnvironment>> nodeHostMap) {
        String parentUuid = node.getParentUuid();
        ProtectedEnvironment agentEnvironment = tdsqlService.getEnvironmentById(parentUuid);
        log.info("agent_id: {},ip is {}", parentUuid, agentEnvironment.getEndpoint());
        ProtectedResource protectedResource = TdsqlUtils.getProtectedResource(resource, node,
            ResourceSubTypeEnum.TDSQL_CLUSTER.getType());

        nodeHostMap.put(protectedResource, Lists.newArrayList(agentEnvironment));
    }

    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource resource) {
        ProtectedEnvironment agentEnv = resource.getEnvironment();
        log.info("Start tdsql cluster connect check. cluster uuid: {}, " + "check agent uuid is {}", resource.getUuid(),
            agentEnv.getUuid());
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
            "end tdsql cluster connect check. cluster uuid: {}, " + "check agent uuid is {}, check result error: {}",
            resource.getUuid(), resource.getEnvironment().getUuid(), JsonUtil.json(actionResult));

        return checkResult;
    }

    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReport, Map<String, Object> context) {
        return super.collectActionResults(updateLinkStatus(checkReport), context);
    }

    private List<CheckReport<Object>> updateLinkStatus(List<CheckReport<Object>> checkReportList) {
        String clusterUuid = Optional.ofNullable(checkReportList.get(0))
            .map(CheckReport::getResource)
            .map(ProtectedResource::getUuid)
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "not find tdsql cluster environment"));

        log.info("start update tdsql cluster[{}] linkStatus, checkReport size is {}", clusterUuid,
            checkReportList.size());
        ProtectedEnvironment clusterEnv = tdsqlService.getEnvironmentById(clusterUuid);
        Map<String, String> extendInfo = clusterEnv.getExtendInfo();
        TdsqlCluster tdsqlCluster = JsonUtil.read(extendInfo.get(TdsqlConstant.CLUSTER_INFO), TdsqlCluster.class);

        boolean isClusterOnline = setNodeLinkStatus(checkReportList, tdsqlCluster);
        // 更新extendInfo中的clusterInfo
        extendInfo.put(TdsqlConstant.CLUSTER_INFO, JsonUtil.json(tdsqlCluster));

        ProtectedEnvironment updateEnv = new ProtectedEnvironment();
        updateEnv.setUuid(clusterUuid);
        // 任一一个节点offline，则集群offline
        updateEnv.setLinkStatus(getLinkStatusByResult(isClusterOnline));
        updateEnv.setExtendInfo(extendInfo);
        // 更新集群及节点的linkStatus
        resourceService.updateSourceDirectly(Collections.singletonList(updateEnv));

        // 集群OFFLINE，则更新子实例的linkStatus为OFFLINE， 集群ONLINE，则不用更新子实例， 由健康检查或实例连通性测试更新。
        if (!isClusterOnline) {
            log.warn("Because tdsql cluster[{}] is OFFLINE, start update all instances to OFFLINE", clusterUuid);
            setInstanceStatus(updateEnv);
            log.warn("End update all instances to OFFLINE");
        }

        log.info("end update tdsql cluster[{}] linkStatus", clusterUuid);
        return checkReportList;
    }

    private void setInstanceStatus(ProtectedEnvironment environment) {
        // 更新非分布式实例状态
        List<ProtectedResource> clusterInstance = Optional.ofNullable(
                tdsqlService.getChildren(environment.getUuid(), ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType()))
            .orElseGet(ArrayList::new);
        clusterInstance.forEach(
            it -> tdsqlService.updateInstanceLinkStatus(it, LinkStatusEnum.OFFLINE.getStatus().toString()));

        // 更新分布式实例状态
        List<ProtectedResource> clusterGroup = Optional.ofNullable(
                tdsqlService.getChildren(environment.getUuid(), ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType()))
            .orElseGet(ArrayList::new);
        clusterGroup.forEach(
            it -> tdsqlService.updateClusterGroupLinkStatus(it, LinkStatusEnum.OFFLINE.getStatus().toString()));
    }
    private boolean setNodeLinkStatus(List<CheckReport<Object>> checkReportList, TdsqlCluster tdsqlCluster) {
        AtomicBoolean isClusterOnline = new AtomicBoolean(true);
        for (CheckReport<Object> checkReport : checkReportList) {
            ProtectedResource resource = checkReport.getResource();
            OssNode node = JsonUtil.read(resource.getExtendInfoByKey(TdsqlConstant.SINGLENODE), OssNode.class);
            boolean isOnline = getCheckResult(checkReport.getResults());
            log.info("check result of node[{}], nodeType is {}, link status is {}", node.getIp(), node.getNodeType(),
                isOnline);
            // 任一一个节点offline，则集群offline
            if (!isOnline) {
                isClusterOnline.set(false);
            }

            // 根据节点类型，更新节点的linkStatus
            if (Objects.equals(node.getNodeType(), TdsqlConstant.OSS_NODE_TYPE)) {
                tdsqlCluster.getOssNodes()
                    .stream()
                    .filter(item -> Objects.equals(item.getParentUuid(), node.getParentUuid()))
                    .forEach(item -> item.setLinkStatus(getLinkStatusByResult(isOnline)));
            } else {
                tdsqlCluster.getSchedulerNodes()
                    .stream()
                    .filter(item -> Objects.equals(item.getParentUuid(), node.getParentUuid()))
                    .forEach(item -> item.setLinkStatus(getLinkStatusByResult(isOnline)));
            }
        }

        return isClusterOnline.get();
    }

    private boolean getCheckResult(List<CheckResult<Object>> results) {
        List<ActionResult> actionResultList = results.stream()
            .map(CheckResult::getResults)
            .collect(Collectors.toList());
        return ResourceCheckContextUtil.isSuccess(actionResultList);
    }

    private String getLinkStatusByResult(boolean isSuccess) {
        if (isSuccess) {
            return LinkStatusEnum.ONLINE.getStatus().toString();
        } else {
            return LinkStatusEnum.OFFLINE.getStatus().toString();
        }
    }
}
