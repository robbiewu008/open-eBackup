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
package openbackup.oceanbase.sla;

import openbackup.data.protection.access.provider.sdk.sla.SlaValidateProvider;
import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.dto.UpdateSlaCommand;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyType;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.ImmutableSet;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * OceanBase的sla校验器provider
 *
 * @author c00826511
 * @since 2023-07-22
 */
@Component
@Slf4j
public class OceanBaseSlaValidatorProvider implements SlaValidateProvider {
    private static final Set<String> DEFAULT_BACKUP_NEED_CHECK_ACTION_SET = ImmutableSet.of(
        PolicyAction.LOG.getAction(), PolicyAction.CUMULATIVE_INCREMENT.getAction(),
        PolicyAction.DIFFERENCE_INCREMENT.getAction(), PolicyAction.PERMANENT_INCREMENT.getAction());

    private static final Set<String> CLUSTER_BACKUP_NOT_SUPPORT_ACTION_SET = ImmutableSet.of(
        PolicyAction.CUMULATIVE_INCREMENT.getAction(), PolicyAction.PERMANENT_INCREMENT.getAction());

    private static final Map<String, String> SLA_POLICY_ACTION_LABEL = ImmutableMap.of(PolicyAction.FULL.getAction(),
        "common_full_backup_label", PolicyAction.LOG.getAction(), "common_log_backup_label",
        PolicyAction.DIFFERENCE_INCREMENT.getAction(), "common_incremental_backup_label",
        PolicyAction.CUMULATIVE_INCREMENT.getAction(), "common_diff_backup_label",
        PolicyAction.PERMANENT_INCREMENT.getAction(), "common_permanent_backup_label",
        PolicyAction.SNAPSHOT.getAction(), "common_production_snapshot_label");

    private ProtectObjectRestApi protectObjectRestApi;

    /**
     * 构造器
     *
     * @param protectObjectRestApi protectObjectRestApi
     */
    public OceanBaseSlaValidatorProvider(ProtectObjectRestApi protectObjectRestApi) {
        this.protectObjectRestApi = protectObjectRestApi;
    }

    /**
     * detect object applicable
     *
     * @param resourceSubType resourceSubType
     * @return detect result
     */
    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.OCEAN_BASE_CLUSTER.getType().equals(resourceSubType)
            || ResourceSubTypeEnum.OCEAN_BASE_TENANT.getType().equals(resourceSubType);
    }

    /**
     * 获取SLA校验器配置
     * 1.指定支持的策略规格配置
     * 2.根据业务情况进行配置规则的开关，用于跳过框架中某些校验规则
     *
     * @return SlaValidateConfig 校验配置类
     */
    @Override
    public SlaValidateConfig getConfig() {
        // 1.5.0版本前端设置的difference_increment字段为累积增量备份，不是差异增量备份
        SlaValidateConfig sla = new SlaValidateConfig();
        sla.getSpecificationConfig()
            .setLimit(PolicyLimitConfig.of(PolicyAction.FULL, SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.DIFFERENCE_INCREMENT,
                SlaConstants.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.CUMULATIVE_INCREMENT,
                SlaConstants.CUMULATIVE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.LOG, SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT));
        log.info("end getConfig in OceanBaseValidatorProvider");
        return sla;
    }

    @Override
    public void validateSLA(SlaBase slaBase) {
        if (!(slaBase instanceof UpdateSlaCommand)) {
            log.info("Current operation of this sla: {} is not modification, no need to check.",
                slaBase.getPolicyList().get(0).getUuid());
            return;
        }
        UpdateSlaCommand updateSlaCommand = (UpdateSlaCommand) slaBase;
        List<String> policyActions = updateSlaCommand.getPolicyList()
            .stream()
            .filter(policyDto -> PolicyType.BACKUP.equals(policyDto.getType()))
            .map(policyDto -> policyDto.getAction().getAction())
            .collect(Collectors.toList());
        boolean isContainsNeedCheckAction = policyActions.stream()
            .anyMatch(DEFAULT_BACKUP_NEED_CHECK_ACTION_SET::contains);
        if (!isContainsNeedCheckAction) {
            log.info("Do not need to check the action of the sla policy.");
            return;
        }
        List<ProtectedObjectInfo> items;
        int page = 0;
        do {
            BasePage<ProtectedObjectInfo> data = protectObjectRestApi.pageQueryProtectObject(updateSlaCommand.getUuid(),
                page, IsmNumberConstant.HUNDRED);
            items = data.getItems();
            if (VerifyUtil.isEmpty(items)) {
                log.info("No any resource is bound to this sla: {}, no need to check.", updateSlaCommand.getUuid());
                return;
            }
            for (ProtectedObjectInfo protectedObjectInfo : items) {
                checkSupportContainsSla(policyActions, protectedObjectInfo);
            }
            page++;
        } while (items.size() >= IsmNumberConstant.HUNDRED);
    }

    private void checkSupportContainsSla(List<String> policyActions, ProtectedObjectInfo protectedObjectInfo) {
        String subType = protectedObjectInfo.getSubType();
        if (ResourceSubTypeEnum.OCEAN_BASE_CLUSTER.equalsSubType(subType)) {
            for (String policyAction : policyActions) {
                if (CLUSTER_BACKUP_NOT_SUPPORT_ACTION_SET.contains(policyAction)) {
                    throw new LegoCheckedException(CommonErrorCode.SLA_NOT_SUPPORT_BACKUP_POLICY,
                        new String[] {subType, SLA_POLICY_ACTION_LABEL.get(policyAction)},
                        "Resource: " + subType + " bound to this sla do not support type:" + policyAction);
                }
            }
        } else if (ResourceSubTypeEnum.OCEAN_BASE_TENANT.equalsSubType(subType)) {
            for (String policyAction : policyActions) {
                if (DEFAULT_BACKUP_NEED_CHECK_ACTION_SET.contains(policyAction)) {
                    throw new LegoCheckedException(CommonErrorCode.SLA_NOT_SUPPORT_BACKUP_POLICY,
                        new String[] {subType, SLA_POLICY_ACTION_LABEL.get(policyAction)},
                        "Resource: " + subType + " bound to this sla do not support type:" + policyAction);
                }
            }
        } else {
            log.debug("Not need to check.");
        }
    }
}
