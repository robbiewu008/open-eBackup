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
package openbackup.opengauss.resources.access.provider;

import com.fasterxml.jackson.core.type.TypeReference;
import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.opengauss.resources.access.enums.OpenGaussClusterStateEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.opengauss.resources.access.constants.OpenGaussConstants;
import openbackup.opengauss.resources.access.constants.OpenGaussErrorCode;
import openbackup.opengauss.resources.access.service.OpenGaussAgentService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.StreamUtil;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * openGauss环境连通性检测提供者
 *
 */
@Component
@Slf4j
public class OpenGaussResourceConnectionChecker extends UnifiedResourceConnectionChecker {
    private static final int UNIQUE_SYSTEM_ID = 1;

    private final OpenGaussAgentService openGaussAgentService;

    private final ResourceService resourceService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param openGaussAgentService openGauss业务接口
     * @param resourceService openGauss资源调度接口
     */
    public OpenGaussResourceConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        AgentUnifiedService agentUnifiedService, OpenGaussAgentService openGaussAgentService,
        ResourceService resourceService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.openGaussAgentService = openGaussAgentService;
        this.resourceService = resourceService;
    }

    /**
     * openGauss应用和集群校验
     *
     * @param protectedResource 需要检查的资源
     * @return checkResult 连通性校验的结果
     */
    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource protectedResource) {
        CheckResult<Object> checkResult;
        try {
            checkResult = super.generateCheckResult(protectedResource);
            AppEnvResponse appEnvResponse = openGaussAgentService.getClusterNodeStatus(protectedResource);
            checkResult.setData(appEnvResponse);
        } catch (LegoCheckedException | NullPointerException e) {
            // 当发生异常时将environment往下传
            ActionResult actionResult = new ActionResult();
            actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
            actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            actionResult.setMessage("OpenGauss connection check failed: agent network error");
            ProtectedEnvironment agentEnv = protectedResource.getEnvironment();
            checkResult = new CheckResult<>();
            checkResult.setEnvironment(agentEnv);
            checkResult.setResults(actionResult);
        }
        return checkResult;
    }

    /**
     * 放置集群上下文信息
     *
     * @param checkReports checkReports联通性检查结果
     * @param context 上下文，根据需要自由使用
     * @return 检查结果列表或者抛出异常
     */
    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReports,
                                                   Map<String, Object> context) {
        boolean isSuccess = true;
        try {
            checkClusterUnique(checkReports);
            context.put(OpenGaussConstants.CLUSTER_INFO, checkReports.get(0).getResults().get(0).getData());
        } catch (LegoCheckedException | NullPointerException e) {
            isSuccess = false;
            log.error("Failed to check cluster unique.");
        }
        return super.collectActionResults(updateLinkStatus(checkReports, isSuccess), context);
    }

    private List<CheckReport<Object>> updateLinkStatus(List<CheckReport<Object>> checkReports, boolean isSuccess) {
        log.info("Try to update cluster status");
        // 主机离线，更新集群状态为离线
        ProtectedResource source = Optional.ofNullable(checkReports.get(0))
                .map(CheckReport::getResource)
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                        "not find opengauss cluster resource"));

        if (source.getUuid() == null) {
            log.info("Resource uuid is null, maybe register");
            return checkReports;
        }

        // 取得集群的extendInfo
        Map<String, String> extendInfo = source.getExtendInfo();
        ProtectedEnvironment newProtectedEnv = new ProtectedEnvironment();
        newProtectedEnv.setUuid(source.getUuid());

        if (!isSuccess) {
            log.error("Fail to check opengauss application");
            // 将数据库信息更新为离线
            extendInfo.put(OpenGaussConstants.CLUSTER_STATE, OpenGaussClusterStateEnum.UNAVAILABLE.getState());
            List<NodeInfo> nodeInfos = JsonUtil.read(extendInfo.get(OpenGaussConstants.NODES),
                    new TypeReference<List<NodeInfo>>() {
                    });
            // 主机离线，更新对应的主机状态为离线
            nodeInfos.forEach(
                    nodeInfo -> {
                        Map<String, String> nodeExtendInfo = nodeInfo.getExtendInfo();
                        nodeExtendInfo.put(OpenGaussConstants.STATUS, OpenGaussClusterStateEnum.UNAVAILABLE.getState());
                        nodeInfo.setExtendInfo(nodeExtendInfo);
                    }
            );
            extendInfo.put(OpenGaussConstants.NODES, JSONObject.writeValueAsString(nodeInfos));
        } else {
            // 打印集群信息 连通测试成功时将集群信息更新
            AppEnvResponse appEnvResponse = (AppEnvResponse) checkReports.get(0).getResults().get(0).getData();
            Map<String, String> clusterInfo = appEnvResponse.getExtendInfo();
            extendInfo.put(OpenGaussConstants.CLUSTER_STATE, clusterInfo.get(OpenGaussConstants.CLUSTER_STATE));
            extendInfo.put(OpenGaussConstants.NODES, JSONObject.writeValueAsString(appEnvResponse.getNodes()));
            log.debug("Successfully check opengauss application");
        }
        newProtectedEnv.setExtendInfo(extendInfo);
        resourceService.updateSourceDirectly(Stream.of(newProtectedEnv).collect(Collectors.toList()));

        return checkReports;
    }

    private void checkClusterUnique(List<CheckReport<Object>> checkReports) {
        List<AppEnvResponse> appEnvResponseList = checkReports.stream()
            .map(CheckReport::getResults)
            .flatMap(List::stream)
            .map(CheckResult::getData)
            .flatMap(StreamUtil.match(AppEnvResponse.class))
            .collect(Collectors.toList());
        // 首先判断systemId
        Set<String> clusterNodesSystemIds = appEnvResponseList.stream()
            .map(appEnvResponse -> appEnvResponse.getExtendInfo().get(OpenGaussConstants.SYSTEM_ID))
            .filter(systemId -> !VerifyUtil.isEmpty(systemId))
            .collect(Collectors.toSet());
        List<ActionResult> checkResultList = checkReports.stream()
            .map(CheckReport::getResults)
            .flatMap(List::stream)
            .map(CheckResult::getResults)
            .collect(Collectors.toList());
        log.info("clusterNodesSystemIds : {}", clusterNodesSystemIds);
        for (ActionResult actionResult : checkResultList) {
            String bodyErrCode = actionResult.getBodyErr();
            long errCode = (VerifyUtil.isEmpty(bodyErrCode)) ? OpenGaussErrorCode.SUCCESS : Long.parseLong(bodyErrCode);
            // 健康检查时发现集群状态不正常不立即抛出错误以便更新集群状态
            if (errCode != OpenGaussErrorCode.ERR_DATABASE_STATUS && errCode != OpenGaussErrorCode.SUCCESS) {
                log.error("ActionResult message: {}", actionResult.getMessage());
                throw new LegoCheckedException(Long.parseLong(bodyErrCode), actionResult.getMessage());
            }
        }
        if (VerifyUtil.isEmpty(clusterNodesSystemIds) || clusterNodesSystemIds.size() != UNIQUE_SYSTEM_ID) {
            throw new LegoCheckedException(OpenGaussErrorCode.CLUSTER_CLUSTER_TYPE_INCONSISTENT,
                "The openGauss selected cluster type does not match the application cluster type.");
        }
        // 加固：如果异机恢复后可能systemId一样，用ip再次判断
        Set<String> allIps = appEnvResponseList.stream().map(AppEnvResponse::getNodes).flatMap(List::stream)
            .map(NodeInfo::getEndpoint).collect(Collectors.toSet());
        Set<String> singleIps = appEnvResponseList.get(0).getNodes().stream()
            .map(NodeInfo::getEndpoint).collect(Collectors.toSet());
        if (!singleIps.containsAll(allIps)) {
            log.error("The openGauss registered nodes are not in the same cluster.");
            throw new LegoCheckedException(OpenGaussErrorCode.CLUSTER_CLUSTER_TYPE_INCONSISTENT,
                "The openGauss registered nodes are not in the same cluster.");
        }
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.OPENGAUSS.equalsSubType(protectedResource.getSubType());
    }
}
