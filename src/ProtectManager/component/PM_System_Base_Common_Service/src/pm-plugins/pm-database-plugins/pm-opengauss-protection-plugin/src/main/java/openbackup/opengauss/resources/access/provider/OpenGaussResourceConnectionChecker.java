package openbackup.opengauss.resources.access.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.opengauss.resources.access.constants.OpenGaussConstants;
import openbackup.opengauss.resources.access.constants.OpenGaussErrorCode;
import openbackup.opengauss.resources.access.service.OpenGaussAgentService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.StreamUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * openGauss环境连通性检测提供者
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-08
 */
@Component
@Slf4j
public class OpenGaussResourceConnectionChecker extends UnifiedResourceConnectionChecker {
    private static final int UNIQUE_SYSTEM_ID = 1;

    private final OpenGaussAgentService openGaussAgentService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param openGaussAgentService openGauss业务接口
     */
    public OpenGaussResourceConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        AgentUnifiedService agentUnifiedService, OpenGaussAgentService openGaussAgentService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.openGaussAgentService = openGaussAgentService;
    }

    /**
     * openGauss应用和集群校验
     *
     * @param protectedResource 需要检查的资源
     * @return checkResult 连通性校验的结果
     */
    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource protectedResource) {
        CheckResult<Object> checkResult = super.generateCheckResult(protectedResource);
        AppEnvResponse appEnvResponse = openGaussAgentService.getClusterNodeStatus(protectedResource);
        checkResult.setData(appEnvResponse);
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
        checkClusterUnique(checkReports);
        context.put(OpenGaussConstants.CLUSTER_INFO, checkReports.get(0).getResults().get(0).getData());
        return super.collectActionResults(checkReports, context);
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
            if (!VerifyUtil.isEmpty(bodyErrCode) && Long.parseLong(bodyErrCode) != OpenGaussErrorCode.SUCCESS) {
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
