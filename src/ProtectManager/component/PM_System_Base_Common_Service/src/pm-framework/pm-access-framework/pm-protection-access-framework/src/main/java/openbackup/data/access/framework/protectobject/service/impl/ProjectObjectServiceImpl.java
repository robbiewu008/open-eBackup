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
package openbackup.data.access.framework.protectobject.service.impl;

import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;
import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.replication.ReplicationService;
import openbackup.data.access.framework.protectobject.model.ProtectionExecuteCheckReq;
import openbackup.data.access.framework.protectobject.service.ProjectObjectService;
import openbackup.data.protection.access.provider.sdk.protection.ProtectObjectProvider;
import openbackup.data.protection.access.provider.sdk.protection.model.CheckProtectObjectDto;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.util.EnumUtil;

import org.springframework.stereotype.Service;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * ProjectObjectService实现类
 *
 */
@Service
@AllArgsConstructor
@Slf4j
public class ProjectObjectServiceImpl implements ProjectObjectService {
    private ResourceService resourceService;

    private ProviderManager providerManager;

    private final CopyRestApi copyRestApi;

    private final SlaQueryService slaQueryService;

    private final BackupStorageApi backupStorageApi;

    private final StorageUnitService storageUnitService;

    private final ReplicationService replicationService;

    @Override
    public void checkProtectObject(ProtectionExecuteCheckReq protectionExecuteCheckReq) {
        List<String> resourceIds = protectionExecuteCheckReq.getResourceIds();
        if (VerifyUtil.isEmpty(resourceIds)) {
            return;
        }
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("uuid", resourceIds);
        List<ProtectedResource> resourceList = resourceService.basicQuery(true, 0, resourceIds.size(), conditions)
                .getRecords();
        for (ProtectedResource resource : resourceList) {
            ProtectObjectProvider provider = providerManager.findProviderOrDefault(ProtectObjectProvider.class,
                    resource.getSubType(), null);
            if (provider == null) {
                continue;
            }
            CheckProtectObjectDto checkProtectObjectDto = initCheckProtectObjectDto(protectionExecuteCheckReq);
            checkProtectObjectDto.setProtectedResource(resource);
            ProtectionExecuteCheckReq.ProtectionPhaseType type = getProtectionPhaseType(
                    protectionExecuteCheckReq.getType());
            if (type == null) {
                return;
            }
            switch (type) {
                case BEFORE_CREATE:
                    provider.beforeCreate(checkProtectObjectDto);
                    break;
                case BEFORE_UPDATE:
                    provider.beforeUpdate(checkProtectObjectDto);
                    break;
                case FAILED_ON_CREATE_OR_UPDATE:
                    provider.failedOnCreateOrUpdate(checkProtectObjectDto);
                    break;
                case REMOVE:
                    provider.remove(resource);
                    break;
                default:
                    break;
            }
        }
    }

    private ProtectionExecuteCheckReq.ProtectionPhaseType getProtectionPhaseType(String type) {
        return EnumUtil.get(ProtectionExecuteCheckReq.ProtectionPhaseType.class,
                ProtectionExecuteCheckReq.ProtectionPhaseType::getValue, type, false, true);
    }

    private CheckProtectObjectDto initCheckProtectObjectDto(ProtectionExecuteCheckReq protectionExecuteCheckReq) {
        CheckProtectObjectDto checkProtectObjectDto = new CheckProtectObjectDto();
        checkProtectObjectDto.setSlaId(protectionExecuteCheckReq.getSlaId());
        checkProtectObjectDto.setExtParameters(protectionExecuteCheckReq.getExtParameters());
        checkProtectObjectDto.setJobId(protectionExecuteCheckReq.getJobId());
        checkProtectObjectDto.setOriginSlaId(protectionExecuteCheckReq.getOriginSlaId());
        return checkProtectObjectDto;
    }

    @Override
    public void checkBeforeManualReplication(String storageType, String storageId, String resourceId) {
        log.debug("No need to checkBeforeManualReplication");
    }
}
