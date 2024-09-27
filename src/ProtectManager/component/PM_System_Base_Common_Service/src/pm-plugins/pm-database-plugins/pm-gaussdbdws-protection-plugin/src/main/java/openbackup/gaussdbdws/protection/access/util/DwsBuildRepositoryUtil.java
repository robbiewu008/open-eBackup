/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.gaussdbdws.protection.access.util;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.enums.BackupToolEnum;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.TargetClusterRequestParm;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.BeanTools;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.springframework.util.CollectionUtils;
import org.springframework.util.ObjectUtils;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 构建Repository 对象工具类
 *
 * @author nwx1077006
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-01
 */
@Slf4j
public class DwsBuildRepositoryUtil {
    /**
     * 获取缓存Repository工具类
     *
     * @param storageRepository storageRepository
     * @return CacheRepository
     */
    public static StorageRepository getCacheRepository(StorageRepository storageRepository) {
        StorageRepository newRepository = BeanTools.copy(storageRepository, StorageRepository::new);
        newRepository.setRole(DwsConstant.MASTER_ROLE);
        newRepository.setType(RepositoryTypeEnum.CACHE.getType());
        return newRepository;
    }

    /**
     * 给Repository修改 角色和添加 esn
     *
     * @param repository repository
     * @param esn esn号
     */
    public static void addRepositoryEsnAndRole(StorageRepository repository, String esn) {
        repository.setRole(DwsConstant.MASTER_ROLE);
        repository.setId(esn);
        addRepositoryEsn(repository, esn);
    }

    /**
     * Repository添加 esn
     *
     * @param repository repository
     * @param esn esn号
     */
    private static void addRepositoryEsn(StorageRepository repository, String esn) {
        Map<String, Object> extendInfo = Optional.ofNullable(repository.getExtendInfo()).orElse(new HashMap<>());
        extendInfo.put(DwsConstant.REPOSITORIES_KEY_ENS, esn);
        repository.setExtendInfo(extendInfo);
    }

    /**
     * 给高级参数修改对应的key=value 值
     *
     * @param advanceParams 高级参数
     * @param oldKey 老的key
     * @param newKey 新的key
     */
    public static void modifyAdvanceParam(Map<String, String> advanceParams, String oldKey, String newKey) {
        String value = advanceParams.get(oldKey);
        if (value != null) {
            advanceParams.put(newKey, value);
            advanceParams.remove(oldKey);
        }
    }

    /**
     * 给高级参数增加对应的key
     *
     * @param advanceParams 高级参数
     * @param existKey 已经存在key
     * @param addKey 需要增加key
     */
    public static void addSpeedStatisticsAdvanceParam(Map<String, String> advanceParams, String existKey,
        String addKey) {
        String value = advanceParams.get(existKey);
        if (BackupToolEnum.GDS.getType().equals(value)) {
            advanceParams.put(addKey, SpeedStatisticsEnum.UBC.getType());
        } else {
            advanceParams.put(addKey, SpeedStatisticsEnum.APPLICATION.getType());
        }
    }

    /**
     * 添加存储信息
     *
     * @param storageRepository 存储
     * @param clusterNativeApi 集群接口
     * @param encryptorService 加解密接口
     * @param isRestoreTask 是否恢复任务
     * @param clusterDetailInfo 本地存储集群信息
     */
    public static void buildAuthentication(StorageRepository storageRepository, ClusterNativeApi clusterNativeApi,
        EncryptorService encryptorService, boolean isRestoreTask, ClusterDetailInfo clusterDetailInfo) {
        if (storageRepository.getProtocol() == RepositoryProtocolEnum.S3.getProtocol()
            || storageRepository.getProtocol() == RepositoryProtocolEnum.TAPE.getProtocol()) {
            return;
        }
        String repositoryId = storageRepository.getId();
        ClusterDetailInfo cluster = getTargetCluster(clusterDetailInfo, repositoryId, clusterNativeApi);
        final Endpoint endpoint = buildTargetClusterEndPoint(repositoryId, cluster);
        storageRepository.setEndpoint(endpoint);

        Authentication authentication = buildAuthentication(cluster, encryptorService, isRestoreTask);
        storageRepository.setExtendAuth(authentication);
        addRepositoryEsn(storageRepository, cluster.getStorageSystem().getStorageEsn());
    }

    private static Endpoint buildTargetClusterEndPoint(String repositoryId, ClusterDetailInfo cluster) {
        final String ip = String.join(",", cluster.getSourceClusters().getMgrIpList());
        return new Endpoint(repositoryId, ip, cluster.getStorageSystem().getStoragePort());
    }

    private static Authentication buildAuthentication(ClusterDetailInfo cluster, EncryptorService encryptorService,
        boolean isRestoreTask) {
        Authentication newAuthInfo = new Authentication();
        newAuthInfo.setAuthKey(cluster.getStorageSystem().getUsername());
        if (isRestoreTask) {
            newAuthInfo.setAuthPwd(cluster.getStorageSystem().getPassword());
        } else {
            newAuthInfo.setAuthPwd(encryptorService.decrypt(cluster.getStorageSystem().getPassword()));
        }
        newAuthInfo.setAuthType(Authentication.APP_PASSWORD);
        return newAuthInfo;
    }

    private static ClusterDetailInfo getTargetCluster(ClusterDetailInfo clusterDetailInfo, String repositoryId,
        ClusterNativeApi clusterNativeApi) {
        if (repositoryId.equals(clusterDetailInfo.getStorageSystem().getStorageEsn())) {
            return clusterDetailInfo;
        }
        TargetClusterRequestParm requestParm = new TargetClusterRequestParm();
        requestParm.setEsnList(Lists.newArrayList(repositoryId));
        List<ClusterDetailInfo> clusterDetailInfoList = clusterNativeApi.queryTargetClusterListDetails(requestParm);
        if (CollectionUtils.isEmpty(clusterDetailInfoList)) {
            String info = "Can not find cluster(repositoryId:" + repositoryId + ") info";
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, info);
        }
        return clusterDetailInfoList.get(0);
    }

    /**
     * 配置修改repositories的认证信息
     *
     * @param repositories 集群列表
     * @param clusterNativeApi 集群接口
     * @param encryptorService 加解密接口
     * @param deployTypeService deployTypeService
     * @param isRestoreTask 是否需要加密
     */
    public static void addRepositoriesAuth(List<StorageRepository> repositories, ClusterNativeApi clusterNativeApi,
        EncryptorService encryptorService, DeployTypeService deployTypeService, boolean isRestoreTask) {
        if (deployTypeService.isE1000()) {
            return;
        }
        final ClusterDetailInfo clusterDetailInfo = clusterNativeApi.queryCurrentGroupClusterDetails();
        try {
            repositories.forEach(
                repository -> DwsBuildRepositoryUtil.buildAuthentication(repository, clusterNativeApi, encryptorService,
                    isRestoreTask, getClusterDetailInfo(clusterDetailInfo)));
        } finally {
            if (!ObjectUtils.isEmpty(clusterDetailInfo.getStorageSystem())) {
                StringUtil.clean(clusterDetailInfo.getStorageSystem().getPassword());
            }
        }
    }

    // 序列化clusterDetailInfo对象避免密码信息被反复加解密
    private static ClusterDetailInfo getClusterDetailInfo(ClusterDetailInfo clusterDetailInfo) {
        String jsonDetailInfo = JSONObject.fromObject(clusterDetailInfo).toString();
        return JsonUtil.read(jsonDetailInfo, ClusterDetailInfo.class);
    }
}
