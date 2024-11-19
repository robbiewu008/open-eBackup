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
package openbackup.data.access.framework.backup.service.impl;

import static com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants.STORAGE_INFO;

import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.backup.dto.StorageInfoDto;
import openbackup.data.access.framework.protection.service.repository.RepositoryStrategyManager;
import openbackup.data.access.framework.protection.service.repository.strategies.RepositoryStrategy;
import openbackup.data.protection.access.provider.sdk.backup.v2.StorageRepositoryCreateService;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseStorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.repository.model.BackupUnitVo;

import org.springframework.stereotype.Service;

import java.math.BigDecimal;
import java.math.RoundingMode;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * 功能描述
 *
 */
@Service
@AllArgsConstructor
@Slf4j
public class StorageRepositoryCreateServiceImpl implements StorageRepositoryCreateService {
    private final StorageUnitService storageUnitService;

    private final BackupStorageApi backupStorageApi;

    private final RepositoryStrategyManager repositoryStrategyManager;

    @Override
    public StorageRepository createRepositoryByStorageUnit(String storageUnitId) {
        StorageUnitVo storageUnitVo = storageUnitService.getStorageUnitById(storageUnitId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Storage unit not exist"));
        return buildStorageRepositoryFromStorageUnitVo(storageUnitVo);
    }

    private StorageRepository buildStorageRepositoryFromStorageUnitVo(StorageUnitVo storageUnitVo) {
        log.info("Build repository from storage unit({})", storageUnitVo.getId());
        RepositoryStrategy repositoryStrategy =
            repositoryStrategyManager.getStrategy(RepositoryProtocolEnum.NATIVE_NFS);
        BaseStorageRepository repository = new BaseStorageRepository();
        repository.setType(RepositoryTypeEnum.DATA.getType());
        repository.setId(storageUnitVo.getDeviceId());
        if (VerifyUtil.isEmpty(repository.getExtendInfo())) {
            repository.setExtendInfo(new HashMap<>());
        }
        Map<String, Object> extendInfo = repository.getExtendInfo();
        extendInfo.put(STORAGE_INFO, buildStorageInfo(storageUnitVo));
        extendInfo.put(StorageRepository.CAPACITY_AVAILABLE, capacityLimitExceeded(storageUnitVo));
        extendInfo.put(StorageRepository.REPOSITORIES_KEY_ENS, storageUnitVo.getDeviceId());
        return repositoryStrategy.getRepository(repository);
    }

    private static boolean capacityLimitExceeded(StorageUnitVo storageUnitVo) {
        // 剩余可用容量是否大于0
        return storageUnitVo.getTotalCapacity()
            .multiply(new BigDecimal(storageUnitVo.getThreshold()))
            .divide(new BigDecimal(IsmNumberConstant.HUNDRED), 2, RoundingMode.DOWN)
            .subtract(storageUnitVo.getUsedCapacity())
            .compareTo(BigDecimal.ZERO) > 0;
    }

    private StorageInfoDto buildStorageInfo(StorageUnitVo storageUnitVo) {
        StorageInfoDto storageInfoDto = new StorageInfoDto();
        storageInfoDto.setStorageDevice(storageUnitVo.getDeviceId());
        storageInfoDto.setStoragePool(storageUnitVo.getPoolId());
        return storageInfoDto;
    }

    @Override
    public List<StorageRepository> createRepositoryByStorageUnitGroup(String groupId) {
        return backupStorageApi.getDetail(groupId)
            .getUnitList()
            .stream()
            .map(BackupUnitVo::toStorageUnitVo)
            .map(this::buildStorageRepositoryFromStorageUnitVo)
            .collect(Collectors.toList());
    }
}
