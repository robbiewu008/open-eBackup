package openbackup.data.protection.access.provider.sdk.util;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

/**
 * ProtectResourceUtil
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-10-18
 */
public class ProtectedResourceUtil {
    /**
     * 默认不可更新字段
     *
     * @param resource resource
     */
    public static void cleanUnmodifiableFields(ProtectedResource resource) {
        resource.setRootUuid(null);
        resource.setSubType(null);
        resource.setUserId(null);
        resource.setAuthorizedUser(null);
        resource.setParentUuid(null);
        resource.setParentName(null);
        resource.setCreatedTime(null);
        resource.setProtectionStatus(null);
    }
}
