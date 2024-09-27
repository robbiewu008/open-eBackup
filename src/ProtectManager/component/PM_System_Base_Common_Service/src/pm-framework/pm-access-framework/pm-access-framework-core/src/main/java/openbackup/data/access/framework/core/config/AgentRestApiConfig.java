package openbackup.data.access.framework.core.config;

import openbackup.data.access.client.sdk.api.framework.agent.AgentUnifiedRestApi;
import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.util.RequestUriUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import java.net.InetSocketAddress;
import java.net.Proxy;

/**
 * 功能描述: AgentRestApiConfig
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-27
 */
@Slf4j
@Configuration
public class AgentRestApiConfig {
    @Value("${dme.ip}")
    private String dmeProxyIp;

    @Value("${dme.proxyPort}")
    private Integer dmeProxyPort;

    /**
     * 生成访问agent的 rest api
     *
     * @return agentRestApi
     */
    @Bean
    public AgentUnifiedRestApi agentRestApi() {
        return FeignBuilder.buildHttpsTarget(AgentUnifiedRestApi.class, null, true, true, getDmeProxy());
    }

    private Proxy getDmeProxy() {
        RequestUriUtil.verifyIpAndPort(dmeProxyIp, dmeProxyPort);
        return new Proxy(Proxy.Type.HTTP, new InetSocketAddress(dmeProxyIp, dmeProxyPort));
    }
}