package openbackup.gaussdb.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.gaussdb.protection.access.service.GaussDBService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Objects;

/**
 * GaussDB资源类提供者
 *
 * @author t30021437
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-02-21
 */
@Component
@Slf4j
public class GaussDBResourceProvider implements ResourceProvider {
    @Autowired
    private GaussDBService gaussDbService;

    /**
     * 联通性子类过滤接口
     *
     * @param object 受保护资源
     * @return boolean 是否使用该类进行连通性校验
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.HCS_GAUSSDB_INSTANCE.getType()
            .equals(object.getSubType());
    }

    /**
     * 设置path信息，否则复制的时候会报错
     *
     * @param resource 资源
     */
    @Override
    public void beforeCreate(ProtectedResource resource) {
        log.info("GaussDBResourceProvider beforeCreate setPath:{}, resource.getEndpoint:{}", resource.getPath(),
            resource.getEndpoint());
    }

    /**
     * 集群环境修改的时候会去检查连通性、实例和数据库资源属于自动扫描。更新资源时不再去检查
     *
     * @param resource 资源
     */
    @Override
    public void beforeUpdate(ProtectedResource resource) {
    }

    /**
     * 不支持lanfree的应用实现ResourceProvider接口
     *
     * @return GaussDB资源是否更新主机信息配置
     */
    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = ResourceProvider.super.getResourceFeature();
        resourceFeature.setSupportedLanFree(false);
        return resourceFeature;
    }
}
