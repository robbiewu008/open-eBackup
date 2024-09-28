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
package com.huawei.emeistor.console.config.datasource;

import com.huawei.emeistor.console.util.EncryptorRestClient;

import feign.Client;
import feign.Feign;
import feign.Request;
import feign.RequestInterceptor;
import feign.RequestTemplate;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.io.FileUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.UnrecoverableKeyException;
import java.security.cert.CertificateException;
import java.util.Objects;
import java.util.concurrent.TimeUnit;

import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;

/**
 * 功能描述
 *
 */
@Slf4j
public class FeignClientConfiguration implements RequestInterceptor {
    private static final int CONNECT_TIMEOUT = 30 * 1000; // 30s

    private static final int READ_TIMEOUT = 2 * 60 * 1000; // 2分钟

    private static final Request.Options DEFAULT_TIMEOUT_OPTIONS = new Request.Options(CONNECT_TIMEOUT,
        TimeUnit.MILLISECONDS, READ_TIMEOUT, TimeUnit.MILLISECONDS, true);

    private static SSLSocketFactory feignTrustingSslSocketFactory;

    @Value("${server.ssl.key-store-password-file}")
    private String keyStorePwdFile;

    @Value("${server.ssl.key-store}")
    private String keyStoreFile;

    @Value("${server.ssl.key-store-type}")
    private String type;

    @Autowired
    private EncryptorRestClient encryptorRestClient;

    /**
     * 通用Feign Builder
     *
     * @return 通用FeignBuilder
     */
    @Bean("commonBuilder")
    public Feign.Builder feignBuilder() {
        Client internalClient = createInternalClient();
        log.info("create internal client success.");
        return new Feign.Builder().options(DEFAULT_TIMEOUT_OPTIONS).client(internalClient);
    }

    private Client createInternalClient() {
        if (Objects.isNull(feignTrustingSslSocketFactory)) {
            try {
                feignTrustingSslSocketFactory = getFeignTrustingSslSocketFactory();
            } catch (NoSuchAlgorithmException | UnrecoverableKeyException | KeyStoreException | KeyManagementException
                exception) {
                log.error("get feign ssl socket factory failed", exception);
            }
        }
        Client internalClient = new Client.Default(feignTrustingSslSocketFactory, (host, session) -> true);
        log.info("create internal client success.");
        return internalClient;
    }

    private SSLSocketFactory getFeignTrustingSslSocketFactory()
        throws KeyStoreException, NoSuchAlgorithmException, KeyManagementException, UnrecoverableKeyException {
        // 加载keystore
        KeyStore keyStore = loadKeystore(getKeyStorePassword(keyStorePwdFile), keyStoreFile);
        KeyManagerFactory keyManagerFactory = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
        keyManagerFactory.init(keyStore, getKeyStorePassword(keyStorePwdFile).toCharArray());
        KeyManager[] keyManagers = keyManagerFactory.getKeyManagers();
        TrustManagerFactory trustManagerFactory = TrustManagerFactory.getInstance(
            TrustManagerFactory.getDefaultAlgorithm());
        trustManagerFactory.init(keyStore);
        TrustManager[] trustManagers = trustManagerFactory.getTrustManagers();
        SSLContext sslContext = SSLContext.getInstance("TLSv1.3");
        sslContext.init(keyManagers, trustManagers, SecureRandom.getInstanceStrong());
        return sslContext.getSocketFactory();
    }

    private String getKeyStorePassword(String keyStoreAuthFile) {
        String cipherText = null;
        try {
            cipherText = FileUtils.readFileToString(new File(keyStoreAuthFile), StandardCharsets.UTF_8);
        } catch (IOException e) {
            log.error("read keystore auth file error: ", e);
        }
        return encryptorRestClient.decrypt(cipherText).getPlaintext();
    }

    private KeyStore loadKeystore(String password, String path) {
        KeyStore keyStore = null;
        try (InputStream ins = Files.newInputStream(Paths.get(path))) {
            keyStore = KeyStore.getInstance(type);
            keyStore.load(ins, password.toCharArray());
        } catch (IOException | KeyStoreException | NoSuchAlgorithmException | CertificateException e) {
            log.error("load keystore file error: ", e);
        }
        return keyStore;
    }

    @Override
    public void apply(RequestTemplate template) {
        log.debug("send request. method: {}", template.method());
    }
}
