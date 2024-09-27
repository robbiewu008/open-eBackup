package openbackup.tdsql.resources.access.provider;

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
import openbackup.tdsql.resources.access.constant.TdsqlConstant;
import openbackup.tdsql.resources.access.dto.instance.DataNode;
import openbackup.tdsql.resources.access.dto.instance.TdsqlInstance;
import openbackup.tdsql.resources.access.service.TdsqlService;
import openbackup.tdsql.resources.access.util.TdsqlUtils;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

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
 * 功能描述 测试实例节点连通性
 *
 * @author z30047175
 * @since 2023-05-30
 */
@Component
@Slf4j
public class TdsqlInstanceConnectionChecker extends UnifiedResourceConnectionChecker {
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
    public TdsqlInstanceConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
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
        return Objects.nonNull(object) && ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType()
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
        log.info("start tdsql instances[{}] collectConnectableResources", instanceResource.getUuid());

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
            log.error("end tdsql instances[{}] collectConnectableResources, but tdsql cluster[{}] is offline",
                instanceResource.getUuid(), cluster.getUuid());
            throw cast(results.get(0));
        }

        log.info("tdsql cluster[{}] connect check success, begin check date node of tdsql instance", cluster.getUuid());

        // 如果集群连通性正常， 再检查实例的连通性
        Map<ProtectedResource, List<ProtectedEnvironment>> dataNodeHostMap = collectDataNodeResource(instanceResource);

        log.info("end tdsql instances[{}] collectConnectableResources", instanceResource.getUuid());
        return dataNodeHostMap;
    }

    private Map<ProtectedResource, List<ProtectedEnvironment>> collectDataNodeResource(
        ProtectedResource instanceResource) {
        Map<ProtectedResource, List<ProtectedEnvironment>> nodeHostMap = new HashMap<>();
        List<DataNode> dataNodeList = tdsqlService.getInstanceDataNodes(instanceResource);
        dataNodeList.forEach(dataNode -> {
            ProtectedResource protectedResource = TdsqlUtils.getProtectedResource(instanceResource, dataNode,
                ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType());

            ProtectedEnvironment agentEnvironment = tdsqlService.getEnvironmentById(dataNode.getParentUuid());
            nodeHostMap.put(protectedResource, Lists.newArrayList(agentEnvironment));
        });

        return nodeHostMap;
    }

    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource resource) {
        ProtectedEnvironment agentEnv = resource.getEnvironment();
        log.info("Start tdsql instance connect check. cluster uuid: {}, " + "check agent uuid is {}",
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
            "end tdsql instance connect check. cluster uuid: {}, " + "check agent uuid is {}, check result error: {}",
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
                () -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "not find tdsql instance resource"));
        log.info("start update tdsql instance[{}] linkStatus, checkReport length is {}", instanceUuid,
            checkReportList.size());

        ProtectedResource resource = tdsqlService.getResourceById(instanceUuid);
        Map<String, String> extendInfo = resource.getExtendInfo();
        TdsqlInstance instance = JsonUtil.read(extendInfo.get(TdsqlConstant.CLUSTER_INSTANCE_INFO),
            TdsqlInstance.class);

        AtomicBoolean isInstanceOnline = new AtomicBoolean(true);
        instance.getGroups().stream().flatMap(item -> item.getDataNodes().stream()).forEach(dataNode -> {
            if (getCheckResult(dataNode.getParentUuid(), checkReportList)) {
                log.info("the data node[{}], of tdsql instance[{}] is ONLINE", dataNode.getIp(), instanceUuid);
                dataNode.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
            } else {
                log.warn("the data node[{}], of tdsql instance[{}] is OFFLINE", dataNode.getIp(), instanceUuid);
                isInstanceOnline.set(false);
                dataNode.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
            }
        });

        ProtectedResource updateResource = new ProtectedResource();
        updateResource.setUuid(instanceUuid);
        updateResource.setExtendInfoByKey(TdsqlConstant.LINKSTATUS, isInstanceOnline.get()
            ? LinkStatusEnum.ONLINE.getStatus().toString()
            : LinkStatusEnum.OFFLINE.getStatus().toString());
        updateResource.setExtendInfoByKey(TdsqlConstant.CLUSTER_INSTANCE_INFO, JsonUtil.json(instance));

        resourceService.updateSourceDirectly(Lists.newArrayList(updateResource));
        log.info("end update tdsql instance[{}] linkStatus", instanceUuid);

        return checkReportList;
    }

    private boolean getCheckResult(String uuid, List<CheckReport<Object>> checkReportList) {
        List<ActionResult> actionResultList = checkReportList.stream()
            .filter(item -> Objects.equals(uuid, item.getResource().getEnvironment().getUuid()))
            .flatMap(item -> item.getResults().stream())
            .map(CheckResult::getResults)
            .collect(Collectors.toList());
        return ResourceCheckContextUtil.isSuccess(actionResultList);
    }

    private LegoCheckedException cast(ActionResult result) {
        String[] params = Optional.ofNullable(result.getDetailParams())
            .map(e -> e.toArray(new String[0]))
            .orElse(new String[0]);
        return new LegoCheckedException(result.getCode(), params, result.getMessage());
    }
}
