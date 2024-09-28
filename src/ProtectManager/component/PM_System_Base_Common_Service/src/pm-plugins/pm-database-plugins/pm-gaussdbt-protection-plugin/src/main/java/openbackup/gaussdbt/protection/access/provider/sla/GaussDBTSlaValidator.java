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
package openbackup.gaussdbt.protection.access.provider.sla;

import openbackup.data.protection.access.provider.sdk.sla.SlaValidateProvider;
import openbackup.database.base.plugin.service.impl.SlaValidService;
import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * GaussDBTSlaValidator
 *
 */
@Slf4j
@Component
public class GaussDBTSlaValidator implements SlaValidateProvider {
    private final SlaValidService slaValidService;

    public GaussDBTSlaValidator(SlaValidService slaValidService) {
        this.slaValidService = slaValidService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.GAUSSDBT.getType().equals(object);
    }

    @Override
    public SlaValidateConfig getConfig() {
        SlaValidateConfig slaValidateConfig = new SlaValidateConfig();

        // GaussDBT共需要配置四种备份类型（全量，差异增量，累积增量，日志）/复制/归档
        slaValidateConfig.getSpecificationConfig()
            .setLimit(PolicyLimitConfig.of(PolicyAction.FULL, SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.DIFFERENCE_INCREMENT,
                SlaConstants.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.CUMULATIVE_INCREMENT,
                SlaConstants.CUMULATIVE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.LOG, SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT));
        log.info("set GaussDBT sla PolicyLimitConfig success.");
        return slaValidateConfig;
    }

    @Override
    public void validateSLA(SlaBase slaBase) {
        slaValidService.modifySlaCheckResourcePublicPolicy(slaBase, ResourceSubTypeEnum.GAUSSDBT_SINGLE.getType(),
            PolicyAction.DIFFERENCE_INCREMENT.getAction());
        slaValidService.modifySlaCheckResourcePublicPolicy(slaBase, ResourceSubTypeEnum.GAUSSDBT_SINGLE.getType(),
            PolicyAction.CUMULATIVE_INCREMENT.getAction());
        SlaValidateProvider.super.validateSLA(slaBase);
    }
}
