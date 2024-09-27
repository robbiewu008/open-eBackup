package openbackup.tdsql.resources.access.provider;

import openbackup.data.protection.access.provider.sdk.copy.CapabilityProvider;
import openbackup.data.protection.access.provider.sdk.enums.CopyFeatureEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * TdsqlCapabilityProvider
 *
 * @author z00445440
 * @since 2023-12-19
 */
@Component
public class TdsqlCapabilityProvider implements CapabilityProvider {
    private static final List<CopyFeatureEnum> SUPPORTED_FEATURES = Arrays.asList(CopyFeatureEnum.RESTORE,
        CopyFeatureEnum.MOUNT);

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType().equals(subType);
    }

    @Override
    public List<CopyFeatureEnum> supportFeatures() {
        return SUPPORTED_FEATURES;
    }
}
