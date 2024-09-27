/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 */

package openbackup.system.base.config;

import lombok.Getter;
import lombok.Setter;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.context.annotation.PropertySource;
import org.springframework.stereotype.Component;

/**
 * 功能描述: DME 备份网络平面代理
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-17
 */
@Setter
@Getter
@Component
@PropertySource("classpath:application.yaml")
@ConfigurationProperties(prefix = "dme")
public class DmeProxyProperties {
    /**
     * DME 备份网络平面代理IP: ${dme.ip}
     */
    private String ip;

    /**
     * DME 备份网络平面代理PORT: ${dme.proxyPort}
     */
    private Integer proxyPort;
}