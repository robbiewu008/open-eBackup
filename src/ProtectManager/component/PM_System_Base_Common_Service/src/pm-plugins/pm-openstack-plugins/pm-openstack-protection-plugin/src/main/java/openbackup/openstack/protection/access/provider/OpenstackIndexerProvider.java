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
package openbackup.openstack.protection.access.provider;

import openbackup.openstack.protection.access.dto.CopyVolInfo;

import openbackup.data.access.framework.copy.index.provider.AbstractVmIndexerProvider;
import openbackup.data.protection.access.provider.sdk.copy.CopyBo;
import openbackup.data.protection.access.provider.sdk.index.IndexerProvider;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.security.EncryptorUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.storage.StorageRestClient;

import org.springframework.stereotype.Component;

/**
 * OpenstackIndexerProvider
 *
 */
@Component
public class OpenstackIndexerProvider extends AbstractVmIndexerProvider implements IndexerProvider {
    /**
     * 构造注入
     *
     * @param storageRestClient storageRestClient
     * @param copyRestApi copyRestApi
     * @param notifyManager notifyManager
     * @param encryptorUtil encryptorUtil
     */
    public OpenstackIndexerProvider(StorageRestClient storageRestClient, CopyRestApi copyRestApi,
        NotifyManager notifyManager, EncryptorUtil encryptorUtil) {
        super(storageRestClient, copyRestApi, notifyManager, encryptorUtil);
    }

    /**
     * 生成rfi文件
     *
     * @param requestId 请求id
     * @param copy 副本信息
     * @return 是否成功
     */
    @Override
    public boolean generateIndexFile(String requestId, CopyBo copy) {
        sendScanRequest(requestId, copy);
        return true;
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.equalsSubType(resourceSubType);
    }

    @Override
    protected String buildSnapMetadata(JSONObject properties) {
        return CopyVolInfo.convert2IndexDiskInfos(properties);
    }
}