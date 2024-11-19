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
package openbackup.database.base.plugin.provider.sla;

import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.dto.UpdateSlaCommand;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyType;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.JsonNode;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.ImmutableSet;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.resource.ResourceExtendInfoService;
import openbackup.data.protection.access.provider.sdk.resource.model.ProtectedResourceExtendInfo;
import openbackup.data.protection.access.provider.sdk.sla.SlaValidateProvider;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * GeneralDb的SlaValidateProvider实现
 *
 */
@Component
@Slf4j
public class GeneralDbSlaValidateProvider implements SlaValidateProvider {
    private static final Set<String> DEFAULT_BACKUP_SUPPORT_ACTION_SET = ImmutableSet.of(
            PolicyAction.CUMULATIVE_INCREMENT.getAction(), PolicyAction.LOG.getAction(),
            PolicyAction.DIFFERENCE_INCREMENT.getAction(), PolicyAction.FULL.getAction(),
            PolicyAction.PERMANENT_INCREMENT.getAction(), PolicyAction.SNAPSHOT.getAction());

    private static final Map<String, String> SLA_POLICY_ACTION_LABEL = ImmutableMap.of(
            PolicyAction.FULL.getAction(), "common_full_backup_label",
            PolicyAction.LOG.getAction(), "common_log_backup_label",
            PolicyAction.DIFFERENCE_INCREMENT.getAction(), "common_incremental_backup_label",
            PolicyAction.CUMULATIVE_INCREMENT.getAction(), "common_diff_backup_label",
            PolicyAction.PERMANENT_INCREMENT.getAction(), "common_permanent_backup_label",
            PolicyAction.SNAPSHOT.getAction(), "common_production_snapshot_label");

    private static final int DATABASE_TYPE_NUM = 2;

    private final ProtectObjectRestApi protectObjectRestApi;

    private final ResourceExtendInfoService resourceExtendInfoService;

    public GeneralDbSlaValidateProvider(ProtectObjectRestApi protectObjectRestApi,
            ResourceExtendInfoService resourceExtendInfoService) {
        this.protectObjectRestApi = protectObjectRestApi;
        this.resourceExtendInfoService = resourceExtendInfoService;
    }

    @Override
    public SlaValidateConfig getConfig() {
        SlaValidateConfig slaValidateConfig = new SlaValidateConfig();
        slaValidateConfig.getSpecificationConfig()
                .setLimit(PolicyLimitConfig.of(PolicyAction.FULL, SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
                .setLimit(PolicyLimitConfig.of(PolicyAction.CUMULATIVE_INCREMENT,
                        SlaConstants.CUMULATIVE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
                .setLimit(PolicyLimitConfig.of(PolicyAction.DIFFERENCE_INCREMENT,
                        SlaConstants.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
                .setLimit(PolicyLimitConfig.of(PolicyAction.LOG, SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
                .setLimit(PolicyLimitConfig.of(PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT))
                .setLimit(PolicyLimitConfig.of(PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT));
        return slaValidateConfig;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.GENERAL_DB.equalsSubType(object);
    }

    @Override
    public void validateSLA(SlaBase slaBase) {
        checkResourcesSupportSla(slaBase);
    }

    private void checkResourcesSupportSla(SlaBase slaBase) {
        if (!(slaBase instanceof UpdateSlaCommand)) {
            log.info("Current operation of this sla: {} is not modification, no need to check.",
                    slaBase.getPolicyList().get(0).getUuid());
            return;
        }
        UpdateSlaCommand updateSlaCommand = (UpdateSlaCommand) slaBase;
        List<String> policyActions = updateSlaCommand.getPolicyList().stream()
                .filter(policyDto -> PolicyType.BACKUP.equals(policyDto.getType()))
                .map(policyDto -> policyDto.getAction().getAction()).collect(Collectors.toList());
        List<ProtectedObjectInfo> items;
        Set<String> filteredDatabaseTypes = new HashSet<>();
        int page = 0;
        do {
            BasePage<ProtectedObjectInfo> data = protectObjectRestApi.pageQueryProtectObject(updateSlaCommand.getUuid(),
                    page, IsmNumberConstant.HUNDRED);
            items = data.getItems();
            if (VerifyUtil.isEmpty(items)) {
                log.info("No any resource is bound to this sla: {}, no need to check.",
                        updateSlaCommand.getUuid());
                return;
            }
            List<String> relatedResourceIds = items.stream().map(ProtectedObjectInfo::getResourceId)
                    .collect(Collectors.toList());
            List<ProtectedResourceExtendInfo> extendInfos = resourceExtendInfoService
                    .queryExtendInfo(relatedResourceIds, "scriptConf");
            for (ProtectedResourceExtendInfo extendInfo : extendInfos) {
                JsonNode scriptConfNode = JsonUtil.read(extendInfo.getValue(), new TypeReference<JsonNode>() {
                });
                String databaseType = scriptConfNode.get("databaseType").textValue();
                if (!filteredDatabaseTypes.add(databaseType)) {
                    continue;
                }
                checkSupportContainsSla(databaseType, policyActions, scriptConfNode);
            }
        } while (items.size() >= IsmNumberConstant.HUNDRED && filteredDatabaseTypes.size() < DATABASE_TYPE_NUM);
    }

    private void checkSupportContainsSla(String databaseType, List<String> policyActions, JsonNode scriptConfNode) {
        Set<String> supportBackupTypes = getSupportBackupTypes(scriptConfNode);
        for (String policyAction : policyActions) {
            if (!supportBackupTypes.contains(policyAction)) {
                throw new LegoCheckedException(CommonErrorCode.SLA_NOT_SUPPORT_BACKUP_POLICY,
                        new String[]{databaseType, SLA_POLICY_ACTION_LABEL.get(policyAction)},
                        "Resource: " + databaseType + " bound to this sla do not support type:" + policyAction);
            }
        }
    }

    private Set<String> getSupportBackupTypes(JsonNode scriptConfNode) {
        if (VerifyUtil.isEmpty(scriptConfNode)
                || VerifyUtil.isEmpty(scriptConfNode.get("backup"))
                || VerifyUtil.isEmpty(scriptConfNode.get("backup").get("support"))) {
            return DEFAULT_BACKUP_SUPPORT_ACTION_SET;
        }
        return new HashSet<>(scriptConfNode.get("backup").get("support").findValuesAsText("backupType"));
    }
}
