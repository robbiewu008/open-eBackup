package openbackup.oracle.copy;

import openbackup.data.protection.access.provider.sdk.copy.CapabilityProvider;
import openbackup.data.protection.access.provider.sdk.enums.CopyFeatureEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * 副本支持功能provider
 *
 * @author c30016231
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-20
 */
@Component
public class OracleCapabilityProvider implements CapabilityProvider {
    private static final List<String> SUPPORTED_SUBTYPE = Arrays.asList(ResourceSubTypeEnum.ORACLE.getType(),
            ResourceSubTypeEnum.ORACLE_CLUSTER.getType());

    private static final List<CopyFeatureEnum> SUPPORTED_FEATURES = Arrays.asList(CopyFeatureEnum.RESTORE,
            CopyFeatureEnum.MOUNT, CopyFeatureEnum.INSTANT_RESTORE);

    @Override
    public boolean applicable(String subType) {
        return SUPPORTED_SUBTYPE.contains(subType);
    }

    @Override
    public List<CopyFeatureEnum> supportFeatures() {
        return SUPPORTED_FEATURES;
    }
}
