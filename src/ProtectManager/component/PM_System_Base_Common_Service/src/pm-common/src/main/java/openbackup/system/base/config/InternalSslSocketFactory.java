/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.config;

import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.util.KeyToolUtil;

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
import java.util.Objects;

import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;

/**
 * InternalSSLSocketFactory
 *
 * @author y30046482
 * @since 2023-12-23
 */
@Slf4j
@Component
public class InternalSslSocketFactory {
    private static SSLSocketFactory internalTrustingSslSocketFactory;
    private static final Object LOCK = new Object();

    /**
     * KeyStore密钥文件
     */
    @Value("${server.ssl.key-store-password-file}")
    private String keyStorePwdFile;

    @Autowired
    private KeyToolUtil keyToolUtil;

    /**
     * 内部认证SSLSocketFactory
     *
     * @return SSLSocketFactory
     */
    public SSLSocketFactory getInternalSSLSocketFactory() {
        if (internalTrustingSslSocketFactory == null) {
            synchronized (LOCK) {
                if (internalTrustingSslSocketFactory == null) {
                    createInternalTrustingSslSocketFactory();
                }
            }
        }
        return internalTrustingSslSocketFactory;
    }

    private void createInternalTrustingSslSocketFactory() {
        if (Objects.isNull(internalTrustingSslSocketFactory)) {
            try {
                internalTrustingSslSocketFactory = getInternalTrustingSslSocketFactory();
            } catch (NoSuchAlgorithmException
                | UnrecoverableKeyException
                | KeyStoreException
                | KeyManagementException e) {
                log.error("Create internal ssl socket factory failed", ExceptionUtil.getErrorMessage(e));
            }
        }
    }

    private SSLSocketFactory getInternalTrustingSslSocketFactory()
        throws KeyStoreException, NoSuchAlgorithmException, KeyManagementException, UnrecoverableKeyException {
        // 加载keystore
        KeyStore keyStore = keyToolUtil.getInternalKeystore();

        KeyManagerFactory keyManagerFactory = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
        keyManagerFactory.init(keyStore, keyToolUtil.getKeyStorePassword(keyStorePwdFile).toCharArray());
        KeyManager[] keyManagers = keyManagerFactory.getKeyManagers();

        TrustManagerFactory trustManagerFactory =
            TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
        trustManagerFactory.init(keyStore);
        TrustManager[] trustManagers = trustManagerFactory.getTrustManagers();

        SSLContext sslContext = SSLContext.getInstance(KeyToolUtil.SSL_CONTEXT_VERSION);
        sslContext.init(keyManagers, trustManagers, SecureRandom.getInstanceStrong());
        return sslContext.getSocketFactory();
    }
}