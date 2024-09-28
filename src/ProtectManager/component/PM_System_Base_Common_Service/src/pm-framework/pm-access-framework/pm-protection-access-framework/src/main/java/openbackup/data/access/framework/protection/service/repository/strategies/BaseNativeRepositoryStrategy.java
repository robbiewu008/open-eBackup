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
package openbackup.data.access.framework.protection.service.repository.strategies;

import static com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants.STORAGE_INFO;

import com.huawei.oceanprotect.base.cluster.sdk.entity.TargetCluster;
import com.huawei.oceanprotect.base.cluster.sdk.service.BackupClusterUnitService;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterQueryService;
import com.huawei.oceanprotect.base.cluster.sdk.util.IpUtil;
import com.huawei.oceanprotect.client.resource.manager.utils.VerifyUtil;
import openbackup.data.access.framework.backup.dto.StorageInfoDto;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseStorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.common.model.repository.StoragePool;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.TargetClusterRequestParm;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.session.IStorageDeviceRepository;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.StorageDevice;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.StoragePoolRestApi;
import openbackup.system.base.sdk.repository.model.BackupClusterVo;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.OptionalInt;

/**
 * 本地存储C库协议的策略基类
 *
 **/
@Slf4j
public class BaseNativeRepositoryStrategy {
    /**
     * 本地集群本地api
     */
    protected final ClusterNativeApi clusterNativeApi;

    private final IStorageDeviceRepository repository;

    @Autowired
    private BackupClusterUnitService backupClusterUnitService;

    @Autowired
    private EncryptorService encryptorService;

    @Autowired
    private StoragePoolRestApi storagePoolRestApi;

    @Autowired
    private ClusterQueryService clusterQueryService;

    @Value("${repository.storage.port}")
    private int storagePort;

    public BaseNativeRepositoryStrategy(ClusterNativeApi clusterNativeApi, IStorageDeviceRepository repository) {
        this.clusterNativeApi = clusterNativeApi;
        this.repository = repository;
    }

    /**
     * 构造本地存储的认证信息
     *
     * @return 统一认证信息对象{@code Authentication}
     */
    protected Authentication buildAuthentication() {
        StorageDevice device = repository.findLocalStorage(true);
        Authentication newAuthInfo = new Authentication();
        newAuthInfo.setAuthKey(device.getUserName());
        newAuthInfo.setAuthPwd(device.getPassword());
        newAuthInfo.setAuthType(Authentication.APP_PASSWORD);
        return newAuthInfo;
    }

    /**
     * 构造本地存储的认证信息
     *
     * @param esn 存储ESN
     * @return 统一认证信息对象{@code Authentication}
     */
    protected Authentication buildAuthentication(String esn) {
        BackupClusterVo backupClusterVo = backupClusterUnitService.queryBackupClusterByEsn(esn);
        List<Integer> clusterIdList = Lists.newArrayList(backupClusterVo.getClusterId());
        List<ClusterDetailInfo> clusterDetailInfos =
            clusterNativeApi.queryTargetClusterListDetails(new TargetClusterRequestParm(clusterIdList));
        if (CollectionUtils.isEmpty(clusterDetailInfos)) {
            log.error("clusterDetailInfos is null. esn is {}", esn);
            return new Authentication();
        }
        Authentication newAuthInfo = new Authentication();
        newAuthInfo.setAuthKey(backupClusterVo.getUsername());
        newAuthInfo.setAuthPwd(encryptorService.decrypt(clusterDetailInfos.get(0).getStorageSystem().getPassword()));
        newAuthInfo.setAuthType(Authentication.APP_PASSWORD);
        return newAuthInfo;
    }

    /**
     * 构造本地存储库连接信息
     *
     * @param repositoryId 存储库信息
     * @return 统一连接信息对象{@code Endpoint}
     */
    protected Endpoint buildEndPoint(String repositoryId) {
        String ip;
        int port;
        if (StringUtils.isEmpty(repositoryId)) {
            ip = String.join(",", clusterNativeApi.queryCurrentGroupManageIpList());
            port = storagePort;
        } else {
            BackupClusterVo backupClusterVo = backupClusterUnitService.queryBackupClusterByEsn(repositoryId);
            ip = String.join(",", IpUtil.convertIpStringToList(backupClusterVo.getClusterIp()));
            port = backupClusterVo.getPort();
        }
        return new Endpoint(repositoryId, ip, port);
    }

    /**
     * 获取本地存储库信息
     *
     * @param baseRepository 存储库基础信息
     * @return 存储库详细信息 {@code StorageRepository}
     */
    protected StorageRepository getNativeRepository(BaseStorageRepository baseRepository) {
        // repositoryId为对应存储的ESN
        Map<String, Object> extendInfo = baseRepository.getExtendInfo();
        // 升级场景
        if (!extendInfo.containsKey(STORAGE_INFO)) {
            buildUpgradeStorageInfo(extendInfo);
        }
        Object extendInfoObj = extendInfo.get(STORAGE_INFO);
        StorageInfoDto storageInfoDto = null;
        if (extendInfoObj instanceof StorageInfoDto) {
            storageInfoDto = (StorageInfoDto) extendInfoObj;
        } else {
            storageInfoDto = JsonUtil.read(JsonUtil.json(extendInfoObj), StorageInfoDto.class);
        }
        String repositoryId = storageInfoDto.getStorageDevice();
        final Endpoint endpoint = buildEndPoint(repositoryId);

        // 本地存储path和extendInfo为空
        StorageRepository storageRepository = new StorageRepository();
        BeanUtils.copyProperties(baseRepository, storageRepository);
        storageRepository.setLocal(Boolean.TRUE);
        storageRepository.setEndpoint(endpoint);

        final Authentication authentication =
            StringUtils.isEmpty(repositoryId) ? buildAuthentication() : buildAuthentication(repositoryId);
        storageRepository.setExtendAuth(authentication);
        addExtendInfo(storageRepository, repositoryId);
        if (!VerifyUtil.isEmpty(repositoryId)) {
            storageRepository.setLocalCluster(
                repositoryId.equals(storageRepository.getExtendInfo().get(StorageRepository.REPOSITORIES_KEY_ENS)));
        }
        return storageRepository;
    }

    private void buildUpgradeStorageInfo(Map<String, Object> extendInfo) {
        if (VerifyUtil.isEmpty(extendInfo.get("esn"))) {
            extendInfo.put("esn", clusterQueryService.getCurrentClusterEsn());
        }
        String deviceId = extendInfo.get("esn").toString();
        TargetCluster targetCluster = clusterQueryService.getTargetClusterByEsn(deviceId);
        StorageInfoDto updateStorageInfoDto = new StorageInfoDto();
        List<StoragePool> storagePools =
                storagePoolRestApi.getStoragePools(deviceId, targetCluster.getUsername()).getData();
        OptionalInt min = storagePools.stream().mapToInt(s -> Integer.parseInt(s.getId())).min();
        updateStorageInfoDto.setStoragePool(Integer.toString(min.getAsInt()));
        updateStorageInfoDto.setStorageDevice(deviceId);
        extendInfo.put(STORAGE_INFO, updateStorageInfoDto);
        log.info("build upgrade storage info finished. esn is {}", deviceId);
    }

    private void addExtendInfo(StorageRepository repository, String repositoryId) {
        Map<String, Object> extendInfo = Optional.ofNullable(repository.getExtendInfo()).orElse(new HashMap<>());
        extendInfo.put(StorageRepository.REPOSITORIES_KEY_ENS, StringUtils.isEmpty(repositoryId)
            ? clusterNativeApi.getCurrentClusterVoInfo().getStorageEsn() : repositoryId);
        repository.setExtendInfo(extendInfo);
    }
}
