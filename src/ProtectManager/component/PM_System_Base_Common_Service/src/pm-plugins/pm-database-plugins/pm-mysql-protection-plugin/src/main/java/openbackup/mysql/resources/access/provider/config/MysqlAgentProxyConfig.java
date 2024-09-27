package openbackup.mysql.resources.access.provider.config;

import lombok.Getter;
import lombok.Setter;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.context.annotation.PropertySource;
import org.springframework.stereotype.Component;

/**
 * AgentProxyProperties agent代理
 *
 * @author xwx950025
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-31
 */
@Setter
@Getter
@Component
@PropertySource("classpath:application.yaml")
@ConfigurationProperties(prefix = "services.endpoints.proxy.agent")
public class MysqlAgentProxyConfig {
    private String host;

    private Integer port;
}