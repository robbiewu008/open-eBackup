package openbackup.system.base.config;

import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.sdk.cluster.NodeRestApi;
import openbackup.system.base.sdk.config.api.PmConfigRestApi;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Scope;
import org.springframework.context.annotation.ScopedProxyMode;

/**
 * NodeRestApi配置类
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-05
 */
@Configuration
public class NodeRestApiConfiguration {
    @Autowired
    private FeignClientConfig feignClientConfig;

    /**
     * 获取NodeRestApi
     *
     * @return nodeRestApi
     */
    @Bean("nodeRestApi")
    @Scope(value = "prototype", proxyMode = ScopedProxyMode.INTERFACES)
    public NodeRestApi createNodeRestApiClient() {
        return FeignBuilder.buildInternalHttpsClientWithSpringMvcContract(NodeRestApi.class,
            feignClientConfig.getInternalClient());
    }


    /**
     * 获取pmConfigRestApi
     *
     * @return nodeRestApi
     */
    @Bean("pmConfigRestApi")
    @Scope(value = "prototype", proxyMode = ScopedProxyMode.INTERFACES)
    public PmConfigRestApi createPmConfigRestApiClient() {
        return FeignBuilder.buildInternalHttpsClient(PmConfigRestApi.class, feignClientConfig.getInternalClient());
    }
}
