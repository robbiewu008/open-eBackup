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
package openbackup.database.base.plugin.provider;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.AppConf;
import openbackup.database.base.plugin.common.GeneralDbConstant;
import openbackup.database.base.plugin.util.GeneralDbUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.beans.BeanUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * 通用数据库健康检查
 *
 */
@Component
@Slf4j
public class GeneralDbResourceConnectionChecker extends UnifiedResourceConnectionChecker {
    private final ResourceService resourceService;

    public GeneralDbResourceConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        AgentUnifiedService agentUnifiedService, ResourceService resourceService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.resourceService = resourceService;
    }

    /**
     * 增加应用的校验逻辑
     *
     * @param protectedResource 受保护资源
     * @return boolean 连通性校验的结果
     */
    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource protectedResource) {
        CheckResult<Object> checkResult = super.generateCheckResult(protectedResource);
        // 根据结果更新资源状态
        updateResourceStatus(protectedResource.getUuid(), checkResult.getResults().getCode());
        return checkResult;
    }

    private void updateResourceStatus(String resourceId, long returnCode) {
        if (returnCode == ActionResult.SUCCESS_CODE) {
            log.info("Start to update GeneralDbResource Online after connection check, resource id: {}", resourceId);
            updateEnvStatus(resourceId, LinkStatusEnum.ONLINE.getStatus().toString());
        } else {
            log.info("Start to update GeneralDbResource OFFLINE after connection check, resource id: {}", resourceId);
            updateEnvStatus(resourceId, LinkStatusEnum.OFFLINE.getStatus().toString());
        }
        log.info("Finished update GeneralDbResource status after connection check, resource id: {}", resourceId);
    }

    private void updateEnvStatus(String resourceId, String status) {
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid(resourceId);
        resource.setLinkStatus(status);
        // 直接将环境的状态同步到数据库，不走update的重接口，避免两次检查连通性而导致同步数据库失败
        resourceService.updateSourceDirectly(Stream.of(resource).collect(Collectors.toList()));
    }

    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(ProtectedResource resource) {
        List<ProtectedResource> nodes = GeneralDbUtil.getNodesFromDependency(resource);
        log.info("Nodes size: {}, resource id: {}.", nodes.size(), resource.getUuid());
        if (VerifyUtil.isEmpty(nodes)) {
            List<ProtectedEnvironment> hosts = GeneralDbUtil.getHostsFromHostKey(resource);
            log.info("Hosts size: {}, resource id: {}.", hosts.size(), resource.getUuid());
            return Collections.singletonMap(resource, hosts);
        }
        String script = resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY);
        String confStr = resource.getExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_CONF);
        Map<ProtectedResource, List<ProtectedEnvironment>> res = new HashMap<>();
        for (ProtectedResource node : nodes) {
            List<ProtectedEnvironment> hosts = GeneralDbUtil.getHostsFromHostKey(node);
            node.setExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY, script);
            node.setExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_CONF, confStr);
            ProtectedEnvironment cluster = new ProtectedEnvironment();
            BeanUtils.copyProperties(resource, cluster);
            cluster.setDependencies(null);
            node.setExtendInfoByKey(GeneralDbConstant.CLUSTER_FULL_INFO, JsonUtil.json(cluster));
            res.put(node, hosts);
        }
        return res;
    }

    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReport, Map<String, Object> context) {
        if (VerifyUtil.isEmpty(checkReport)) {
            return Collections.emptyList();
        }
        log.info("Check report size is: {}.", checkReport.size());

        int successCount = 0;
        List<ActionResult> failedActionResults = new ArrayList<>();
        for (CheckReport<Object> objectCheckReport : checkReport) {
            List<CheckResult<Object>> checkReportResults = objectCheckReport.getResults();
            if (VerifyUtil.isEmpty(checkReportResults)) {
                continue;
            }
            for (CheckResult<Object> checkResult : checkReportResults) {
                if (Objects.equals(checkResult.getResults().getCode(), ActionResult.SUCCESS_CODE)) {
                    successCount++;
                } else {
                    failedActionResults.add(checkResult.getResults());
                }
            }
        }

        AppConf appConf = GeneralDbUtil.getAppConf(
            checkReport.get(0).getResource().getExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_CONF)).orElse(null);
        Double checkThreshold = Optional.ofNullable(appConf)
            .map(AppConf::getResource)
            .map(AppConf.Resource::getClusterCheckResultThreshold)
            .orElse(1d);
        int validNum = (int) Math.ceil((successCount + failedActionResults.size()) * checkThreshold);
        log.info("Success count, valid num are: {}, {}.", successCount, validNum);
        if (successCount >= validNum) {
            return Collections.singletonList(new ActionResult());
        } else {
            return failedActionResults;
        }
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.GENERAL_DB.equalsSubType(object.getSubType());
    }
}
