package openbackup.oracle.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * OracleConnectionChecker
 *
 * @author c30038333
 * @since 2023/3/31
 * @version [OceanProtect DataBackup 1.3.0]
 */
@Component
@Slf4j
public class OracleConnectionChecker extends UnifiedResourceConnectionChecker {
    private final ResourceService resourceService;

    private final OracleBaseService oracleBaseService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService   agent接口实现
     * @param resourceService   resourceService
     * @param oracleBaseService oracleBaseService
     */
    public OracleConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
            AgentUnifiedService agentUnifiedService, ResourceService resourceService,
            OracleBaseService oracleBaseService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.resourceService = resourceService;
        this.oracleBaseService = oracleBaseService;
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.ORACLE.equalsSubType(object.getSubType()) || ResourceSubTypeEnum.ORACLE_CLUSTER
                .equalsSubType(object.getSubType());
    }

    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(ProtectedResource resource) {
        Map<ProtectedResource, List<ProtectedEnvironment>> result = new HashMap<>();
        List<ProtectedEnvironment> agents;
        if (ResourceSubTypeEnum.ORACLE.equalsSubType(resource.getSubType())) {
            agents = Collections.singletonList(resource.getEnvironment());
        } else {
            agents = oracleBaseService.getOracleClusterHosts(resource);
        }
        result.put(resource, agents);
        return result;
    }

    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReport, Map<String, Object> context) {
        return super.collectActionResults(updateResourceLinkStatus(checkReport), context);
    }

    private List<CheckReport<Object>> updateResourceLinkStatus(List<CheckReport<Object>> checkReport) {
        String resourceId = checkReport.get(0).getResource().getUuid();
        // 注册时，检查连通性之后不做任何操作，直接返回
        if (!resourceService.getResourceById(resourceId).isPresent()) {
            return checkReport;
        }
        log.info("Start update Oracle link status after check connection, resource id: {}", resourceId);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(resourceId);
        // 对于集群数据库来说，只要有一个实例在线，集群数据库就在线
        List<CheckReport<Object>> online = checkReport.stream().filter(report -> isOnline(report.getResults()))
                .collect(Collectors.toList());
        if (online.size() > 0) {
            protectedResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
                    LinkStatusEnum.ONLINE.getStatus().toString());
            checkReport.get(0).getResults().forEach(checkResult -> checkResult.getResults()
                    .setCode(DatabaseConstants.SUCCESS_CODE));
            resourceService.updateSourceDirectly(Collections.singletonList(protectedResource));
            log.info("Finished update Oracle Online after check connection, resource id: {}", resourceId);
            return Collections.singletonList(checkReport.get(0));
        } else {
            protectedResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
                    LinkStatusEnum.OFFLINE.getStatus().toString());
            resourceService.updateSourceDirectly(Collections.singletonList(protectedResource));
            log.info("Finished update Oracle Offline after check connection, resource id: {}", resourceId);
            return checkReport;
        }
    }

    private boolean isOnline(List<CheckResult<Object>> results) {
        return results.stream().anyMatch(checkResult ->
                checkResult.getResults().getCode() == DatabaseConstants.SUCCESS_CODE);
    }
}
