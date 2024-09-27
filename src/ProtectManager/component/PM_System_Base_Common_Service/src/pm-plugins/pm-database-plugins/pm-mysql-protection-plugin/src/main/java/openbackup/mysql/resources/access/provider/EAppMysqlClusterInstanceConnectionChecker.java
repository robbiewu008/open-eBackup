package openbackup.mysql.resources.access.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.Lists;
import com.google.common.collect.Maps;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;

/**
 * 功能描述
 *
 * @author fWX1071802
 * @since 2023/8/3
 */
@Slf4j
@Component
public class EAppMysqlClusterInstanceConnectionChecker extends UnifiedResourceConnectionChecker {
    private final MysqlBaseService mysqlBaseService;

    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final AgentUnifiedService agentUnifiedService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param mysqlBaseService mysql基础服务
     * @param protectedEnvironmentService 环境服务
     */
    public EAppMysqlClusterInstanceConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        AgentUnifiedService agentUnifiedService, MysqlBaseService mysqlBaseService,
        ProtectedEnvironmentService protectedEnvironmentService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.mysqlBaseService = mysqlBaseService;
        this.agentUnifiedService = agentUnifiedService;
        this.protectedEnvironmentService = protectedEnvironmentService;
    }

    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource protectedResource) {
        CheckResult<Object> checkResult = new CheckResult<>();
        ActionResult actionResult = new ActionResult(ActionResult.SUCCESS_CODE, "");
        checkResult.setResults(actionResult);
        AppEnvResponse agentInfo = queryAgentInfo(protectedResource);
        if (agentInfo == null || agentInfo.getExtendInfo() == null) {
            actionResult.setBodyErr(CommonErrorCode.AGENT_NETWORK_ERROR + "");
            actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
            return checkResult;
        }
        String errorCode = agentInfo.getExtendInfo().get(MysqlConstants.ERROR_CODE);
        if (StringUtils.isNotBlank(errorCode)) {
            actionResult.setCode(Long.parseLong(errorCode));
            actionResult.setBodyErr(errorCode);
            log.error("Failed to check eapp, resource id:{}, code:{}", protectedResource.getUuid(), errorCode);
            return checkResult;
        }
        return super.generateCheckResult(protectedResource);
    }

    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(ProtectedResource resource) {
        List<ProtectedResource> nodes = resource.getDependencies().get(DatabaseConstants.CHILDREN);
        Map<ProtectedResource, List<ProtectedEnvironment>> res = Maps.newHashMapWithExpectedSize(nodes.size());
        for (ProtectedResource node : nodes) {
            String envId = node.getExtendInfo().get(DatabaseConstants.HOST_ID);
            ProtectedEnvironment env = protectedEnvironmentService.getEnvironmentById(envId);
            res.put(node, Lists.newArrayList(env));
        }
        return res;
    }

    @Override
    public boolean applicable(ProtectedResource resource) {
        if (resource == null) {
            return false;
        }
        String clusterType = resource.getExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE);
        return ResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.equalsSubType(resource.getSubType())
            && MysqlConstants.EAPP.equals(clusterType);
    }

    /**
     * 查询agent信息
     *
     * @param node 节点信息
     * @return agent信息
     */
    private AppEnvResponse queryAgentInfo(ProtectedResource node) {
        // 修改时，密码可能为空，需重新赋值密码
        if (VerifyUtil.isEmpty(node.getAuth().getAuthPwd())) {
            ProtectedResource oldInstance = mysqlBaseService.getResource(node.getUuid());
            node.getAuth().setAuthPwd(oldInstance.getAuth().getAuthPwd());
        }
        String envId = node.getExtendInfo().get(DatabaseConstants.HOST_ID);
        ProtectedEnvironment environment = mysqlBaseService.getEnvironmentById(envId);
        node.setPath(environment.getEndpoint());
        return agentUnifiedService.getClusterInfoNoRetry(node, environment);
    }
}
