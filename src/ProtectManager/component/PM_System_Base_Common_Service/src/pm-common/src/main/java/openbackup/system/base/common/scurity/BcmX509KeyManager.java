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

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;

import lombok.extern.slf4j.Slf4j;

import java.net.Socket;
import java.security.Key;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.Principal;
import java.security.PrivateKey;
import java.security.UnrecoverableKeyException;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;

import javax.net.ssl.X509ExtendedKeyManager;

/**
 * PM Feign Key Manager
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/12/8
 */
@Slf4j
public class BcmX509KeyManager extends X509ExtendedKeyManager {
    /**
     * pm server key的别名
     */
    private final String pmServerCertName;

    /**
     * pm server key的保护密码
     */
    private final char[] pmServerCertNameProtectPass;

    /**
     * 构造函数
     *
     * @param pmServerCertName            pm server key的别名
     * @param pmServerCertNameProtectPass pm server key的保护密码
     */
    public BcmX509KeyManager(String pmServerCertName, char[] pmServerCertNameProtectPass) {
        this.pmServerCertName = pmServerCertName;
        this.pmServerCertNameProtectPass = pmServerCertNameProtectPass;
    }

    /**
     * 选择使用keystore里哪一个证书
     *
     * @param keyType keyType
     * @param issuers issuers
     * @param socket  socket
     * @return server证书别名
     */
    @Override
    public String chooseClientAlias(String[] keyType, Principal[] issuers, Socket socket) {
        return pmServerCertName;
    }

    /**
     * 选择使用keystore里哪一个证书
     *
     * @param keyType keyType
     * @param issuers issuers
     * @param socket  socket
     * @return server证书别名
     */
    @Override
    public String chooseServerAlias(String keyType, Principal[] issuers, Socket socket) {
        return pmServerCertName;
    }

    /**
     * 获取证书
     *
     * @param alias 别名
     * @return 证书证书
     */
    @Override
    public X509Certificate[] getCertificateChain(String alias) {
        try {
            Certificate[] certificates = SecurityCertificateManager.getKeyStore().getCertificateChain(alias);
            if (certificates == null || certificates.length == 0) {
                return null;
            }
            X509Certificate[] x509Certificates = new X509Certificate[certificates.length];
            System.arraycopy(certificates, 0, x509Certificates, 0, certificates.length);
            return x509Certificates;
        } catch (KeyStoreException e) {
            log.error("Get x509 crt error.", ExceptionUtil.getErrorMessage(e));
        }
        return null;
    }

    /**
     * 选择使用keystore里哪一个证书
     *
     * @param keyType keyType
     * @param issuers issuers
     * @return server证书别名
     */
    @Override
    public String[] getClientAliases(String keyType, Principal[] issuers) {
        return new String[]{pmServerCertName};
    }

    /**
     * 获取私钥
     *
     * @param alias 别名
     * @return 证书私钥
     */
    @Override
    public PrivateKey getPrivateKey(String alias) {
        try {
            final Key key = SecurityCertificateManager.getKeyStore().getKey(alias, pmServerCertNameProtectPass);
            if (key instanceof PrivateKey) {
                return ((PrivateKey) key);
            } else {
                throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "not known pk");
            }
        } catch (UnrecoverableKeyException | KeyStoreException | NoSuchAlgorithmException e) {
            log.error("Get x509 pki error.", ExceptionUtil.getErrorMessage(e));
        }
        return null;
    }

    /**
     * 选择使用keystore里哪一个证书
     *
     * @param keyType keyType
     * @param issuers issuers
     * @return server证书别名
     */
    @Override
    public String[] getServerAliases(String keyType, Principal[] issuers) {
        return new String[]{pmServerCertName};
    }
}