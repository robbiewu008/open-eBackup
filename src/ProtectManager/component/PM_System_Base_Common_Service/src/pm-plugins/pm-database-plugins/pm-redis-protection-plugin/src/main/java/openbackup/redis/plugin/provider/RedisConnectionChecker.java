package openbackup.redis.plugin.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.utils.AuthParamUtil;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.redis.plugin.service.RedisService;
import openbackup.redis.plugin.util.RedisValidator;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * 集群环境和单节点连通性校验
 *
 * @author x30028756
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-14
 */
@Component
@Slf4j
public class RedisConnectionChecker extends UnifiedResourceConnectionChecker {
    private final RedisService redisService;

    private final KerberosService kerberosService;

    private final EncryptorService encryptorService;

    @Autowired
    private ProtectedEnvironmentService environmentService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param redisService redisService
     * @param kerberosService kerberosService
     * @param encryptorService encryptorService
     */
    public RedisConnectionChecker(final ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        final AgentUnifiedService agentUnifiedService, RedisService redisService, KerberosService kerberosService,
        EncryptorService encryptorService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.redisService = redisService;
        this.kerberosService = kerberosService;
        this.encryptorService = encryptorService;
    }

    /**
     * 连通性子类过滤接口
     *
     * @param object 受保护资源
     * @return boolean 是否使用该类进行连通性校验
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.REDIS.getType().equals(object.getSubType());
    }

    /**
     * 增加Redis应用的校验逻辑
     *
     * @param protectedResource 受保护资源
     * @return boolean 连通性校验的结果
     */
    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource protectedResource) {
        RedisValidator.checkNode(protectedResource);
        redisService.checkNodeExists(protectedResource);
        AuthParamUtil.convertKerberosAuth(protectedResource.getAuth(), kerberosService,
            protectedResource.getAuth().getExtendInfo().get(DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID),
            encryptorService);
        CheckResult<Object> checkResult = super.generateCheckResult(protectedResource);
        // 敏感信息最后一次使用结束后未在内存中移除
        AuthParamUtil.removeSensitiveInfo(protectedResource);
        return checkResult;
    }

    /**
     * 获取集群节点及其对应的主机
     *
     * @param environment 集群
     * @return key为集群节点，value为集群节点对应的主机列表
     */
    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(
        ProtectedResource environment) {
        log.info("RedisConnectionChecker,collectConnectableResources,environment.uuid: {}", environment.getUuid());
        Map<ProtectedResource, List<ProtectedEnvironment>> nodeHostMap = new LinkedHashMap<>();
        List<ProtectedResource> children = environment.getDependencies().get(ResourceConstants.CHILDREN);
        for (ProtectedResource child : children) {
            String uuid = child.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid();
            log.info("agent_id: {}", uuid);
            ProtectedEnvironment host = environmentService.getEnvironmentById(uuid);
            nodeHostMap.put(child, Lists.newArrayList(host));
        }
        return nodeHostMap;
    }
}