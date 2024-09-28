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

import static com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants.REPLICATION_TARGET_MODE;
import static com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants.STORAGE_ID;
import static com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants.STORAGE_INFO;

import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;
import openbackup.data.access.framework.backup.constant.BackupConstant;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.replication.ReplicationService;
import openbackup.data.access.framework.protectobject.model.ProtectionExecuteCheckReq;
import openbackup.data.access.framework.protectobject.service.ProjectObjectService;
import openbackup.data.protection.access.provider.sdk.protection.ProtectObjectProvider;
import openbackup.data.protection.access.provider.sdk.protection.model.CheckProtectObjectDto;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants;
import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyType;
import com.huawei.oceanprotect.sla.sdk.enums.ReplicationMode;
import openbackup.system.base.bean.PolicyStorageInfo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.repository.model.BackupUnitVo;
import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.databind.JsonNode;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

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
    public void checkExistCopiesLocationBeforeProtect(String slaId, String resourceId) {
        List<PolicyDto> policyList = Optional.ofNullable(slaQueryService.querySlaById(slaId))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Cant find sla: " + slaId))
            .getPolicyList();
        checkReplicationPolicyBeforeProtect(policyList, resourceId);
        PolicyDto policyDto = getPolicyDto(policyList);
        if (!policyDto.getExtParameters().has(BackupConstant.BACKUP_EXT_PARAM_STORAGE_INFO_KEY)) {
            log.info("Sla policy doesn't specify storage_info. No need to check. Resource id: {}", resourceId);
            return;
        }
        PolicyStorageInfo storageInfo = JSONObject.DEFAULT_OBJ_MAPPER.convertValue(
            policyDto.getExtParameters().get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_INFO_KEY),
            PolicyStorageInfo.class);
        String storageType = storageInfo.getStorageType();
        if (VerifyUtil.isEmpty(storageType)) {
            log.info("Sla policy doesn't specify storage_info. No need to check. Resource id: {}", resourceId);
            return;
        }
        checkBeforeManualReplication(storageType, storageInfo.getStorageId(), resourceId);
    }

    private void checkReplicationPolicyBeforeProtect(List<PolicyDto> policyList, String resourceId) {
        policyList.stream().filter(policy -> PolicyType.REPLICATION.equals(policy.getType())).forEach(policyDto -> {
            int replicationMode =
                policyDto.getIntegerFormExtParameters(REPLICATION_TARGET_MODE, ReplicationMode.EXTRA.getValue());
            // 策略未指定复制目标单元，则跳过校验
            if (!policyDto.getExtParameters().has(STORAGE_INFO)
                || !policyDto.getExtParameters().get(STORAGE_INFO).has(STORAGE_ID)) {
                log.info("Remote unit or group not specified. All units available.");
                return;
            }
            JsonNode storageInfo = policyDto.getExtParameters().get(ExtParamsConstants.STORAGE_INFO);
            String storageId = storageInfo.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_ID_KEY).textValue();
            String storageType = storageInfo.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_TYPE_KEY).textValue();
            if (ReplicationMode.EXTRA.getValue() == replicationMode) { // 跨域复制
                replicationService.checkManualRep(
                    Integer.parseInt(policyDto.getExtParameters().get("external_system_id").textValue()), storageType,
                    storageId, resourceId);
            } else {
                checkBeforeManualReplication(storageType, storageId, resourceId);
            }
        });
    }

    private PolicyDto getPolicyDto(List<PolicyDto> policyList) {
        return policyList.stream()
            .filter(policy -> PolicyType.BACKUP.equals(policy.getType()))
            .findAny()
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Sla doesn't have any backup policy"));
    }

    @Override
    public void checkBeforeManualReplication(String storageType, String storageId, String resourceId) {
        Set<String> unitIdSetInSla;
        if (BackupConstant.BACKUP_EXT_PARAM_STORAGE_UNIT_GROUP_VALUE.equals(storageType)) {
            unitIdSetInSla = backupStorageApi.getDetail(storageId)
                    .getUnitList()
                    .stream()
                    .map(BackupUnitVo::getUnitId)
                    .collect(Collectors.toSet());
        } else {
            unitIdSetInSla = new HashSet<>();
            unitIdSetInSla.add(storageId);
        }
        List<StorageUnitVo> unitVos = storageUnitService.getAllAvailableStorageUnitForResource(resourceId);
        if (VerifyUtil.isEmpty(unitVos)
                || unitVos.stream().noneMatch(storageUnitVo -> unitIdSetInSla.contains(storageUnitVo.getId()))) {
            log.error("Current sla doesn't contains available storage unit. Resource id :{}", resourceId);
            throw new LegoCheckedException(CommonErrorCode.SAME_DEVICE_HAVING_SAME_RESOURCE_COPY,
                    unitVos.stream().map(StorageUnitVo::getName).toArray(String[]::new),
                    "The new SLA lacks storage units for existing copies");
        }
    }
}
