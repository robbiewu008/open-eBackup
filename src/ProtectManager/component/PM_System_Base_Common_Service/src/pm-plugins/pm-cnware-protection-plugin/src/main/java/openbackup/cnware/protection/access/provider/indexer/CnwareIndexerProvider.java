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
package openbackup.cnware.protection.access.provider.indexer;

import openbackup.cnware.protection.access.dto.CnwareVolInfo;
import openbackup.data.access.framework.copy.index.provider.AbstractVmIndexerProvider;
import openbackup.data.protection.access.provider.sdk.copy.CopyBo;
import openbackup.data.protection.access.provider.sdk.index.IndexerProvider;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.security.EncryptorUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.storage.StorageRestClient;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * CnwareIndexerProvider CNware索引
 *
 */
@Component
@Slf4j
public class CnwareIndexerProvider extends AbstractVmIndexerProvider implements IndexerProvider {
    /**
     * 构造函数注入
     *
     * @param storageRestClient storageRestClient
     * @param copyRestApi copyRestApi
     * @param notifyManager notifyManager
     * @param encryptorUtil encryptorUtil
     */
    public CnwareIndexerProvider(StorageRestClient storageRestClient, CopyRestApi copyRestApi,
        NotifyManager notifyManager, EncryptorUtil encryptorUtil) {
        super(storageRestClient, copyRestApi, notifyManager, encryptorUtil);
    }

    /**
     * provider过滤器，过滤条件接口
     *
     * @param resourceSubType 资源子类型
     * @return 检测结果true or false
     */
    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.CNWARE_VM.equalsSubType(resourceSubType);
    }

    /**
     * 生成索引文件
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

    /**
     * 生成快照元数据信息
     *
     * @param properties 配置信息
     * @return 元数据信息
     */
    @Override
    protected String buildSnapMetadata(JSONObject properties) {
        return CnwareVolInfo.convert2IndexDiskInfos(properties);
    }
}
