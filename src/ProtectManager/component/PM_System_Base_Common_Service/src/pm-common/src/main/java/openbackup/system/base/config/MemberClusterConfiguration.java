package openbackup.system.base.config;

import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.sdk.cluster.MemberClusterRestApi;
import openbackup.system.base.util.RequestUriUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Scope;
import org.springframework.context.annotation.ScopedProxyMode;

/**
 * MemberClusterConfiguration
 *
 * @author y30046482
 * @since 2023-05-27
 */
@Slf4j
@Configuration
public class MemberClusterConfiguration {
    /**
     * 生成 MemberClusterRestApi, 使用DMA代理，校验是否有外部集群对应的证书
     *
     * @param proxyProperties dma代理，域名和端口
     * @return TargetClusterRestApi
     */
    @Bean("memberClusterApiWithDmaProxyCheckCert")
    @Scope(value = "prototype", proxyMode = ScopedProxyMode.INTERFACES)
    public MemberClusterRestApi createTargetRequestBeanCheckCert(DmaProxyProperties proxyProperties) {
        return FeignBuilder.buildDefaultMemberClusterClient(MemberClusterRestApi.class,
            RequestUriUtil.getDmaProxy(proxyProperties));
    }
}
