package openbackup.kingbase.protection.access.provider.backup;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.kingbase.protection.access.service.KingBaseService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * KingBase备份拦截器
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-13
 */
@Component
@Slf4j
public class KingBaseBackupInterceptorProvider extends AbstractDbBackupInterceptor {
    private final KingBaseService kingBaseService;

    /**
     * 构造方法
     *
     * @param kingBaseService kingbase业务类
     */
    public KingBaseBackupInterceptorProvider(KingBaseService kingBaseService) {
        this.kingBaseService = kingBaseService;
    }

    @Override
    public BackupTask initialize(BackupTask backupTask) {
        log.info("Start kingBase backup interceptor to set parameters. uuid: {}", backupTask.getTaskId());
        // 设置保护环境扩展参数deployType
        setProtectEnvExtendInfo(backupTask);

        // 设置存储仓
        backupTask.setRepositories(setRepositories(backupTask));

        ProtectedResource resource = kingBaseService.getResourceById(backupTask.getProtectObject().getUuid());

        // 设置子实例
        backupTask.setProtectSubObjects(kingBaseService.getSubInstances(resource));

        // 设置nodes
        backupTask.getProtectEnv().setNodes(kingBaseService.getEnvNodesByInstanceResource(resource));

        // 设置agents
        backupTask.setAgents(kingBaseService.getAgentsByInstanceResource(resource));
        log.info("End KingBase backup interceptor setting parameters. uuid: {}", backupTask.getTaskId());

        // 设置副本格式
        backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        log.info("Set copy format success!");

        // 设置速度统计方式为UBC
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);
        return backupTask;
    }

    private void setProtectEnvExtendInfo(BackupTask backupTask) {
        Map<String, String> envExtendInfo = Optional.ofNullable(backupTask.getProtectEnv().getExtendInfo())
            .orElseGet(HashMap::new);
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE,
            kingBaseService.getDeployType(backupTask.getProtectObject().getSubType()));
        backupTask.getProtectEnv().setExtendInfo(envExtendInfo);
    }

    private List<StorageRepository> setRepositories(BackupTask backupTask) {
        List<StorageRepository> repositories = backupTask.getRepositories();
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            StorageRepository logRepository = BeanTools.copy(repositories.get(IsmNumberConstant.ZERO),
                StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            repositories.add(logRepository);
            repositories.remove(0);
        }
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(IsmNumberConstant.ZERO),
            StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);
        return repositories;
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return Arrays.asList(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType(),
            ResourceSubTypeEnum.KING_BASE_CLUSTER_INSTANCE.getType()).contains(resourceSubType);
    }
}
