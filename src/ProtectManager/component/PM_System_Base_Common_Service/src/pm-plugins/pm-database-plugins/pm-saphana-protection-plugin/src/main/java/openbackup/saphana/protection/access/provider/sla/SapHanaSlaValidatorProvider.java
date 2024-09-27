package openbackup.saphana.protection.access.provider.sla;

import openbackup.data.protection.access.provider.sdk.sla.SlaValidateProvider;
import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * SAP HANA的SLA校验器Provider
 *
 * @author wWX1013713
 * @version [DataBackup 1.5.0]
 * @since 2023-06-09
 */
@Slf4j
@Component
public class SapHanaSlaValidatorProvider implements SlaValidateProvider {
    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.SAPHANA_DATABASE.getType().equals(resourceSubType);
    }

    @Override
    public SlaValidateConfig getConfig() {
        SlaValidateConfig slaConfig = new SlaValidateConfig();
        slaConfig.getSpecificationConfig()
            .setLimit(PolicyLimitConfig.of(PolicyAction.FULL, SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.DIFFERENCE_INCREMENT,
                SlaConstants.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.CUMULATIVE_INCREMENT,
                SlaConstants.CUMULATIVE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.LOG, SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT));
        return slaConfig;
    }
}
