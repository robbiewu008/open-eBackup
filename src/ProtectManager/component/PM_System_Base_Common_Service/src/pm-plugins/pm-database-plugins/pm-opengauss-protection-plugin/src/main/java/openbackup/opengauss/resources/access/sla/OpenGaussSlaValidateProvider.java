/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.opengauss.resources.access.sla;

import openbackup.data.protection.access.provider.sdk.sla.SlaValidateProvider;
import openbackup.database.base.plugin.service.impl.SlaValidService;
import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

/**
 * openGauss sla provider
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-16
 */
@Service
@Slf4j
public class OpenGaussSlaValidateProvider implements SlaValidateProvider {
    private final SlaValidService slaValidService;

    public OpenGaussSlaValidateProvider(SlaValidService slaValidService) {
        this.slaValidService = slaValidService;
    }

    @Override
    public void validateSLA(SlaBase slaBase) {
        slaValidService.modifySlaCheckResourcePublicPolicy(slaBase, ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType(),
            PolicyAction.DIFFERENCE_INCREMENT.getAction());
        SlaValidateProvider.super.validateSLA(slaBase);
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.OPENGAUSS.getType().equals(subType);
    }

    @Override
    public SlaValidateConfig getConfig() {
        SlaValidateConfig config = new SlaValidateConfig();
        log.info("opengauss start to config sla");
        config.getSpecificationConfig()
            .setLimit(PolicyLimitConfig.of(PolicyAction.FULL, SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.DIFFERENCE_INCREMENT,
                SlaConstants.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.LOG, SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT));
        return config;
    }
}
