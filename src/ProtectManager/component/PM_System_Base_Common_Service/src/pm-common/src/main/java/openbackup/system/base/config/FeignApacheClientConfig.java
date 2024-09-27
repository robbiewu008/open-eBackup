package openbackup.system.base.config;

import openbackup.system.base.common.rest.ApacheHttp5ClientBuilder;
import openbackup.system.base.util.KeyToolUtil;

import feign.Client;
import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.UnrecoverableKeyException;

import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;

/**
 * Internal Client Https Config
 *
 * @author xwx1016404
 * @since 2022-01-04
 */
@Slf4j
@Component
public class FeignApacheClientConfig {
    private static volatile Client client = null;

    private static final Object LOCK = new Object();

    /**
     * KeyStore密钥文件
     */
    @Value("${server.ssl.key-store-password-file}")
    private String keyStorePwdFile;

    @Autowired
    private KeyToolUtil keyToolUtil;

    /**
     * 内部认证Feign客户端
     *
     * @return Feign客户端
     */
    public Client getInternalClient() {
        if (client == null) {
            synchronized (LOCK) {
                if (client == null) {
                    client = createApacheClient();
                }
            }
        }
        return client;
    }

    private Client createApacheClient() {
        try {
            SSLContext sslContext = getFeignTrustingSslContext();
            return ApacheHttp5ClientBuilder.buildClientFromSslContext(sslContext);
        } catch (NoSuchAlgorithmException | UnrecoverableKeyException | KeyStoreException | KeyManagementException e) {
            log.error("get feign ssl socket factory failed", e);
            return ApacheHttp5ClientBuilder.buildDefaultClient();
        }
    }

    private SSLContext getFeignTrustingSslContext()
        throws KeyStoreException, NoSuchAlgorithmException, KeyManagementException, UnrecoverableKeyException {
        // 加载keystore
        KeyStore keyStore = keyToolUtil.getInternalKeystore();

        KeyManagerFactory keyManagerFactory = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
        keyManagerFactory.init(keyStore, keyToolUtil.getKeyStorePassword(keyStorePwdFile).toCharArray());
        KeyManager[] keyManagers = keyManagerFactory.getKeyManagers();

        TrustManagerFactory trustManagerFactory = TrustManagerFactory.getInstance(
            TrustManagerFactory.getDefaultAlgorithm());
        trustManagerFactory.init(keyStore);
        TrustManager[] trustManagers = trustManagerFactory.getTrustManagers();

        SSLContext sslContext = SSLContext.getInstance(KeyToolUtil.SSL_CONTEXT_VERSION);
        sslContext.init(keyManagers, trustManagers, SecureRandom.getInstanceStrong());
        return sslContext;
    }
}