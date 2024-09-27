package openbackup.saphana.protection.access.provider.resource;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.saphana.protection.access.service.SapHanaResourceService;
import openbackup.saphana.protection.access.util.SapHanaUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
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
 * SAP HANA实例连通测试校验类
 *
 * @author wWX1013713
 * @version [DataBackup 1.5.0]
 * @since 2023-05-20
 */
@Component
@Slf4j
public class SapHanaInstConnectionChecker extends UnifiedResourceConnectionChecker {
    private final ResourceService resourceService;

    private final ProtectedEnvironmentService environmentService;

    private final SapHanaResourceService hanaResourceService;

    /**
     * SapHanaInstConnectionChecker有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param resourceService 资源业务类
     * @param environmentService 环境业务类
     * @param hanaResourceService SAP HANA资源业务类
     */
    public SapHanaInstConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        AgentUnifiedService agentUnifiedService, ResourceService resourceService,
        ProtectedEnvironmentService environmentService, SapHanaResourceService hanaResourceService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.resourceService = resourceService;
        this.environmentService = environmentService;
        this.hanaResourceService = hanaResourceService;
    }

    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(ProtectedResource resource) {
        log.info("Test connectivity collect connectable resources, instance uuid: {}.", resource.getUuid());
        List<ProtectedResource> instHostResList = SapHanaUtil.parseDbHostProtectedResourceList(resource);
        // 主机端口可能发生变化，查询最新的
        List<ProtectedEnvironment> envList = hanaResourceService.queryEnvironments(instHostResList);
        Map<ProtectedResource, List<ProtectedEnvironment>> res = new HashMap<>();
        res.put(resource, envList);
        return res;
    }

    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReport, Map<String, Object> context) {
        if (VerifyUtil.isEmpty(checkReport)) {
            log.error("Test connectivity collect action results, instance check report is empty.");
            return Collections.emptyList();
        }
        String instanceUuid = checkReport.get(0).getResource().getUuid();
        List<ActionResult> actionResults = checkReport.stream()
            .map(CheckReport::getResults)
            .flatMap(List::stream)
            .map(CheckResult::getResults)
            .collect(Collectors.toList());
        log.info("Test connectivity collect action results, instance uuid: {}, results: {}.", instanceUuid,
            JSONObject.stringify(actionResults));
        // 注册时，不处理资源状态，直接返回
        if (!resourceService.getResourceById(instanceUuid).isPresent()) {
            return actionResults;
        }
        // 修改时，不处理资源状态，直接返回
        if (hanaResourceService.isModifyResource(checkReport.get(0).getResource())) {
            log.info("Test connectivity collect action results when modify, instance uuid: {}.", instanceUuid);
            return actionResults;
        }
        ProtectedEnvironment instance = environmentService.getEnvironmentById(instanceUuid);
        String instStatus = hanaResourceService.getInstStatusByActionResults(instance, actionResults);
        // 实例离线，更新实例、系统数据库、租户数据库状态；实例在线，只更新实例、系统数据库状态
        hanaResourceService.updateInstAndDbLinkStatusByInst(instance, instStatus, true,
            LinkStatusEnum.OFFLINE.getStatus().toString().equals(instStatus));
        return actionResults;
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.SAPHANA_INSTANCE.equalsSubType(object.getSubType());
    }
}
