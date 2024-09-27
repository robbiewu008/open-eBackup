package openbackup.goldendb.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.sla.SlaValidateProvider;
import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * GoldenDBSlaValidator
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-30
 */
@Slf4j
@Component
public class GoldenDBSlaValidator implements SlaValidateProvider {
    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.GOLDENDB_CLUSETER_INSTANCE.getType().equals(object)
            || ResourceSubTypeEnum.GOLDENDB_CLUSTER.getType().equals(object) || ResourceSubTypeEnum.GOLDENDB.getType()
            .equals(object);
    }

    @Override
    public SlaValidateConfig getConfig() {
        SlaValidateConfig slaValidateConfig = new SlaValidateConfig();

        // GoldenDB共需要配置四种备份类型（全量，永久增量，日志）/复制/归档
        slaValidateConfig.getSpecificationConfig()
            .setLimit(PolicyLimitConfig.of(PolicyAction.FULL, SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.DIFFERENCE_INCREMENT,
                SlaConstants.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.LOG, SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT));
        return slaValidateConfig;
    }
}
