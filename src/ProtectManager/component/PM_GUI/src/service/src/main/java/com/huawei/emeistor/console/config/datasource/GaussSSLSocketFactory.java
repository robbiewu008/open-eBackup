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

import com.huawei.emeistor.console.util.ExceptionUtil;

import org.postgresql.ssl.WrappedFactory;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.GeneralSecurityException;
import java.security.KeyStore;
import java.security.SecureRandom;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509TrustManager;

/**
 * GaussSSLSocketFactory
 *
 */
public class GaussSSLSocketFactory extends WrappedFactory {
    private static final Logger log = LoggerFactory.getLogger(GaussSSLSocketFactory.class);

    private static final String CA_CERT_PATH = "/opt/OceanProtect/infrastructure/cert/internal/ca/ca.crt.pem";

    /**
     * Constructor
     *
     * @throws Exception Exception
     */
    public GaussSSLSocketFactory() throws Exception {
        SSLContext sslContext = SSLContext.getInstance("TLSv1.2");
        sslContext.init(null, new TrustManager[] {SingleCertTrustManager.getInstance()},
            SecureRandom.getInstanceStrong());
        this._factory = sslContext.getSocketFactory();
    }

    private static class SingleCertTrustManager implements X509TrustManager {
        private static volatile SingleCertTrustManager instance = null;

        private X509TrustManager trustManager;

        private SingleCertTrustManager() {
            InputStream is = null;
            FileInputStream fis = null;
            try {
                KeyStore ks = KeyStore.getInstance(KeyStore.getDefaultType());
                ks.load(null);
                CertificateFactory cf = CertificateFactory.getInstance("X509");
                fis = new FileInputStream(CA_CERT_PATH);
                is = new BufferedInputStream(fis);
                Certificate cert = cf.generateCertificate(is);
                X509Certificate x509Cert;
                if (cert instanceof X509Certificate) {
                    x509Cert = (X509Certificate) cert;
                } else {
                    throw new GeneralSecurityException("Not X509Certificate");
                }
                ks.setCertificateEntry("internalca", x509Cert);
                TrustManagerFactory tmf = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
                tmf.init(ks);
                for (TrustManager tm : tmf.getTrustManagers()) {
                    if (tm instanceof X509TrustManager) {
                        trustManager = (X509TrustManager) tm;
                        break;
                    }
                }
                if (trustManager == null) {
                    throw new GeneralSecurityException("No X509TrustManager found");
                }
            } catch (IOException | GeneralSecurityException e) {
                log.error("generalSecurity exception", ExceptionUtil.getErrorMessage(e));
            } finally {
                try {
                    if (is != null) {
                        is.close();
                    }
                    if (fis != null) {
                        fis.close();
                    }
                } catch (IOException e) {
                    log.error("stream close exception", ExceptionUtil.getErrorMessage(e));
                }
            }
        }

        public static SingleCertTrustManager getInstance() {
            if (instance == null) {
                synchronized (SingleCertTrustManager.class) {
                    if (instance == null) {
                        instance = new SingleCertTrustManager();
                    }
                }
            }
            return instance;
        }

        @Override
        public void checkClientTrusted(X509Certificate[] chain, String authType) throws CertificateException {
        }

        @Override
        public void checkServerTrusted(X509Certificate[] chain, String authType) throws CertificateException {
            trustManager.checkServerTrusted(chain, authType);
        }

        @Override
        public X509Certificate[] getAcceptedIssuers() {
            return new X509Certificate[0];
        }
    }
}
