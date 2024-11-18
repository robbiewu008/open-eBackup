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
package openbackup.database.base.plugin.service.impl;

import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.dto.UpdateSlaCommand;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;

import com.google.common.collect.ImmutableMap;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * SlaValidService校验
 *
 */
@Service
@Slf4j
public class SlaValidService {
    private static final Map<String, String> SLA_POLICY_ACTION_LABEL = ImmutableMap.of(
        PolicyAction.FULL.getAction(), "common_full_backup_label",
        PolicyAction.LOG.getAction(), "common_log_backup_label",
        PolicyAction.DIFFERENCE_INCREMENT.getAction(), "common_incremental_backup_label",
        PolicyAction.CUMULATIVE_INCREMENT.getAction(), "common_diff_backup_label",
        PolicyAction.PERMANENT_INCREMENT.getAction(), "common_permanent_backup_label");

    private final ProtectObjectRestApi protectObjectRestApi;

    public SlaValidService(ProtectObjectRestApi protectObjectRestApi) {
        this.protectObjectRestApi = protectObjectRestApi;
    }

    /**
     * 修改sla，只支持修改该资源公有的sla策略
     *
     * @param slaBase slaBase sla策略
     * @param subType 检验的子类类型
     * @param action 策略行动
     */
    public void modifySlaCheckResourcePublicPolicy(SlaBase slaBase, String subType, String action) {
        if (slaBase instanceof UpdateSlaCommand) {
            UpdateSlaCommand updateSlaCommand = (UpdateSlaCommand) slaBase;
            List<String> policyActions = updateSlaCommand.getPolicyList()
                .stream()
                .map(PolicyDto::getAction)
                .map(PolicyAction::getAction)
                .collect(Collectors.toList());
            boolean isContainsDifferenceAction = policyActions.stream().anyMatch(action::equals);
            if (!isContainsDifferenceAction) {
                log.info("Do not need to check the action of the sla policy.");
                return;
            }
            List<ProtectedObjectInfo> items;
            int page = 0;
            do {
                BasePage<ProtectedObjectInfo> data = protectObjectRestApi.pageQueryProtectObject(
                    updateSlaCommand.getUuid(), page, IsmNumberConstant.HUNDRED);
                items = data.getItems();
                Optional<ProtectedObjectInfo> databaseProject = items.stream()
                    .filter(project -> subType.equals(project.getSubType()))
                    .findFirst();
                // 存在已经绑定sla的资源
                if (databaseProject.isPresent()) {
                    throw new LegoCheckedException(CommonErrorCode.SLA_NOT_SUPPORT_BACKUP_POLICY,
                        new String[] {subType, SLA_POLICY_ACTION_LABEL.get(action)},
                        "Resources bound to this sla have different policy sets, modification is not supported.");
                }
                page++;
            } while (items.size() >= IsmNumberConstant.HUNDRED);
        }
    }
}