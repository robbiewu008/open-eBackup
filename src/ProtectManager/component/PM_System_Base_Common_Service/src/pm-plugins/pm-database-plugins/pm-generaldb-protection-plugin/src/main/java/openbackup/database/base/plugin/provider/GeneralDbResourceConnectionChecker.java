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

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.AppConf;
import openbackup.database.base.plugin.common.GeneralDbConstant;
import openbackup.database.base.plugin.util.GeneralDbUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.BeanUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;

/**
 * 通用数据库健康检查
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-29
 */
@Component
@Slf4j
public class GeneralDbResourceConnectionChecker extends UnifiedResourceConnectionChecker {
    public GeneralDbResourceConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        AgentUnifiedService agentUnifiedService) {
        super(environmentRetrievalsService, agentUnifiedService);
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
