package openbackup.data.protection.access.provider.sdk.copy;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.enums.CopyFeatureEnum;

import java.util.List;

/**
 * Capability Provider
 *
 * @author tWX1009756
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-2-28
 */
public interface CapabilityProvider extends DataProtectionProvider<String> {
    /**
     * 副本有哪些支持的特性
     *
     * @return 副本支持的特性
     */
    List<CopyFeatureEnum> supportFeatures();
}