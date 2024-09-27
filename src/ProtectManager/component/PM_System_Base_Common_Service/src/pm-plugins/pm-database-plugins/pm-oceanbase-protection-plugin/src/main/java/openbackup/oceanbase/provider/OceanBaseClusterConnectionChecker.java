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
package openbackup.oceanbase.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.oceanbase.common.constants.OBConstants;
import openbackup.oceanbase.common.dto.OBAgentInfo;
import openbackup.oceanbase.common.dto.OBClusterInfo;
import openbackup.oceanbase.common.util.OceanBaseUtils;
import openbackup.oceanbase.service.OceanBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-03
 */

@Component
@Slf4j
public class OceanBaseClusterConnectionChecker extends UnifiedResourceConnectionChecker {
    private static final long SUCCESS_CODE = 0L;

    private static final String ONLINE = LinkStatusEnum.ONLINE.getStatus().toString();

    private static final String OFFLINE = LinkStatusEnum.OFFLINE.getStatus().toString();

    private final OceanBaseService oceanBaseService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param oceanBaseService oceanBaseService
     */
    public OceanBaseClusterConnectionChecker(final ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        final AgentUnifiedService agentUnifiedService, OceanBaseService oceanBaseService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.oceanBaseService = oceanBaseService;
    }

    /**
     * 连通性子类过滤接口
     *
     * @param object 受保护资源
     * @return boolean 是否使用该类进行连通性校验
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && Objects.equals(ResourceSubTypeEnum.OCEAN_BASE_CLUSTER.getType(),
            object.getSubType());
    }

    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(ProtectedResource resource) {
        OBClusterInfo clusterInfo = OceanBaseUtils.readExtendClusterInfo(resource);
        List<ProtectedEnvironment> list = new ArrayList<>();
        for (OBAgentInfo agentInfo : clusterInfo.getObServerAgents()) {
            ProtectedEnvironment agentEnv = oceanBaseService.getEnvironmentById(agentInfo.getParentUuid());
            agentEnv.setExtendInfoByKey(OBConstants.KEY_CHECK_TYPE, OBConstants.CHECK_OBSERVER);
            agentEnv.setExtendInfoByKey(OBConstants.KEY_AGENT_IP, agentInfo.getIp());
            list.add(agentEnv);
        }
        for (OBAgentInfo agentInfo : clusterInfo.getObClientAgents()) {
            ProtectedEnvironment agentEnv = oceanBaseService.getEnvironmentById(agentInfo.getParentUuid());
            agentEnv.setExtendInfoByKey(OBConstants.KEY_CHECK_TYPE, OBConstants.CHECK_OBCLIENT);
            agentEnv.setExtendInfoByKey(OBConstants.KEY_AGENT_IP, agentEnv.getEndpoint());
            list.add(agentEnv);
        }

        Map<ProtectedResource, List<ProtectedEnvironment>> map = new HashMap<>();
        map.put(resource, list);
        return map;
    }

    /**
     * 检查OceanBase的OBClient和OBServer节点的状态。 该方法每次检查一个节点。
     *
     * @param resource 需要检查的资源
     * @return 检查结果
     */
    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource resource) {
        ProtectedEnvironment agentEnv = resource.getEnvironment();
        log.info("Start OceanBase cluster connect check. cluster uuid: {}, "
                + "check agent of OceanBase agent uuid is {}, check type is {}", resource.getUuid(), agentEnv.getUuid(),
            agentEnv.getExtendInfoByKey(OBConstants.KEY_CHECK_TYPE));

        resource.setExtendInfoByKey(OBConstants.KEY_CHECK_TYPE,
            agentEnv.getExtendInfoByKey(OBConstants.KEY_CHECK_TYPE));
        resource.setExtendInfoByKey(OBConstants.KEY_AGENT_IP, agentEnv.getExtendInfoByKey(OBConstants.KEY_AGENT_IP));

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

        log.info("end OceanBase cluster connect check. cluster uuid: {}, "
                + "check agent of OceanBase agent uuid is {}, check result error: {}", resource.getUuid(),
            resource.getEnvironment().getUuid(), JsonUtil.json(actionResult));

        return checkResult;
    }

    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReport, Map<String, Object> context) {
        // 用于集群注册和修改
        context.put(OBConstants.CONTENT_KEY_CONNECT_RESULT, JsonUtil.json(checkReport));
        return super.collectActionResults(updateLinkStatus(checkReport), context);
    }

    private List<CheckReport<Object>> updateLinkStatus(List<CheckReport<Object>> checkReportList) {
        ProtectedResource envInReq = checkReportList.get(0).getResource();
        // 注册或修改时，检查连通性之后不做任何操作，直接返回
        if (Objects.equals(envInReq.getExtendInfoByKey(OBConstants.KEY_CHECK_SCENE), OBConstants.CLUSTER_REGISTER)) {
            log.error("this is OceanBase cluster register or update, return");
            return checkReportList;
        }

        // copyEnv中仅保存连通状态， 用于更新数据库。
        ProtectedEnvironment copyEnv = initLinkStatus(envInReq);
        OceanBaseUtils.setLinkStatusBaseCheckResult(checkReportList, copyEnv);
        log.info("start update link status, cluster={}", copyEnv.getUuid());

        // 开始更新数据库
        if (Objects.equals(copyEnv.getLinkStatus(), OFFLINE)) {
            // 集群离线，设置集群下的所有租户集也离线
            oceanBaseService.setTenantSetStatue(copyEnv);
        } else {
            // 设置检查结果为Success
            checkReportList.stream()
                .flatMap(item -> item.getResults().stream())
                .map(CheckResult::getResults)
                .forEach(item -> item.setCode(SUCCESS_CODE));
        }
        log.info("cluster[uuid: {}] link status is {}", copyEnv.getUuid(), copyEnv.getLinkStatus());
        OceanBaseUtils.clearExtendInfo(copyEnv);
        oceanBaseService.updateSourceDirectly(copyEnv);
        log.info("end update link status, cluster={}", copyEnv.getUuid());
        return checkReportList;
    }

    private ProtectedEnvironment initLinkStatus(ProtectedResource protectedResource) {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid(protectedResource.getUuid());
        environment.setExtendInfo(protectedResource.getExtendInfo());
        OBClusterInfo clusterInfo = OceanBaseUtils.readExtendClusterInfo(environment);

        // 先将所有linkStatus默认设置为ONLINE
        OceanBaseUtils.updateAllLinkStatusOnline(clusterInfo);
        environment.setLinkStatus(ONLINE);

        environment.setExtendInfoByKey(OBConstants.KEY_CLUSTER_INFO, JsonUtil.json(clusterInfo));
        return environment;
    }
}
