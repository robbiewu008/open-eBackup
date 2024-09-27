package openbackup.exchange.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.exchange.protection.access.constant.ExchangeConstant;
import openbackup.exchange.protection.access.service.ExchangeService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

/**
 * ExchangeMailboxProvider
 *
 * @author s30036254
 * @since 2023-04-27
 */
@Slf4j
@Component
public class ExchangeMailboxProvider implements ResourceProvider {
    @Autowired
    private ExchangeService exchangeService;

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return false;
    }

    /**
     * 检查受保护资源，创建逻辑资源（文件集，NAS共享）时调用该接口
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码<br/>
     * 回调函数中不允许对资源的UUID等关键字段进行修改。
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeCreate(ProtectedResource resource) {
        ProtectedEnvironment environment = exchangeService.getEnvironmentById(
            resource.getDependencies().get(ExchangeConstant.EXCHANGE_AGENTS).get(ExchangeConstant.INT_ZERO).getUuid());

        // 设置path信息，否则复制的时候会报错
        resource.setPath(environment.getEndpoint());
    }

    /**
     * 检查受保护资源， 修改逻辑资源（文件集，NAS共享）时调用该接口
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码<br/>
     * 回调函数中不允许对资源的UUID等关键字段进行修改。
     * <p>
     * 提供的资源不包含dependency信息，如果应用需要补齐depen信息，请调用 “补充资源的dependency信息” 接口
     *
     * @param resource 受保护资源
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
