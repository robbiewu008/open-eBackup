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
import openbackup.database.base.plugin.common.DatabaseConstants;
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
 * SAP HANA数据库连通测试校验类
 *
 * @author wWX1013713
 * @version [DataBackup 1.5.0]
 * @since 2023-05-23
 */
@Component
@Slf4j
public class SapHanaDbConnectionChecker extends UnifiedResourceConnectionChecker {
    private final ResourceService resourceService;

    private final ProtectedEnvironmentService environmentService;

    private final SapHanaResourceService hanaResourceService;

    /**
     * SapHanaDbConnectionChecker有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param resourceService 资源业务类
     * @param environmentService 环境业务类
     * @param hanaResourceService SAP HANA资源业务类
     */
    public SapHanaDbConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        AgentUnifiedService agentUnifiedService, ResourceService resourceService,
        ProtectedEnvironmentService environmentService, SapHanaResourceService hanaResourceService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.resourceService = resourceService;
        this.environmentService = environmentService;
        this.hanaResourceService = hanaResourceService;
    }

    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(ProtectedResource resource) {
        log.info("Test connectivity collect connectable resources, database uuid: {}.", resource.getUuid());
        // 主机端口可能发生变化，查询最新的
        hanaResourceService.setDatabaseResourceInfo(resource);
        List<ProtectedEnvironment> envList = SapHanaUtil.parseHostProtectedEnvironmentList(resource);
        Map<ProtectedResource, List<ProtectedEnvironment>> res = new HashMap<>();
        res.put(resource, envList);
        return res;
    }

    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReport, Map<String, Object> context) {
        if (VerifyUtil.isEmpty(checkReport)) {
            log.error("Test connectivity collect action results, database check report is empty.");
            return Collections.emptyList();
        }
        String dbUuid = checkReport.get(0).getResource().getUuid();
        List<ActionResult> actionResults = checkReport.stream()
            .map(CheckReport::getResults)
            .flatMap(List::stream)
            .map(CheckResult::getResults)
            .collect(Collectors.toList());
        log.info("Test connectivity collect action results, database uuid: {}, results: {}.", dbUuid,
            JSONObject.stringify(actionResults));
        // 注册时，不处理资源状态，直接返回
        if (!resourceService.getResourceById(dbUuid).isPresent()) {
            return actionResults;
        }
        // 修改时，不处理资源状态，直接返回
        if (hanaResourceService.isModifyResource(checkReport.get(0).getResource())) {
            log.info("Test connectivity collect action results when modify, database uuid: {}.", dbUuid);
            return actionResults;
        }
        ProtectedResource dbResource = hanaResourceService.getResourceById(dbUuid);
        if (SapHanaUtil.isSystemDatabase(dbResource)) {
            // 系统数据库更新系统数据库和所属实例状态
            ProtectedEnvironment instance = environmentService.getEnvironmentById(dbResource.getParentUuid());
            if (checkHasSuccessNode(actionResults)) {
                hanaResourceService.updateDbLinkStatus(dbResource, LinkStatusEnum.ONLINE.getStatus().toString());
                hanaResourceService.updateInstanceLinkStatus(instance, LinkStatusEnum.ONLINE.getStatus().toString());
            } else {
                hanaResourceService.updateDbLinkStatus(dbResource, LinkStatusEnum.OFFLINE.getStatus().toString());
                // 系统数据库离线场景，租户数据库也设置为离线
                hanaResourceService.updateDbLinkStatusOfInstance(instance,
                    LinkStatusEnum.OFFLINE.getStatus().toString(), false, true);
                hanaResourceService.updateInstanceLinkStatus(instance, LinkStatusEnum.OFFLINE.getStatus().toString());
            }
        } else {
            if (checkAllNodeSuccess(actionResults)) {
                // 租户数据库在线则更新租户数据库、实例状态
                ProtectedEnvironment instance = environmentService.getEnvironmentById(dbResource.getParentUuid());
                hanaResourceService.updateDbLinkStatus(dbResource, LinkStatusEnum.ONLINE.getStatus().toString());
                hanaResourceService.updateInstanceLinkStatus(instance, LinkStatusEnum.ONLINE.getStatus().toString());
            } else {
                hanaResourceService.updateDbLinkStatus(dbResource, LinkStatusEnum.OFFLINE.getStatus().toString());
            }
        }
        return actionResults;
    }

    private boolean checkHasSuccessNode(List<ActionResult> actionResults) {
        return actionResults.stream().anyMatch(result -> result.getCode() == DatabaseConstants.SUCCESS_CODE);
    }

    private boolean checkAllNodeSuccess(List<ActionResult> actionResults) {
        return actionResults.stream().allMatch(result -> result.getCode() == DatabaseConstants.SUCCESS_CODE);
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.SAPHANA_DATABASE.equalsSubType(object.getSubType());
    }
}
