/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.system.base.config;

import feign.Client;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.util.KeyToolUtil;

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
 * Internal Client Https Config
 *
 */
@Slf4j
@Component
public class FeignClientConfig {
    private static SSLSocketFactory feignTrustingSslSocketFactory;

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
                    client = createInternalClient();
                }
            }
        }
        return client;
    }

    private Client createInternalClient() {
        if (Objects.isNull(feignTrustingSslSocketFactory)) {
            try {
                feignTrustingSslSocketFactory = getFeignTrustingSslSocketFactory();
            } catch (NoSuchAlgorithmException | UnrecoverableKeyException | KeyStoreException
                    | KeyManagementException e) {
                log.error("get feign ssl socket factory failed", e);
            }
        }
        Client internalClient = new Client.Default(feignTrustingSslSocketFactory, (host, session) -> true);
        log.info("create internal client success.");
        return internalClient;
    }

    private SSLSocketFactory getFeignTrustingSslSocketFactory()
            throws KeyStoreException, NoSuchAlgorithmException, KeyManagementException, UnrecoverableKeyException {
        // 加载keystore
        KeyStore keyStore = keyToolUtil.getInternalKeystore();

        KeyManagerFactory keyManagerFactory = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
        keyManagerFactory.init(keyStore, keyToolUtil.getKeyStorePassword(keyStorePwdFile).toCharArray());
        KeyManager[] keyManagers = keyManagerFactory.getKeyManagers();

        TrustManagerFactory trustManagerFactory = TrustManagerFactory
                .getInstance(TrustManagerFactory.getDefaultAlgorithm());
        trustManagerFactory.init(keyStore);
        TrustManager[] trustManagers = trustManagerFactory.getTrustManagers();

        SSLContext sslContext = SSLContext.getInstance(KeyToolUtil.SSL_CONTEXT_VERSION);
        sslContext.init(keyManagers, trustManagers, SecureRandom.getInstanceStrong());
        return sslContext.getSocketFactory();
    }
}