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
package openbackup.eccoracle.sla;

import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.sla.SlaValidateProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

/**
 * SAP ECC 的 SLA 校验器 Provider
 *
 */
@Slf4j
@Component
public class EccOracleSlaValidateProvider implements SlaValidateProvider {
    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.SAP_ON_ORACLE.getType().equals(resourceSubType);
    }

    @Override
    public SlaValidateConfig getConfig() {
        SlaValidateConfig slaConfig = new SlaValidateConfig();
        slaConfig.getSpecificationConfig()
                .setLimit(PolicyLimitConfig.of(PolicyAction.FULL, SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
                .setLimit(PolicyLimitConfig.of(PolicyAction.LOG, SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
                .setLimit(PolicyLimitConfig.of(PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT))
                .setLimit(PolicyLimitConfig.of(PolicyAction.REPLICATION_LOG,
                        SlaConstants.REPLICATION_LOG_POLICY_COUNT_LIMIT))
                .setLimit(PolicyLimitConfig.of(PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT));
        return slaConfig;
    }
}


