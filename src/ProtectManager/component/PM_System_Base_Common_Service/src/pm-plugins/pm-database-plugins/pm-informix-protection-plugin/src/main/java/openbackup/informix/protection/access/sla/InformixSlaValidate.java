package openbackup.informix.protection.access.sla;

import openbackup.data.protection.access.provider.sdk.sla.SlaValidateProvider;
import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

/**
 * informix的sla应用校验
 *
 * @author zWX951267
 * @version [OceanProtect X8000 1.5.0]
 * @since 2023-05-06
 */
@Component
public class InformixSlaValidate implements SlaValidateProvider {
    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.INFORMIX_SERVICE.getType().equals(subType);
    }

    @Override
    public SlaValidateConfig getConfig() {
        SlaValidateConfig slaValidateConfig = SlaValidateConfig.defaults();
        slaValidateConfig.getSpecificationConfig()
                .setLimit(PolicyLimitConfig.of(PolicyAction.CUMULATIVE_INCREMENT,
                        SlaConstants.CUMULATIVE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
                .setLimit(PolicyLimitConfig.of(PolicyAction.LOG,
                        SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT));
        return slaValidateConfig;
    }
}
