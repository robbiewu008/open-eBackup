package openbackup.system.base.config;

import lombok.Getter;
import lombok.Setter;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.context.annotation.PropertySource;
import org.springframework.stereotype.Component;

/**
 * dma url配置
 *
 * @author g00500588
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-01
 */
@Setter
@Getter
@Component
@PropertySource("classpath:application.yaml")
@ConfigurationProperties(prefix = "dma")
public class DmaProxyProperties {
    private String host;

    private Integer port;
}