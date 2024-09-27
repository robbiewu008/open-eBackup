package openbackup.tidb.resources.access.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.constants.TidbConstants;
import openbackup.tidb.resources.access.service.TidbService;
import openbackup.tidb.resources.access.util.TidbUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * tidb 数据库连接检查
 *
 * @author w00426202
 * @since 2023-07-06
 */

@Slf4j
@Component
public class TidbDatabaseConnectionChecker extends UnifiedResourceConnectionChecker {
    /**
     * tidb service
     */
    private final TidbService tidbService;

    /**
     * TidbClusterConnectionChecker 构造函数
     *
     * @param environmentRetrievalsService environmentRetrievalsService
     * @param agentUnifiedService agentUnifiedService
     * @param tidbService tidbService
     */
    public TidbDatabaseConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        AgentUnifiedService agentUnifiedService, TidbService tidbService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.tidbService = tidbService;
    }

    /**
     * 连通性子类过滤接口
     *
     * @param object 受保护资源
     * @return boolean 是否使用该类进行连通性校验
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.TIDB_DATABASE.getType().equals(object.getSubType());
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
        log.info("TidbDatabaseConnectionChecker,collectConnectableResources,environment.uuid: {}",
            environment.getUuid());
        ProtectedResource clusterResource = tidbService.getResourceByCondition(environment.getParentUuid());
        clusterResource.getExtendInfo()
            .put(TidbConstants.DATABASE_NAME, environment.getExtendInfo().get(TidbConstants.DATABASE_NAME));
        clusterResource.getExtendInfo().put(TidbConstants.ACTION_TYPE, TidbConstants.CHECK_DB);
        return TidbUtil.getProtectedResourceListMap(clusterResource, tidbService);
    }
}
