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

import openbackup.system.base.common.constants.CertErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.extern.slf4j.Slf4j;

import java.security.cert.X509CRL;
import java.security.cert.X509Certificate;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * 吊销列表
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/28
 */
@Slf4j
public class CrlContent {
    /**
     * 错误信息，代表证书被吊销
     */
    private static final String CERT_IS_REVOKED = "target cert is revoked.";

    /**
     * 所有的吊销列表对象
     * 此共享变量涉及读读并发，读写并发，写写不并发
     */
    private static CopyOnWriteArrayList<X509CRL> crlList = new CopyOnWriteArrayList<>();

    /**
     * 检验服务端的证书是否已被吊销
     *
     * @param serverCertChain 服务端证书链
     */
    public static void checkServerCertIsRevoked(X509Certificate[] serverCertChain) {
        for (X509Certificate x509Certificate : serverCertChain) {
            for (X509CRL crl : crlList) {
                if (!crl.isRevoked(x509Certificate)) {
                    continue;
                }
                throw new LegoCheckedException(CertErrorCode.CERT_IS_REVOKED, CERT_IS_REVOKED);
            }
        }
    }

    /**
     * refresh crl list
     *
     * @param newCrlList 吊销列表集合
     * @return 刷新吊销列表对象
     */
    public static CopyOnWriteArrayList<X509CRL> refreshCrlList(CopyOnWriteArrayList<X509CRL> newCrlList) {
        log.debug("crl size: {}", newCrlList.size());
        return crlList = newCrlList;
    }

    /**
     * 检验异常是不是证书被吊销的异常，如果是则报相应的错
     *
     * @param cause 异常
     */
    public static void checkIsErrorByCertRevoked(Throwable cause) {
        if (CrlContent.CERT_IS_REVOKED.equals(cause.getMessage())) {
            throw new LegoCheckedException(CertErrorCode.CERT_IS_REVOKED, CrlContent.CERT_IS_REVOKED);
        }
    }
}