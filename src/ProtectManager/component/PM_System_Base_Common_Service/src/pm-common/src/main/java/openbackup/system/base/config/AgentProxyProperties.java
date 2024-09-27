package openbackup.system.base.config;

import lombok.Getter;
import lombok.Setter;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.context.annotation.PropertySource;
import org.springframework.stereotype.Component;

/**
 * AgentProxyProperties agent代理
 *
 * @author zwx1010134
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-03-04
 */
@Setter
@Getter
@Component
@PropertySource("classpath:application.yaml")
@ConfigurationProperties(prefix = "services.endpoints.proxy.agent")
public class AgentProxyProperties {
    private String host;

    private Integer port;
}
