package openbackup.system.base.common.rest;

import openbackup.system.base.util.KeyToolUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.autoconfigure.web.ServerProperties;
import org.springframework.boot.web.server.Ssl;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Primary;

/**
 * 微服务内部证书双向认证配置类
 *
 * @author xwx1016404
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-11-24
 */
@Slf4j
@Configuration
public class SslFeignConfiguration {
    /**
     * KeyStore文件
     */
    @Value("${server.ssl.key-store}")
    private String keyStoreFile;

    /**
     * KeyStore中存放的双向证书别名
     */
    @Value("${server.ssl.key-alias}")
    private String alias;

    /**
     * KeyStore类型
     */
    @Value("${server.ssl.key-store-type}")
    private String type;

    /**
     * KeyStore密钥文件
     */
    @Value("${server.ssl.key-store-password-file}")
    private String keyStorePwdFile;

    @Autowired
    private KeyToolUtil keyToolUtil;

    /**
     * Feign服务端配置
     *
     * @return Feign服务端配置
     */
    @Primary
    @Bean
    public ServerProperties serverProperties() {
        keyToolUtil.getInternalKeystore();

        // ssl配置
        Ssl ssl = new Ssl();

        ssl.setKeyStoreType(type);

        ssl.setTrustStoreType(type);

        ssl.setProtocol(KeyToolUtil.SSL_CONTEXT_VERSION);

        // 通信keystore路径
        ssl.setKeyStore(keyStoreFile);

        // 通信keystore密码
        String pwd = keyToolUtil.getKeyStorePassword(keyStorePwdFile);
        ssl.setKeyStorePassword(pwd);

        // 通信（服务端）证书的别名
        ssl.setKeyAlias(alias);

        // 信任库路径
        ssl.setTrustStore(keyStoreFile);

        // 信任库密码
        ssl.setTrustStorePassword(pwd);

        // 开启ssl支持
        ssl.setEnabled(true);

        // 开启客户端证书校验
        ssl.setClientAuth(Ssl.ClientAuth.NEED);

        ServerProperties serverProperties = new ServerProperties();
        serverProperties.setSsl(ssl);
        return serverProperties;
    }
}
