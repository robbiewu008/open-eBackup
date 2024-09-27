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

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseStorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import com.huawei.oceanprotect.repository.tapelibrary.service.MediaSetService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.repository.tape.TapeSetDetailResponse;
import openbackup.system.base.common.utils.asserts.PowerAssert;

import org.springframework.beans.BeanUtils;
import org.springframework.stereotype.Component;

/**
 * 磁带库存储库协议的策略类
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/14
 **/
@Component("tapeRepositoryStrategy")
public class TapeRepositoryStrategy implements RepositoryStrategy {
    private final MediaSetService mediaSetService;

    public TapeRepositoryStrategy(MediaSetService mediaSetService) {
        this.mediaSetService = mediaSetService;
    }

    @Override
    public Authentication getAuthentication(String repositoryId) {
        // 磁带库无需认证
        final Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.NO_AUTH);
        return authentication;
    }

    @Override
    public Endpoint getEndpoint(String repositoryId) {
        // 磁带库无连接信息
        return new Endpoint();
    }

    @Override
    public StorageRepository getRepository(BaseStorageRepository baseRepository) {
        final StorageRepository storageRepository = new StorageRepository();
        BeanUtils.copyProperties(baseRepository, storageRepository);
        storageRepository.setLocal(Boolean.FALSE);
        storageRepository.setAuth(this.getAuthentication(baseRepository.getId()));
        final TapeSetDetailResponse tapeSetDetail = mediaSetService.getTapeSetDetail(baseRepository.getId());
        PowerAssert.notNull(tapeSetDetail,
            () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "MediaSet not exist"));
        fillStorageRepositoryEndpoint(storageRepository, tapeSetDetail);
        storageRepository.setPath(tapeSetDetail.getMediaSetName());
        storageRepository.setProtocol(RepositoryProtocolEnum.TAPE.getProtocol());
        return storageRepository;
    }

    private void fillStorageRepositoryEndpoint(StorageRepository storageRepository,
        TapeSetDetailResponse tapeSetDetail) {
        Endpoint endpoint = new Endpoint();
        endpoint.setId(tapeSetDetail.getMediaSetId());
        endpoint.setIp(tapeSetDetail.getMediaSetName());
        storageRepository.setEndpoint(endpoint);
    }
}
