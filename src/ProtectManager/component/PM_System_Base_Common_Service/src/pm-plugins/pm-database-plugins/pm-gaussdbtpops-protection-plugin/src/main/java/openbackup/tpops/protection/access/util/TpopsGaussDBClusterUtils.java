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
package openbackup.tpops.protection.access.util;

import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import com.baomidou.mybatisplus.core.toolkit.CollectionUtils;
import com.google.common.collect.Lists;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.TargetClusterRequestParm;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.BeanTools;
import openbackup.tpops.protection.access.constant.TpopsGaussDBConstant;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * GaussDB项目实例工具类
 *
 */
public class TpopsGaussDBClusterUtils {
    /**
     * 获取缓存Repository工具类
     *
     * @param storageRepository storageRepository
     * @return CacheRepository
     */
    public static StorageRepository getCacheRepository(StorageRepository storageRepository) {
        StorageRepository newRepository = BeanTools.copy(storageRepository, StorageRepository::new);
        newRepository.setType(RepositoryTypeEnum.CACHE.getType());
        return newRepository;
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
     * 增加用户名
     *
     * @param protectEnv env
     * @param gaussDbUser 用户名
     */
    public static void initProtectEnvOfGaussDbUser(TaskEnvironment protectEnv, String gaussDbUser) {
        protectEnv.getExtendInfo().put(TpopsGaussDBConstant.EXTEND_INFO_KEY_GAUSSDB_USER, gaussDbUser);
    }

    /**
     * 给Repository修改 角色和添加 esn
     *
     * @param repository repository
     * @param esn esn号
     */
    public static void addRepositoryEsnAndRole(StorageRepository repository, String esn) {
        repository.setRole(TpopsGaussDBConstant.MASTER_ROLE);
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
        extendInfo.put(TpopsGaussDBConstant.REPOSITORIES_KEY_ENS, esn);
        repository.setExtendInfo(extendInfo);
    }

    /**
     * 添加存储信息
     *
     * @param storageRepository 存储
     * @param clusterNativeApi 集群接口
     * @param encryptorService 加解密接口
     * @param deployTypeService deployTypeService
     * @param isRestoreTask 是否恢复任务
     */
    public static void buildAuthentication(StorageRepository storageRepository, ClusterNativeApi clusterNativeApi,
        EncryptorService encryptorService, DeployTypeService deployTypeService, boolean isRestoreTask) {
        if (storageRepository.getProtocol() == RepositoryProtocolEnum.S3.getProtocol()
            || storageRepository.getProtocol() == RepositoryProtocolEnum.TAPE.getProtocol()) {
            return;
        }
        if (deployTypeService.isE1000()) {
            return;
        }
        String repositoryId = storageRepository.getId();
        ClusterDetailInfo cluster = getTargetCluster(repositoryId, clusterNativeApi);
        final Endpoint endpoint = buildTargetClusterEndPoint(repositoryId, cluster);
        storageRepository.setEndpoint(endpoint);

        Authentication authentication = buildAuthentication(cluster, encryptorService, isRestoreTask);
        storageRepository.setExtendAuth(authentication);
        addRepositoryEsn(storageRepository, cluster.getStorageSystem().getStorageEsn());
    }

    private static ClusterDetailInfo getTargetCluster(String repositoryId, ClusterNativeApi clusterNativeApi) {
        ClusterDetailInfo localCluster = clusterNativeApi.queryCurrentGroupClusterDetails();
        if (repositoryId.equals(localCluster.getStorageSystem().getStorageEsn())) {
            return localCluster;
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
}
