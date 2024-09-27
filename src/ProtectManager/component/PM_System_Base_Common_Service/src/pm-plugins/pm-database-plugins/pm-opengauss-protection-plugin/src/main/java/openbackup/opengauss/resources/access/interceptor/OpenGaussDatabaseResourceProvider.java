package openbackup.opengauss.resources.access.interceptor;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Arrays;

/**
 * OpenGauss资源类提供者
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-12
 */
@Component
public class OpenGaussDatabaseResourceProvider implements ResourceProvider {
    /**
     * 集群环境修改的时候会去检查连通性、实例和数据库资源属于自动扫描。更新资源时不再去检查
     *
     * @param resource 资源
     */
    @Override
    public void beforeCreate(ProtectedResource resource) {
    }

    /**
     * 集群环境修改的时候会去检查连通性、实例和数据库资源属于自动扫描。更新资源时不再去检查
     *
     * @param resource 资源
     */
    @Override
    public void beforeUpdate(ProtectedResource resource) {
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return Arrays.asList(ResourceSubTypeEnum.OPENGAUSS.getType(), ResourceSubTypeEnum.OPENGAUSS_INSTANCE.getType(),
            ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType()).contains(protectedResource.getSubType());
    }
}
