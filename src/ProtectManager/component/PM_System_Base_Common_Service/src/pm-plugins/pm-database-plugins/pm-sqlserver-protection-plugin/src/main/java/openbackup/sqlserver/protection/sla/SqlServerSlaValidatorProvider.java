package openbackup.sqlserver.protection.sla;

import openbackup.data.protection.access.provider.sdk.sla.SlaValidateProvider;
import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * SQL Server的SLA应用校验
 *
 * @author xWX1016404
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-15
 */
@Component
public class SqlServerSlaValidatorProvider implements SlaValidateProvider {
    @Override
    public boolean applicable(String subType) {
        List<String> typeList = Arrays.asList(ResourceSubTypeEnum.SQL_SERVER.getType(),
            ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType(), ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType(),
            ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType(),
            ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType());
        return typeList.contains(subType);
    }

    @Override
    public SlaValidateConfig getConfig() {
        SlaValidateConfig slaValidateConfig = new SlaValidateConfig();
        slaValidateConfig.getSpecificationConfig()
            .setLimit(PolicyLimitConfig.of(PolicyAction.FULL, SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.CUMULATIVE_INCREMENT,
                SlaConstants.CUMULATIVE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.LOG, SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT));
        return slaValidateConfig;
    }
}
