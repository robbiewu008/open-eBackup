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
package openbackup.kingbase.protection.access.provider.resource;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.util.ResourceCheckContextUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.kingbase.protection.access.service.KingBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.stream.Collectors;

/**
 * KingBaseInstanceConnectionChecker连接检查
 *
 */
@Slf4j
@Component
public class KingBaseInstanceConnectionChecker extends UnifiedResourceConnectionChecker {
    private final KingBaseService kingBaseService;

    private final ResourceService resourceService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param kingBaseService KingBase服务
     * @param resourceService 资源服务
     */
    public KingBaseInstanceConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        AgentUnifiedService agentUnifiedService, KingBaseService kingBaseService, ResourceService resourceService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.kingBaseService = kingBaseService;
        this.resourceService = resourceService;
    }

    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource protectedResource) {
        try {
            CheckResult<Object> checkResult = super.generateCheckResult(protectedResource);
            if (checkResult.getResults().getCode() != ActionResult.SUCCESS_CODE) {
                ActionResult actionResult = new ActionResult(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                    "check application failed.");
                actionResult.setBodyErr(String.valueOf(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE));
                checkResult.setResults(actionResult);
            }
            return checkResult;
        } catch (LegoCheckedException | NullPointerException e) {
            log.error("Generate Check Result kingbase single instance error", ExceptionUtil.getErrorMessage(e));
            // 当无法收集到单实例的信息时，认为单实例状态异常，需要更新数据库中单实例在线信息
            ProtectedResource instanceResource = resourceService.getResourceById(protectedResource.getUuid())
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "not exist"));
            String status = LinkStatusEnum.OFFLINE.getStatus().toString();
            ProtectedResource updateResource = new ProtectedResource();
            updateResource.setUuid(instanceResource.getUuid());
            updateResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, status);
            resourceService.updateSourceDirectly(Collections.singletonList(updateResource));
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "target environment is offline.");
        }
    }

    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReport, Map<String, Object> context) {
        log.info("To deal with KingBase single instance collectActionResults");
        return super.collectActionResults(updateLinkStatus(checkReport), context);
    }

    private List<CheckReport<Object>> updateLinkStatus(List<CheckReport<Object>> checkReportList) {
        String instanceUuid = Optional.ofNullable(checkReportList.get(0))
            .map(CheckReport::getResource)
            .map(ProtectedResource::getUuid)
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "not find tdsql instance resource"));
        log.info("start update kingbase single instance {} linkStatus, checkReport length is {}", instanceUuid,
            checkReportList.size());

        ProtectedResource resource;
        try {
            resource = kingBaseService.getResourceById(instanceUuid);
        } catch (LegoCheckedException | NullPointerException e) {
            log.info("No need to update the resource status when registering,", ExceptionUtil.getErrorMessage(e));
            return checkReportList;
        }

        AtomicBoolean isInstanceOnline = new AtomicBoolean(true);
        resource.getDependencies().get(DatabaseConstants.AGENTS).forEach(agent -> {
            if (getCheckResult(agent.getUuid(), checkReportList)) {
                log.info("the agent {}, of kingbase single instance {} is ONLINE", agent.getUuid(), instanceUuid);
            } else {
                log.warn("the agent {}, of kingbase single instance {} is OFFLINE", agent.getUuid(), instanceUuid);
                isInstanceOnline.set(false);
            }
        });

        ProtectedResource updateResource = new ProtectedResource();
        updateResource.setUuid(instanceUuid);
        updateResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, isInstanceOnline.get()
            ? LinkStatusEnum.ONLINE.getStatus().toString()
            : LinkStatusEnum.OFFLINE.getStatus().toString());

        resourceService.updateSourceDirectly(Lists.newArrayList(updateResource));
        log.info("end update kingbase single instance {} linkStatus", instanceUuid);
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

    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.KING_BASE_INSTANCE.getType().equals(object.getSubType());
    }
}
