package openbackup.exchange.protection.access.interceptor;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.exchange.protection.access.service.ExchangeService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.StreamUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * Exchange 邮箱备份拦截器
 *
 * @author s30036254
 * @since 2023-05-09
 */
@Slf4j
@Component
public class ExchangeMailboxBackupInterceptor extends AbstractDbBackupInterceptor {
    private final ExchangeService exchangeService;

    /**
     * 构造器
     *
     * @param exchangeService exchangeService
     */
    public ExchangeMailboxBackupInterceptor(ExchangeService exchangeService) {
        this.exchangeService = exchangeService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.EXCHANGE_MAILBOX.getType().equals(object);
    }

    /**
     * 填充agent信息
     *
     * @param backupTask 备份对象
     */
    @Override
    protected void supplyAgent(BackupTask backupTask) {
        String envUuid = backupTask.getProtectEnv().getRootUuid();
        ProtectedEnvironment protectedEnvironment = exchangeService.getEnvironmentById(envUuid);
        List<Endpoint> supplyAgent = getSupplyAgent(protectedEnvironment.getDependencies());
        log.info("get supply agent: {}", supplyAgent);
        backupTask.setAgents(supplyAgent);
    }

    private List<Endpoint> getSupplyAgent(Map<String, List<ProtectedResource>> dependencies) {
        List<ProtectedResource> resourceList = dependencies.get(DatabaseConstants.AGENTS);
        return resourceList.stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .map(this::applyAgentEndpoint)
            .collect(Collectors.toList());
    }

    private Endpoint applyAgentEndpoint(ProtectedEnvironment agentProtectedEnv) {
        Endpoint endpoint = new Endpoint();
        endpoint.setIp(agentProtectedEnv.getEndpoint());
        endpoint.setPort(agentProtectedEnv.getPort());
        endpoint.setId(agentProtectedEnv.getUuid());
        endpoint.setAgentOS("windows");
        return endpoint;
    }

    /**
     * 不去检查实例连通性
     *
     * @param backupTask 备份对象
     */
    @Override
    protected void checkConnention(BackupTask backupTask) {
    }

    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        // 副本格式
        backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());

        // 部署类型
        Map<String, String> extendInfo = backupTask.getProtectEnv().getExtendInfo();
        extendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SHARDING.getType());
        List<StorageRepository> repositories = backupTask.getRepositories();
        StorageRepository dataRepository = repositories.get(IsmNumberConstant.ZERO);
        dataRepository.setProtocol(RepositoryProtocolEnum.CIFS.getProtocol());
        // 添加存储仓库类型：2-CACHE_REPOSITORY
        StorageRepository cacheRepository = BeanTools.copy(dataRepository, StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);
        // 高级参数设置speedStatistics等于true，表示使用UBC统计速度
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);

        // 查询上一次最新的副本id，保存在高级参数里
        Optional<Copy> latestCopy = exchangeService.getLatestCopy(backupTask.getProtectObject().getUuid());
        latestCopy.ifPresent(copy -> backupTask.getAdvanceParams().put("latest_copy_id", copy.getUuid()));
        return backupTask;
    }
}
