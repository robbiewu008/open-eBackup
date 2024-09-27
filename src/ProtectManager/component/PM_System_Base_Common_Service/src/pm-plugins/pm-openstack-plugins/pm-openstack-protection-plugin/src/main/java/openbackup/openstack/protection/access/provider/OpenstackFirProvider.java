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
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import com.fasterxml.jackson.core.JsonProcessingException;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * OpenstackFirProvider
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-31
 */
@Component
public class OpenstackFirProvider extends AbstractVmFileLevelRestoreProvider implements RestoreProvider {
    private static final List<String> SUPPORT_GENERATE_TYPE =
        Arrays.asList(CopyGeneratedByEnum.BY_BACKUP.value(), CopyGeneratedByEnum.BY_REPLICATED.value());

    /**
     * 构造注入
     *
     * @param copyRestApi copyRestApi
     * @param resourceService resourceService
     * @param protectedResourceMonitorService protectedResourceMonitorService
     * @param flrService flrService
     * @param resourceSetApi resourceSetApi
     */
    public OpenstackFirProvider(CopyRestApi copyRestApi, ResourceService resourceService,
        ProtectedResourceMonitorService protectedResourceMonitorService, IvmFileLevelRestoreService flrService,
        ResourceSetApi resourceSetApi) {
        super(copyRestApi, resourceService, protectedResourceMonitorService, flrService, resourceSetApi);
    }

    @Override
    public boolean applicable(RestoreObject restoreObject) {
        if (restoreObject == null) {
            return false;
        }
        return ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.equalsSubType(restoreObject.getObjectType())
            && (SUPPORT_GENERATE_TYPE.contains(restoreObject.getCopyGeneratedBy()));
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
    protected boolean checkIfEnableStop(Copy copy) {
        return SUPPORT_GENERATE_TYPE
            .contains(copy == null ? CopyGeneratedByEnum.BY_BACKUP.value() : copy.getGeneratedBy());
    }

    @Override
    protected String buildSnapMetadata(JSONObject properties) {
        return CopyVolInfo.convert2IndexDiskInfos(properties);
    }
}
