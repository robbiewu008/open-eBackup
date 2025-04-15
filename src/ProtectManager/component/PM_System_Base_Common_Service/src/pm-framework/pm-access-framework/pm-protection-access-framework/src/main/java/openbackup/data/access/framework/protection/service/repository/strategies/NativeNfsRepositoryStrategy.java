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

import com.huawei.oceanprotect.system.base.sdk.devicemanager.ability.session.IStorageDeviceRepository;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.BaseStorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;

import org.springframework.stereotype.Component;

/**
 * 本地存储库NFS协议的策略类
 *
 **/
@Component("nativeNfsRepositoryStrategy")
public class NativeNfsRepositoryStrategy extends BaseNativeRepositoryStrategy implements RepositoryStrategy {
    public NativeNfsRepositoryStrategy(ClusterNativeApi clusterNativeApi, IStorageDeviceRepository repository) {
        super(clusterNativeApi, repository);
    }

    @Override
    public Authentication getAuthentication(String repositoryId) {
        // 本地存储 用户名/密码字段进行认证
        return super.buildAuthentication();
    }

    @Override
    public Endpoint getEndpoint(String repositoryId) {
        return super.buildEndPoint(repositoryId);
    }

    @Override
    public StorageRepository getRepository(BaseStorageRepository baseRepository) {
        final StorageRepository nativeRepository = super.getNativeRepository(baseRepository);
        nativeRepository.setProtocol(RepositoryProtocolEnum.NFS.getProtocol());
        return nativeRepository;
    }
}
