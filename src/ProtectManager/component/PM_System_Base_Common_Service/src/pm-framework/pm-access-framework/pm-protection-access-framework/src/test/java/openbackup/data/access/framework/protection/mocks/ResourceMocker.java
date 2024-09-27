package openbackup.data.access.framework.protection.mocks;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

/**
 * 资源相关的mock
 *
 * @author z30027603
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/7/22
 */
public class ResourceMocker {
    public static ProtectedResource mockResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
        protectedResource.setUuid("test1");
        return protectedResource;
    }
}
