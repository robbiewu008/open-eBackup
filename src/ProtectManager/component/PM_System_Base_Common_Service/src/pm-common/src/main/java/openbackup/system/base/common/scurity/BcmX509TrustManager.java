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
package openbackup.system.base.common.scurity;

import openbackup.system.base.common.constants.ErrorCodeConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.LegoConfig;
import openbackup.system.base.common.utils.VerifyUtil;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateNotYetValidException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.util.concurrent.ConcurrentSkipListSet;

import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509TrustManager;

/**
 * 用于验证服务器证书的TrustManager
 *
 * @author l90005176
 * @version [OceanStor BCManager V200R001C00, 2016年2月16日]
 * @since 2022-12-15
 */
public class BcmX509TrustManager implements X509TrustManager {
    /**
     * 证书不合法
     */
    public static final long SYS_CERT_NOT_VALID = 1073947460L;

    /**
     * 证书不受信
     */
    public static final long SYS_CERT_NOT_TRUST = 1073947461L;

    private static final Logger logger = LoggerFactory.getLogger(BcmX509TrustManager.class);

    private static final ConcurrentSkipListSet<String> ALARMS = new ConcurrentSkipListSet<>();

    /**
     * 使用指定证书初始化的X509TrustManager实例
     */
    private X509TrustManager trustManager = null;

    /**
     * IP地址
     */
    private String ipAddress = "";

    private final IBcmX509TrustHandler handler;

    /**
     * 使用Certificate初始化TrustManager
     *
     * @param handler throwErrorOnAuthFail flag
     */
    public BcmX509TrustManager(IBcmX509TrustHandler handler) {
        this.handler = handler;
    }

    /**
     * 使用Certificate初始化TrustManager
     *
     * @param ipAddress IP
     * @param handler throwErrorOnAuthFail flag
     */
    public BcmX509TrustManager(String ipAddress, IBcmX509TrustHandler handler) {
        this(handler);
        this.ipAddress = ipAddress;
    }

    /**
     * 使用KeyStore初始化TrustManager
     *
     * @param keyStore 证书库
     * @param handler throwErrorOnAuthFail flag
     */
    public BcmX509TrustManager(KeyStore keyStore, IBcmX509TrustHandler handler) {
        this(handler);
        trustManager = getTrustManager(keyStore);
    }

    /**
     * 执行客户端SSL验证
     *
     * @param chain 客户端证书链
     * @param authType 基于客户端证书的验证类型
     * @throws CertificateException 认证异常
     */
    @Override
    public void checkClientTrusted(X509Certificate[] chain, String authType) throws CertificateException {
        return;
    }

    /**
     * 执行服务器SSL验证
     *
     * @param chain 服务器证书链
     * @param authType 使用的密钥交换算法
     * @throws CertificateException 认证异常
     */
    @Override
    public void checkServerTrusted(X509Certificate[] chain, String authType) throws CertificateException {
        // 证书告警开关关闭，则不进行证书认证
        if (!LegoConfig.getInstance().isCertAlarmSwitch()) {
            return;
        }

        // 检查证书是否失效
        boolean isValid = checkValidity(chain);
        if (!isValid) {
            return;
        }

        // 检查证书是否受信任
        boolean isTrusted = checkTrusted(chain, authType);
        if (isTrusted) {
            if (handler != null) {
                handler.handle(0L);
            }
        }

        // 检验服务端的证书是否已被吊销
        CrlContent.checkServerCertIsRevoked(chain);
    }

    private boolean checkValidity(X509Certificate[] chain) throws CertificateException {
        try {
            // 检查服务器证书有效性, 若服务器时间与设备时间不一致可能会出现设备证书提前失效或延后失效
            for (int i = 0; i < chain.length; i++) {
                chain[i].checkValidity();
            }
            cleanCertAlarmRecord("cert-invalidity");
            return true;
        } catch (CertificateException e) {
            failForCertInvalidity(e);
        } catch (Exception e) {
            failForCertInvalidity(e);
        }

        return false;
    }

    private void cleanCertAlarmRecord(String string) {
        if (ALARMS.remove(ipAddress + ":" + string)) {
            logger.info("remove {} record for {} success.", string, ipAddress);
        }
    }

    private void failForCertInvalidity(Exception exception) throws CertificateException {
        // 仅在第一次添加的时候能够添加成功，避免打印日志过多
        if (!VerifyUtil.isEmpty(ipAddress) && ALARMS.add(ipAddress + ":cert-invalidity")) {
            logger.error("Check server certificate failed. cert is not invalidity. ip= {}, msg: ", ipAddress,
                ExceptionUtil.getErrorMessage(exception));
            // 由于不会同时出现不受信和不合法的情况，当出现不合法的情况，清除不受信的记录。
            cleanCertAlarmRecord("cert-not-trust");
        }
        if (handler == null) {
            logger.info("handler is null.");
        } else {
            handler.handle(SYS_CERT_NOT_VALID);
            handler.handle(exception);
        }
    }

    private boolean checkTrusted(X509Certificate[] chain, String authType) throws CertificateException {
        try {
            if (trustManager == null) {
                getTrustManager(SecurityCertificateManager.getKeyStore()).checkServerTrusted(chain, authType);
            } else {
                trustManager.checkServerTrusted(chain, authType);
            }
            cleanCertAlarmRecord("cert-not-trust");
            return true;
        } catch (CertificateException e) {
            failForCertNoTrust(e);
        } catch (Exception e) {
            failForCertNoTrust(e);
        }
        return false;
    }

    private void failForCertNoTrust(Exception exception) throws CertificateException {
        // 仅在第一次添加的时候能够添加成功，避免打印日志过多
        if (!VerifyUtil.isEmpty(ipAddress) && ALARMS.add(ipAddress + ":cert-not-trust")) {
            logger.error("Check server certificate failed, cert is not trusted, ip={}, msg: ", ipAddress,
                ExceptionUtil.getErrorMessage(exception));
            // 由于不会同时出现不受信和不合法的情况，当出现不受信的情况，清除不合法的记录。
            cleanCertAlarmRecord("cert-invalidity");
        }
        if (handler == null) {
            logger.info("handler is null.");
        } else {
            handler.handle(SYS_CERT_NOT_TRUST);
            handler.handle(exception);
        }
    }

    /**
     * 返回受验证同位体信任的认证中心的数组。
     *
     * @return 可接受的 CA 发行者证书的非 null（可能为空）的数组。
     */
    @Override
    public X509Certificate[] getAcceptedIssuers() {
        if (trustManager == null) {
            return getTrustManager(SecurityCertificateManager.getKeyStore()).getAcceptedIssuers();
        } else {
            return trustManager.getAcceptedIssuers();
        }
    }

    /**
     * 获取以指定证书库初始化的TrustManager
     * <p>
     * 如果证书库为空则使用直接信任服务器证书的TrustManager
     *
     * @param keyStore 安全证书库
     * @return X509TrustManager TrustManager实现
     */
    private X509TrustManager getTrustManager(KeyStore keyStore) {
        if (keyStore == null) {
            logger.warn("The keyStore is null.");
            // 证书不存在
            throw new LegoCheckedException(ErrorCodeConstant.SSL_INIT_OR_CONNECT_FAIL);
        }

        removeExpireKeyStore(keyStore);

        X509TrustManager defaultTrustManager;
        try {
            TrustManagerFactory tmf = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
            tmf.init(keyStore);
            TrustManager tm = tmf.getTrustManagers()[0];
            if (tm instanceof X509TrustManager) {
                defaultTrustManager = (X509TrustManager) tm;
            } else {
                throw new LegoCheckedException(ErrorCodeConstant.SSL_INIT_OR_CONNECT_FAIL,
                    "TrustManagerFactory.getTrustManagers()[0] is not instanceof X509TrustManager");
            }
        } catch (Exception e) {
            logger.error("Initialized TrustManager failed by the certificate: ", ExceptionUtil.getErrorMessage(e));
            // 证书错误，根据证书获取TrustManager失败
            throw new LegoCheckedException(ErrorCodeConstant.SSL_INIT_OR_CONNECT_FAIL, e);
        }

        return defaultTrustManager;
    }

    private void removeExpireKeyStore(KeyStore keyStore) {
        // 去掉过期的ca证书，避免外部集群的ca证书过期了，外部集群的服务器证书未过期，校验通过
        Enumeration<String> aliases;
        try {
            aliases = keyStore.aliases();
        } catch (KeyStoreException e) {
            throw new LegoCheckedException(ErrorCodeConstant.SSL_INIT_OR_CONNECT_FAIL);
        }
        List<String> removeAliases = new ArrayList<>();
        while (aliases.hasMoreElements()) {
            String element = aliases.nextElement();
            Certificate certificate;
            try {
                certificate = keyStore.getCertificate(element);
            } catch (KeyStoreException e) {
                throw new LegoCheckedException(ErrorCodeConstant.SSL_INIT_OR_CONNECT_FAIL);
            }
            X509Certificate x509Certificate;
            if (certificate instanceof X509Certificate) {
                x509Certificate = (X509Certificate) certificate;
            } else {
                continue;
            }
            try {
                x509Certificate.checkValidity();
            } catch (CertificateExpiredException | CertificateNotYetValidException e) {
                removeAliases.add(element);
            }
        }
        for (String element : removeAliases) {
            try {
                logger.info("Ct is expire, delete ct name: {}", element);
                keyStore.deleteEntry(element);
            } catch (KeyStoreException keyStoreException) {
                logger.error("delete ct error message:{}.", keyStoreException.getMessage());
                throw new LegoCheckedException(ErrorCodeConstant.SSL_INIT_OR_CONNECT_FAIL);
            }
        }
    }
}
