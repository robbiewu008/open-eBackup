package openbackup.data.access.framework.protection.service.archive;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterQueryService;
import openbackup.data.access.framework.backup.dto.StorageInfoDto;
import openbackup.data.access.framework.protection.service.repository.RepositoryStrategyManager;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseStorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryRoleEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.TargetClusterRequestParm;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.repository.model.BackupClusterVo;
import openbackup.system.base.sdk.repository.model.BackupUnitVo;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * 归档存储服务
 *
 * @author nwx1077006
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-17
 */
@Service
@Slf4j
public class ArchiveRepositoryService {
    private static final int LOCAL_CLUSTER_ID = 1;

    private final RepositoryStrategyManager repositoryStrategyManager;

    private final ClusterNativeApi clusterNativeApi;

    private final ClusterQueryService clusterQueryService;

    private final EncryptorService encryptorService;

    private final BackupStorageApi backupStorageApi;

    private TaskRepositoryManager taskRepositoryManager;

    /**
     * 构造
     *
     * @param repositoryStrategyManager repositoryStrategyManager
     * @param clusterNativeApi clusterNativeApi
     * @param clusterQueryService clusterQueryService
     * @param encryptorService encryptorService
     * @param backupStorageApi backupStorageApi
     */
    public ArchiveRepositoryService(RepositoryStrategyManager repositoryStrategyManager,
        ClusterNativeApi clusterNativeApi, ClusterQueryService clusterQueryService, EncryptorService encryptorService,
        BackupStorageApi backupStorageApi) {
        this.repositoryStrategyManager = repositoryStrategyManager;
        this.clusterNativeApi = clusterNativeApi;
        this.clusterQueryService = clusterQueryService;
        this.encryptorService = encryptorService;
        this.backupStorageApi = backupStorageApi;
    }

    @Autowired
    public void setTaskRepositoryManager(TaskRepositoryManager taskRepositoryManager) {
        this.taskRepositoryManager = taskRepositoryManager;
    }

    /**
     * 获得本地存储信息
     *
     * @return 本地存储
     */
    public StorageRepository buildLocalStorageRepository() {
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setProtocol(RepositoryProtocolEnum.NFS.getProtocol());
        storageRepository.setExtendAuth(queryNativeRepositoryAuth());
        ClusterDetailInfo cluster = this.getLocalCluster();
        storageRepository.setEndpoint(
            buildEndPoint(cluster.getStorageSystem().getStorageEsn(), cluster.getSourceClusters().getMgrIpList(),
                cluster.getStorageSystem().getStoragePort()));
        storageRepository.setId(cluster.getStorageSystem().getStorageEsn());
        storageRepository.setRole(RepositoryRoleEnum.MASTER_ROLE.getRoleType());
        storageRepository.setExtendInfo(buildRepositoryEsn(cluster.getStorageSystem().getStorageEsn()));
        return storageRepository;
    }

    private Map<String, Object> buildRepositoryEsn(String esn) {
        Map<String, Object> extendInfo = new HashMap<>();
        extendInfo.put(StorageRepository.REPOSITORIES_KEY_ENS, esn);
        return extendInfo;
    }

    /**
     * 查询本地存储认证信息
     *
     * @return 认证信息
     */
    public Authentication queryNativeRepositoryAuth() {
        // 本地存储直接查询本地集群，不需要存储库id
        return repositoryStrategyManager.getStrategy(RepositoryProtocolEnum.NATIVE_NFS)
            .getAuthentication(StringUtils.EMPTY);
    }

    /**
     * 获取归档任务对应存储库信息
     *
     * @param repositoryId 存储库id
     * @param protocolEnum 存储库协议枚举
     * @return 存储库信息 {@code StorageRepository}
     */
    public StorageRepository queryRepository(String repositoryId, RepositoryProtocolEnum protocolEnum) {
        final BaseStorageRepository baseRepository = new BaseStorageRepository();
        baseRepository.setId(repositoryId);
        baseRepository.setProtocol(protocolEnum.getProtocol());
        baseRepository.setType(RepositoryTypeEnum.DATA.getType());
        return repositoryStrategyManager.getStrategy(protocolEnum).getRepository(baseRepository);
    }

    /**
     *  查询存储信息列表
     *
     * @param storageId 存储库id
     * @return 存储库信息列表
     */
    public List<StorageRepository> buildSubRepositoryList(String storageId) {
        NasDistributionStorageDetail detail = backupStorageApi.getDetail(storageId);
        return detail.getUnitList()
            .stream()
            .map(BackupUnitVo::getBackupClusterVo)
            .filter(clustersInfoVo -> clustersInfoVo.getStatus() == ClusterEnum.StatusEnum.ONLINE.getStatus())
            .filter(clustersInfoVo -> clustersInfoVo.getClusterId() != LOCAL_CLUSTER_ID)
            .map(this::buildStorageRepository)
            .collect(Collectors.toList());
    }

    private Endpoint buildEndPoint(String repositoryId, List<String> mgrIpList, int port) {
        String ip = String.join(",", mgrIpList);
        return new Endpoint(repositoryId, ip, port);
    }

    private ClusterDetailInfo getLocalCluster() {
        final ClusterDetailInfo clusterDetailInfo = clusterNativeApi.queryDecryptCurrentGroupClusterDetails();
        if (clusterDetailInfo == null || clusterDetailInfo.getSourceClusters() == null
            || clusterDetailInfo.getStorageSystem() == null) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Can not find local cluster info");
        }
        return clusterDetailInfo;
    }

    private StorageRepository buildStorageRepository(BackupClusterVo backupClusterVo) {
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setProtocol(RepositoryProtocolEnum.NFS.getProtocol());
        storageRepository.setExtendAuth(buildAuthentication(backupClusterVo));
        storageRepository.setEndpoint(
            new Endpoint(backupClusterVo.getStorageEsn(), backupClusterVo.getClusterIp(), backupClusterVo.getPort()));
        storageRepository.setId(backupClusterVo.getStorageEsn());
        storageRepository.setRole(RepositoryRoleEnum.SLAVE_ROLE.getRoleType());
        storageRepository.setExtendInfo(buildRepositoryEsn(backupClusterVo.getStorageEsn()));
        return storageRepository;
    }

    private Authentication buildAuthentication(BackupClusterVo backupClusterVo) {
        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.OS_PASSWORD);
        TargetClusterRequestParm requestParm = new TargetClusterRequestParm(
            Collections.singletonList(backupClusterVo.getClusterId()));
        ClusterDetailInfo clusterDetail = clusterNativeApi.queryTargetClusterListDetails(requestParm).get(0);
        authentication.setAuthKey(backupClusterVo.getUsername());
        authentication.setAuthPwd(encryptorService.decrypt(clusterDetail.getStorageSystem().getPassword()));
        return authentication;
    }

    /**
     * 通过copyEsn获取对应节点仓库信息
     *
     * @param copyEsn copyEsn
     * @return 仓库信息
     */
    public StorageRepository buildStorageRepositoryByCopyEsn(String copyEsn) {
        StorageInfoDto storageInfoDto = new StorageInfoDto();
        storageInfoDto.setStorageDevice(copyEsn);
        StorageRepository storageRepository = taskRepositoryManager.buildStorageRepository(copyEsn,
            RepositoryProtocolEnum.NATIVE_NFS, storageInfoDto);

        storageRepository.setRole(RepositoryRoleEnum.ARCHIE_ROLE.getRoleType());
        if (storageRepository.getExtendInfo() == null) {
            storageRepository.setExtendInfo(new HashMap<>());
        }
        storageRepository.getExtendInfo().putAll(buildRepositoryEsn(copyEsn));
        return storageRepository;
    }
}
