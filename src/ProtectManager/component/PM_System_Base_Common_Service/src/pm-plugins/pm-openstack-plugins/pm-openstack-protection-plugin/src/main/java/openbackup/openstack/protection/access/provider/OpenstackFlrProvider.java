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

import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import com.google.common.collect.ImmutableList;
import com.fasterxml.jackson.core.JsonProcessingException;

import openbackup.access.framework.resource.service.ProtectedResourceMonitorService;
import openbackup.data.access.framework.copy.index.provider.AbstractVmFileLevelRestoreProvider;
import openbackup.data.access.framework.copy.index.service.IvmFileLevelRestoreService;
import openbackup.data.protection.access.provider.sdk.job.Task;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.RestoreObject;
import openbackup.data.protection.access.provider.sdk.restore.RestoreProvider;
import openbackup.data.protection.access.provider.sdk.restore.RestoreRequest;
import openbackup.openstack.protection.access.dto.CopyVolInfo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * OpenstackFirProvider
 *
 */
@Component
public class OpenstackFlrProvider extends AbstractVmFileLevelRestoreProvider implements RestoreProvider {
    private static final List<String> SUPPORTED_SUB_TYPE = ImmutableList.of(
            ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType()
    );

    /**
     * 构造注入
     *
     * @param copyRestApi copyRestApi
     * @param resourceService resourceService
     * @param protectedResourceMonitorService protectedResourceMonitorService
     * @param flrService flrService
     * @param resourceSetApi resourceSetApi
     */
    public OpenstackFlrProvider(CopyRestApi copyRestApi, ResourceService resourceService,
                                ProtectedResourceMonitorService protectedResourceMonitorService,
                                IvmFileLevelRestoreService flrService,
                                ResourceSetApi resourceSetApi) {
        super(copyRestApi, resourceService, protectedResourceMonitorService, flrService, resourceSetApi);
    }

    @Override
    public boolean applicable(RestoreObject restoreObject) {
        if (restoreObject == null) {
            return false;
        }
        String resourceSubType = restoreObject.getObjectType();
        return SUPPORTED_SUB_TYPE.contains(resourceSubType) && checkGeneratedType(restoreObject);
    }

    @Override
    public Task restore(RestoreObject restoreObject) {
        return doRestore(restoreObject);
    }

    @Override
    public String createRestoreTask(RestoreRequest restoreRequest) throws JsonProcessingException {
        return doCreateRestoreTask(restoreRequest);
    }

    @Override
    protected String buildSnapMetadata(JSONObject properties) {
        return CopyVolInfo.convert2IndexDiskInfos(properties);
    }
}
