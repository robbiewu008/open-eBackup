package openbackup.mysql.resources.access.provider;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CheckAppReq;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DbClusterProvider;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.RequestUriUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.net.URI;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * pxc集群类型校验
 *
 * @author xWX950025
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-25
 */
@Component
@Slf4j
public class PXClusterProvider implements DbClusterProvider {
    private final ResourceService resourceService;

    private final EncryptorService encryptorService;

    private final AgentUnifiedService agentUnifiedService;

    /**
     * 构造器注入
     *
     * @param resourceService resourceService
     * @param encryptorService encryptorService
     * @param agentUnifiedService agentUnifiedService
     */
    public PXClusterProvider(ResourceService resourceService, EncryptorService encryptorService,
        AgentUnifiedService agentUnifiedService) {
        this.resourceService = resourceService;
        this.encryptorService = encryptorService;
        this.agentUnifiedService = agentUnifiedService;
    }

    /**
     * 根据集群类型过滤对应的集群校验bean
     *
     * @param clusterType 集群类型
     * @return 匹配到bean true, 未匹配到 false
     */
    @Override
    public boolean applicable(String clusterType) {
        return MysqlConstants.MYSQL_PXC.equals(clusterType);
    }

    /**
     * 校验集群条件是否构成物理集群
     *
     * @param protectedResource 受保护资源
     * @return true 构成集群， false 不构成集群
     */
    @Override
    public boolean checkIsCluster(ProtectedResource protectedResource) {
        // 获取字实例
        List<ProtectedResource> subInstance = protectedResource.getDependencies().get(DatabaseConstants.CHILDREN);
        log.info("Build child instance name and collect environments");
        // 环境uuid和ip映射
        Map<String, String> uuidToIp = subInstance.stream().map(instance -> {
            String agentsUuid = instance.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid();
            return getEnvironmentById(agentsUuid);
        }).collect(Collectors.toMap(ProtectedEnvironment::getUuid, ProtectedEnvironment::getEndpoint));

        log.info("Build cluster nodes infos");
        // 创建所有集群所有节点信息，格式ip1:port1,ip2:port2
        String nodes = subInstance.stream().map(instance -> {
            String port = instance.getAuth().getExtendInfo().get(DatabaseConstants.INSTANCE_PORT);
            String instancePort = encryptorService.decrypt(port);
            String agentsUuid = instance.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid();
            String hostIp = uuidToIp.get(agentsUuid);
            return hostIp + DatabaseConstants.IP_PORT_SPLIT_CHAR + instancePort;
        }).collect(Collectors.joining(","));
        // pxc集群校验
        checkPXC(subInstance, nodes);
        return true;
    }

    private void checkPXC(List<ProtectedResource> subInstance, String nodes) {
        // 构建子实例的认证信息
        subInstance.forEach(protectedResource -> {
            log.info("Begin to build MySQL PXC Certification Information,PXC-instanceId: {}",
                protectedResource.getUuid());
            Authentication auth = protectedResource.getAuth();
            String pas = encryptorService.decrypt(auth.getAuthPwd());
            String insP = encryptorService.decrypt(auth.getExtendInfo().get(DatabaseConstants.INSTANCE_PORT));
            auth.getExtendInfo().put(DatabaseConstants.INSTANCE_PORT, insP);
            auth.setAuthPwd(pas);
        });

        ProtectedResource protectedResource = subInstance.get(0);
        ProtectedEnvironment environment = getEnvironmentById(
            protectedResource.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid());
        // agent框架根据subType进行接口调用
        Application copyResource = BeanTools.copy(protectedResource, Application::new);
        copyResource.setSubType(ResourceSubTypeEnum.MYSQL_CLUSTER.getType());
        Map<String, String> extendInfo = Optional.ofNullable(copyResource.getExtendInfo()).orElse(new HashMap<>());
        extendInfo.put(DatabaseConstants.ALL_NODES, nodes);
        copyResource.setExtendInfo(extendInfo);

        log.info("Build MySQL PXC Certification Information success, PXC-instanceId: {}", protectedResource.getUuid());
        CheckAppReq checkAppReq = new CheckAppReq();
        checkAppReq.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        checkAppReq.setApplication(copyResource);
        log.info("Begin to authenticate MySQL PXC cluster, PXC-instanceId: {}", protectedResource.getUuid());
        URI uri = RequestUriUtil.getRequestUri(environment.getEndpoint(), environment.getPort());
        AgentBaseDto checkResult = agentUnifiedService.check(ResourceSubTypeEnum.MYSQL_CLUSTER.getType(), environment,
            checkAppReq);
        if (Long.parseLong(checkResult.getErrorCode()) != 0) {
            log.error("MySQL pxc cluster authenticate failed");
            throw new DataProtectionAccessException(CommonErrorCode.TARGET_CLUSTER_AUTH_FAILED, new String[] {});
        }
        log.info("MySQL pxc cluster authenticate success, PXC-instanceId: {}", protectedResource.getUuid());
    }

    private ProtectedEnvironment getEnvironmentById(String envId) {
        Optional<ProtectedResource> resOptional = resourceService.getResourceById(envId);
        if (resOptional.isPresent() && resOptional.get() instanceof ProtectedEnvironment) {
            return (ProtectedEnvironment) resOptional.get();
        }
        throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected environment is not exists!");
    }
}
