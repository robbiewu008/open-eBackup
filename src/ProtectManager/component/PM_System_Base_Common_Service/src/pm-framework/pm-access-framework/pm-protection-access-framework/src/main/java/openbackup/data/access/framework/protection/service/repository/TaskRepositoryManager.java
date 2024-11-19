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
package openbackup.data.access.framework.protection.service.repository;

import static com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants.STORAGE_INFO;

import com.huawei.oceanprotect.base.cluster.sdk.enums.StorageUnitTypeEnum;
import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants;

import com.google.common.collect.Lists;

import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.backup.constant.BackupConstant;
import openbackup.data.access.framework.backup.dto.StorageInfoDto;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.protection.service.repository.strategies.RepositoryStrategy;
import openbackup.data.protection.access.provider.sdk.backup.v2.StorageRepositoryCreateService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseStorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryRoleEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.cluster.model.TargetClusterRequestParm;
import openbackup.system.base.sdk.protection.emuns.SlaPolicyTypeEnum;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.repository.model.BackupClusterVo;
import openbackup.system.base.sdk.repository.model.BackupUnitVo;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;
import org.springframework.util.Assert;
import org.springframework.util.CollectionUtils;

import java.math.BigDecimal;
import java.math.RoundingMode;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 获取备份/恢复的多集群RepositoryManager
 *
 **/
@Component
@Slf4j
@RequiredArgsConstructor
public class TaskRepositoryManager {
    /**
     * 百分比值
     */
    private static final double PERCENTAGE = 100d;

    private static final String ESN = "esn";

    private final ClusterNativeApi clusterNativeApi;

    private final BackupStorageApi backupStorageApi;

    private final EncryptorService encryptorService;

    private final RepositoryStrategyManager repositoryStrategyManager;

    private final StorageRepositoryCreateService storageRepositoryCreateService;

    private final StorageUnitService storageUnitService;

    /**
     * 根据slatype策略构造多集群的列表
     *
     * @param storageRepository 更新存储信息
     * @param storageId 存储库id
     */
    public void addRepositoryCapacity(StorageRepository storageRepository, String storageId) {
        List<Integer> clusterIdList = backupStorageApi.getDetail(storageId)
            .getUnitList()
            .stream()
            .map(BackupUnitVo::getBackupClusterVo)
            .filter(backupClusterVo -> Objects.equals(backupClusterVo.getStorageEsn(),
                storageRepository.getExtendInfo().get(StorageRepository.REPOSITORIES_KEY_ENS)))
            .map(BackupClusterVo::getClusterId)
            .collect(Collectors.toList());
        if (clusterIdList.isEmpty()) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED);
        }
        BackupClusterVo clusterStorage = backupStorageApi.getClusterStorage(storageId, clusterIdList.get(0));
        Map<String, Object> extendInfo = Optional.ofNullable(storageRepository.getExtendInfo()).orElse(new HashMap<>());
        extendInfo.put(StorageRepository.CAPACITY_AVAILABLE, capacityLimitExceeded(clusterStorage));
        storageRepository.setExtendInfo(extendInfo);
    }

    /**
     * 副本创建后不会更改信息，本地盘切换节点时更新副本中的存储单元信息
     * 更新esn和storage_info中的storage_device，ubc中默认使用storage_device存放存储单元信息，找不到时使用esn
     *
     * @param repositories 存储仓信息
     */
    public void updateCopyRepoInfo(List<StorageRepository> repositories) {
        for (StorageRepository repository : repositories) {
            if (VerifyUtil.isEmpty(repository.getExtendInfo())) {
                continue;
            }
            Object storageInfo = repository.getExtendInfo().get(STORAGE_INFO);
            if (storageInfo == null) {
                continue;
            }
            StorageInfoDto storageInfoDto = JSONObject.fromObject(storageInfo).toBean(StorageInfoDto.class);
            List<StorageUnitVo> storageUnits =
                storageUnitService.queryStorageUnitsByPoolId(storageInfoDto.getStoragePool());
            Optional<StorageUnitVo> storageUnit = storageUnits.stream()
                    .filter(unit -> StorageUnitTypeEnum.BASIC_DISK.getType().equals(unit.getDeviceType()))
                    .findAny();
            if (storageUnit.isPresent()) {
                StorageUnitVo unit = storageUnit.get();
                // 更新esn和storage_info中的storage_device，ubc中默认使用storage_device存放存储单元信息，找不到时使用esn
                repository.getExtendInfo().put(StorageRepository.REPOSITORIES_KEY_ENS, unit.getDeviceId());
                storageInfoDto.setStorageDevice(unit.getDeviceId());
                repository.getExtendInfo().put(ExtParamsConstants.STORAGE_INFO, storageInfoDto);
            } else {
                log.error("storage unit of basic disk is not exist");
            }
        }
    }

    /**
     * 根据slatype策略构造多集群的列表
     *
     * @param storageId 存储库id
     * @param slaType sla的type类型
     * @param isNeedAllRep 是否需要所有存储库
     * @return 多集群的列表
     */
    public List<StorageRepository> buildTargetRepositories(String storageId, String slaType, boolean isNeedAllRep) {
        List<StorageRepository> repositories = new ArrayList<>();
        if (StringUtils.isEmpty(storageId)) {
            return repositories;
        }

        // 根据不同的sla type 策略类型选择不同的构造Repositories列表
        if (SlaPolicyTypeEnum.BACKUP.getName().equals(slaType)) {
            return getNasStorageRepositories(storageId, isNeedAllRep);
        }
        return repositories;
    }

    /**
     * 根据slatype策略构造多集群的列表
     *
     * @param storageId 存储库id
     * @param slaType sla的type类型
     * @return 多集群的列表
     */
    public List<StorageRepository> buildTargetRepositories(String storageId, String slaType) {
        return buildTargetRepositories(storageId, slaType, false);
    }

    private List<StorageRepository> getNasStorageRepositories(String storageId, boolean isNeedAllRep) {
        NasDistributionStorageDetail storageDetail = backupStorageApi.getDetail(storageId);
        // 如果是dws并行存储或指定存储单元组的多节点备份，直接在这里返回所有列表
        if (storageDetail.isHasEnableParallelStorage() || isNeedAllRep) {
            return buildDwsStorageRepositories(storageDetail.getUnitList());
        }
        Map<String, Integer> clusterMap = storageDetail.getUnitList()
            .stream()
            .map(BackupUnitVo::getBackupClusterVo)
            .filter(backupClusterVo -> ClusterEnum.StatusEnum.ONLINE.getStatus() == backupClusterVo.getStatus())
            .filter(
                backupClusterVo -> isNeedAllRep ? true : backupClusterVo.getGeneratedType() == IsmNumberConstant.ONE)
            .collect(Collectors.toMap(BackupClusterVo::getStorageEsn, BackupClusterVo::getClusterId));
        List<Integer> clusterIdList = Lists.newArrayList(clusterMap.values());
        List<StorageRepository> repositories = new ArrayList<>();
        if (clusterIdList.isEmpty()) {
            log.warn("cluster is empty, no repositories");
            return repositories;
        }
        List<ClusterDetailInfo> clusterDetailInfos = clusterNativeApi.queryTargetClusterListDetails(
            new TargetClusterRequestParm(clusterIdList));
        for (ClusterDetailInfo clusterDetailInfo : clusterDetailInfos) {
            // 密码敏感信息 最终使用过后框架统一清理
            clusterDetailInfo.getStorageSystem()
                .setPassword(encryptorService.decrypt(clusterDetailInfo.getStorageSystem().getPassword()));
            getClusterCapacityAddRepository(storageId, repositories, clusterMap, clusterDetailInfo);
        }
        return repositories;
    }

    private List<StorageRepository> buildDwsStorageRepositories(List<BackupUnitVo> backupUnitList) {
        return backupUnitList.stream().map(backupUnitVo -> {
            StorageRepository storageRepository = new StorageRepository();
            storageRepository.setType(RepositoryTypeEnum.DATA.getType());
            storageRepository.setId(backupUnitVo.getDeviceId());
            storageRepository.setLocal(true);
            storageRepository.setProtocol(RepositoryProtocolEnum.NFS.getProtocol());
            Map<String, Object> extendInfo = storageRepository.getExtendInfo();
            if (VerifyUtil.isEmpty(extendInfo)) {
                extendInfo = new HashMap<>();
                storageRepository.setExtendInfo(extendInfo);
            }
            StorageInfoDto storageInfoDto = new StorageInfoDto();
            storageInfoDto.setStoragePool(backupUnitVo.getPoolId());
            storageInfoDto.setStorageDevice(backupUnitVo.getDeviceId());
            extendInfo.put(STORAGE_INFO, storageInfoDto);
            extendInfo.put(ESN, backupUnitVo.getDeviceId());
            return storageRepository;
        }).collect(Collectors.toList());
    }

    /**
     * 判断 能否登录目标存储集群 来决定是否传递下去
     *
     * @param storageId 存储库id
     * @param repositories 存储集群列表
     * @param clusterMap 存储集群列表map
     * @param clusterDetailInfo 单个集群的参数
     */
    private void getClusterCapacityAddRepository(String storageId, List<StorageRepository> repositories,
        Map<String, Integer> clusterMap, ClusterDetailInfo clusterDetailInfo) {
        try {
            repositories.add(buildBackupSlaveRepositories(clusterDetailInfo, storageId,
                clusterMap.get(clusterDetailInfo.getStorageSystem().getStorageEsn())));
        } catch (LegoCheckedException e) {
            log.error("get cluster capacity available failed code: {}, message: {}", e.getErrorCode(),
                e.getMessage());
        }
    }

    /**
     * 构造 slave repository
     *
     * @param clusterDetailInfo 查询的集群信息
     * @param storageId 存储库id
     * @param clusterId 集群id
     * @return slave repository
     */
    private StorageRepository buildBackupSlaveRepositories(ClusterDetailInfo clusterDetailInfo, String storageId,
        int clusterId) {
        StorageRepository newRepository = buildStorageRepository();
        newRepository.setLocal(true);
        String storageEsn = clusterDetailInfo.getStorageSystem().getStorageEsn();
        newRepository.setId(storageEsn);
        newRepository.setLocalCluster(clusterNativeApi.getCurrentEsn().equals(storageEsn));
        BackupClusterVo clusterStorage = backupStorageApi.getClusterStorage(storageId, clusterId);
        Map<String, Object> extendInfo = new HashMap<>();
        extendInfo.put(StorageRepository.CAPACITY_AVAILABLE, capacityLimitExceeded(clusterStorage));
        newRepository.setRole(RepositoryRoleEnum.SLAVE_ROLE.getRoleType());
        extendInfo.put(StorageRepository.REPOSITORIES_KEY_ENS, storageEsn);
        return buildRepositoryAuthInfo(clusterDetailInfo, newRepository, extendInfo);
    }

    private boolean capacityLimitExceeded(BackupClusterVo clusterStorage) {
        int availableCapacityRatio = clusterStorage.getAvailableCapacityRatio();
        BigDecimal count = clusterStorage.getUsedCapacity().multiply(BigDecimal.valueOf(PERCENTAGE))
                .divide(clusterStorage.getCapacity(), RoundingMode.HALF_UP);
        return count.compareTo(BigDecimal.valueOf(availableCapacityRatio)) < 0;
    }

    private StorageRepository buildStorageRepository() {
        RepositoryStrategy repositoryStrategy = repositoryStrategyManager.getStrategy(
            RepositoryProtocolEnum.NATIVE_NFS);
        BaseStorageRepository repository = new BaseStorageRepository();
        repository.setType(RepositoryTypeEnum.DATA.getType());
        return repositoryStrategy.getRepository(repository);
    }

    private StorageRepository buildStorageRepository(RepositoryProtocolEnum protocol) {
        RepositoryStrategy repositoryStrategy = repositoryStrategyManager.getStrategy(protocol);
        BaseStorageRepository repository = new BaseStorageRepository();
        repository.setType(RepositoryTypeEnum.DATA.getType());
        return repositoryStrategy.getRepository(repository);
    }

    /**
     * 查询存储单元组中的手动添加的外部存储单元
     *
     * @param storageType 存储单元类型
     * @param storageId 存储库id
     * @param isNeedAllRep 是否需要所有存储库
     * @return slave repository
     */
    public List<StorageRepository> buildExternalRepositories(String storageType, String storageId,
        boolean isNeedAllRep) {
        List<StorageRepository> repositories = Lists.newArrayList();
        if (StringUtils.isBlank(storageType) || StringUtils.isBlank(storageId)) {
            return repositories;
        }
        if (BackupConstant.BACKUP_EXT_PARAM_STORAGE_UNIT_GROUP_VALUE.equals(storageType)) {
            return getNasStorageRepositories(storageId, isNeedAllRep);
        }
        return repositories;
    }

    /**
     * 根据协议类型和nas存储库集群信息构造存储仓
     *
     * @param clusterStorage nas存储库集群信息
     * @param protocol 协议
     * @param isLocal 是否是本地存储仓
     * @return 存储仓信息
     */
    public StorageRepository buildBackupRepository(BackupClusterVo clusterStorage, RepositoryProtocolEnum protocol,
        boolean isLocal) {
        List<ClusterDetailInfo> clusterDetailInfos = clusterNativeApi.queryTargetClusterListDetails(
            new TargetClusterRequestParm(Collections.singletonList(clusterStorage.getClusterId())));
        if (clusterDetailInfos.isEmpty()) {
            throw new LegoCheckedException(CommonErrorCode.TARGET_CLUSTER_NOT_EXIST, "cluster not exist");
        }
        StorageRepository newRepository = buildStorageRepository(protocol);
        newRepository.setLocal(isLocal);
        newRepository.setId(clusterStorage.getStorageEsn());
        Map<String, Object> extendInfo = new HashMap<>();
        extendInfo.put(StorageRepository.CAPACITY_AVAILABLE, capacityLimitExceeded(clusterStorage));
        newRepository.setRole(RepositoryRoleEnum.SLAVE_ROLE.getRoleType());
        extendInfo.put(StorageRepository.REPOSITORIES_KEY_ENS, clusterStorage.getStorageEsn());
        ClusterDetailInfo clusterDetailInfo = clusterDetailInfos.get(0);
        return buildRepositoryAuthInfo(clusterDetailInfo, newRepository, extendInfo);
    }

    private StorageRepository buildRepositoryAuthInfo(ClusterDetailInfo clusterDetailInfo,
        StorageRepository newRepository, Map<String, Object> extendInfo) {
        Endpoint endpoint = new Endpoint();
        endpoint.setIp(clusterDetailInfo.getSourceClusters().getStorageDisplayIps());
        endpoint.setPort(clusterDetailInfo.getStorageSystem().getStoragePort());
        newRepository.setEndpoint(endpoint);
        newRepository.setExtendInfo(extendInfo);
        Authentication auth = new Authentication();
        auth.setAuthKey(clusterDetailInfo.getStorageSystem().getUsername());
        auth.setAuthType(Authentication.APP_PASSWORD);
        auth.setAuthPwd(clusterDetailInfo.getStorageSystem().getPassword());
        newRepository.setAuth(auth);
        newRepository.setExtendAuth(auth);
        return newRepository;
    }

    /**
     * 根据esn构建StorageRepository
     *
     * @param storageId esn
     * @param protocol 协议
     * @param storageInfoDto StorageInfoDto
     * @return StorageRepository
     */
    public StorageRepository buildStorageRepository(String storageId, RepositoryProtocolEnum protocol,
        StorageInfoDto storageInfoDto) {
        RepositoryStrategy repositoryStrategy = repositoryStrategyManager.getStrategy(protocol);
        BaseStorageRepository repository = new BaseStorageRepository();
        repository.setType(RepositoryTypeEnum.DATA.getType());
        repository.setId(storageId);
        Map<String, Object> extendInfo = repository.getExtendInfo();
        if (VerifyUtil.isEmpty(extendInfo)) {
            extendInfo = new HashMap<>();
            repository.setExtendInfo(extendInfo);
        }
        extendInfo.put(STORAGE_INFO, storageInfoDto);
        return repositoryStrategy.getRepository(repository);
    }

    /**
     * 获取副本信息中的存储库列表
     *
     * @param copyProperties 副本扩展信息
     * @param storageUnitId 副本的存储单元id
     * @return 存储库列表 {@code List<StorageRepository>}
     */
    public List<StorageRepository> getStorageRepositories(String copyProperties, String storageUnitId) {
        if (VerifyUtil.isEmpty(copyProperties)) {
            log.warn("The copyProperties is empty, can not obtain storage");
            return new ArrayList<>();
        }
        // 副本中的扩展参数数据，json字符串形式存储
        final JSONObject extendParam = JSONObject.fromObject(copyProperties);
        final JSONArray repositories = extendParam.getJSONArray(CopyPropertiesKeyConstant.KEY_REPOSITORIES);
        List<StorageRepository> res = null;
        if (CollectionUtils.isEmpty(repositories)) {
            // 通过storage unit id构建repositories
            res = (getRepositoryFromStorageUnit(storageUnitId));
        } else {
            res = repositories.map(repository -> this.buildStorageRepository(
                JSONObject.fromObject(repository).toBean(BaseStorageRepository.class)));
        }
        Assert.notEmpty(res, "Can not get repositories info");
        return res;
    }

    /**
     * 从副本获取存储仓（只有auth 和storageInfo信息） 用于调OSA接口
     *
     * @param copyProperties 副本参数
     * @param storageUnitId 副本的存储单元
     * @param repoType 存储仓类型
     * @return 存储仓（只有auth 和storageInfo信息）
     */
    public Optional<StorageRepository> getCopyRepoWithAuth(String copyProperties, String storageUnitId, int repoType) {
        return getStorageRepositories(copyProperties, storageUnitId).stream()
            .filter(storageRepository -> repoType == storageRepository.getType())
            .map(storageRepository -> {
                StorageRepository newRepo = new StorageRepository();
                newRepo.setExtendAuth(storageRepository.getExtendAuth());
                Map<String, Object> extendInfo = new HashMap<>();
                extendInfo.put(ExtParamsConstants.STORAGE_INFO,
                    storageRepository.getExtendInfo().get(ExtParamsConstants.STORAGE_INFO));
                newRepo.setExtendInfo(extendInfo);
                return newRepo;
            })
            .findAny();
    }

    private StorageRepository buildStorageRepository(BaseStorageRepository base) {
        log.info("Restore task build repository, id={}, type={}, protocol={}, role={}", base.getId(), base.getType(),
            base.getProtocol(), base.getRole());
        RepositoryProtocolEnum protocol = RepositoryProtocolEnum.getByProtocol(base.getProtocol());
        final RepositoryStrategy strategy = repositoryStrategyManager.getStrategy(protocol);
        final StorageRepository repository = strategy.getRepository(base);
        repository.encryptPassword();
        return repository;
    }

    private List<StorageRepository> getRepositoryFromStorageUnit(String storageUnitId) {
        List<StorageRepository> storageRepositoryList = new ArrayList<>();
        if (VerifyUtil.isEmpty(storageUnitId)) {
            log.warn("Storage unit id of copy is empty.");
            return storageRepositoryList;
        }
        storageRepositoryList.add(storageRepositoryCreateService.createRepositoryByStorageUnit(storageUnitId));
        return storageRepositoryList;
    }
}
