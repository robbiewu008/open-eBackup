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

import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import com.fasterxml.jackson.core.JsonProcessingException;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.ProtectedResourceMonitorService;
import openbackup.cnware.protection.access.dto.CnwareVolInfo;
import openbackup.data.access.framework.copy.index.provider.AbstractVmFileLevelRestoreProvider;
import openbackup.data.access.framework.copy.index.service.IvmFileLevelRestoreService;
import openbackup.data.protection.access.provider.sdk.job.Task;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.RestoreObject;
import openbackup.data.protection.access.provider.sdk.restore.RestoreProvider;
import openbackup.data.protection.access.provider.sdk.restore.RestoreRequest;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * CnwareFirProvider CNware细粒度恢复
 *
 */
@Component
@Slf4j
public class CnwareFirProvider extends AbstractVmFileLevelRestoreProvider implements RestoreProvider {
    private static final List<String> SUPPORT_GENERATE_TYPE = Arrays.asList(CopyGeneratedByEnum.BY_BACKUP.value(),
        CopyGeneratedByEnum.BY_REPLICATED.value());

    /**
     * 构造注入
     *
     * @param copyRestApi copyRestApi
     * @param resourceService resourceService
     * @param protectedResourceMonitorService protectedResourceMonitorService
     * @param flrService flrService
     * @param resourceSetApi resourceSetApi
     */
    public CnwareFirProvider(CopyRestApi copyRestApi, ResourceService resourceService,
        ProtectedResourceMonitorService protectedResourceMonitorService, IvmFileLevelRestoreService flrService,
        ResourceSetApi resourceSetApi) {
        super(copyRestApi, resourceService, protectedResourceMonitorService, flrService, resourceSetApi);
    }

    /**
     * 检查是否支持停止任务
     *
     * @param copy 副本信息
     * @return true/false
     */
    @Override
    protected boolean checkIfEnableStop(Copy copy) {
        return SUPPORT_GENERATE_TYPE.contains(
            copy == null ? CopyGeneratedByEnum.BY_BACKUP.value() : copy.getGeneratedBy());
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

    /**
     * detect object applicable
     *
     * @param restoreObject object
     * @return detect result
     */
    @Override
    public boolean applicable(RestoreObject restoreObject) {
        if (restoreObject == null) {
            return false;
        }
        String resourceSubType = restoreObject.getObjectType();
        String generatedBy = restoreObject.getCopyGeneratedBy();
        return ResourceSubTypeEnum.CNWARE_VM.getType().equals(resourceSubType) && (SUPPORT_GENERATE_TYPE.contains(
            generatedBy));
    }

    /**
     * restore methods that need to be implemented by specific providers, This method is responsible for the business
     * logic of copy restore.
     *
     * @param restoreObject the restore object
     * @return the restore task
     */
    @Override
    public Task restore(RestoreObject restoreObject) {
        return doRestore(restoreObject);
    }

    /**
     * 生成恢复任务
     *
     * @param restoreRequest 恢复请求
     * @return 任务json
     * @throws JsonProcessingException json转换异常
     */
    @Override
    public String createRestoreTask(RestoreRequest restoreRequest) throws JsonProcessingException {
        return doCreateRestoreTask(restoreRequest);
    }
}
