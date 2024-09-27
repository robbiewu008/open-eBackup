/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.dameng.protection.access.sla;

import openbackup.data.protection.access.provider.sdk.sla.SlaValidateProvider;
import openbackup.database.base.plugin.service.impl.SlaValidService;
import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

/**
 * dameng的sla应用校验
 *
 * @author lWX1100347
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-20
 */
@Component
public class DamengSlaValidate implements SlaValidateProvider {
    private final SlaValidService slaValidService;

    public DamengSlaValidate(SlaValidService slaValidService) {
        this.slaValidService = slaValidService;
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.DAMENG.getType().equals(subType);
    }

    @Override
    public SlaValidateConfig getConfig() {
        SlaValidateConfig slaValidateConfig = SlaValidateConfig.defaults();
        slaValidateConfig.getSpecificationConfig()
            .setLimit(PolicyLimitConfig.of(PolicyAction.CUMULATIVE_INCREMENT,
                SlaConstants.CUMULATIVE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.LOG, SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT));
        return slaValidateConfig;
    }

    @Override
    public void validateSLA(SlaBase slaBase) {
        slaValidService.modifySlaCheckResourcePublicPolicy(slaBase, ResourceSubTypeEnum.DAMENG_CLUSTER.getType(),
            PolicyAction.LOG.getAction());
        slaValidService.modifySlaCheckResourcePublicPolicy(slaBase, ResourceSubTypeEnum.DAMENG_CLUSTER.getType(),
            PolicyAction.ARCHIVING.getAction());
        SlaValidateProvider.super.validateSLA(slaBase);
    }
}
