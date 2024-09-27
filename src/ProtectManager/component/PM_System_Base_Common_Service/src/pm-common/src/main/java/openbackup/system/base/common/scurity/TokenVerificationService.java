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
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JwtTokenUtils;
import openbackup.system.base.common.utils.KeystoreUtils;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.security.PublicKey;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;

/**
 * Token Verification Service
 *
 * @author l00272247
 * @since 2020-11-27
 */
@Component
@Slf4j
public class TokenVerificationService {
    /**
     * get Token Verify PublicKey
     *
     * @return public key
     * @throws CertificateException certificate exception
     */
    private PublicKey getTokenVerifyPublicKey() throws CertificateException {
        String dataString = "-----BEGIN CERTIFICATE-----" + System.lineSeparator() + KeystoreUtils.getInstance()
                .getCertificate() + System.lineSeparator() + "-----END CERTIFICATE-----";
        CertificateFactory certificateFactory = CertificateFactory.getInstance("X.509");
        try (ByteArrayInputStream dataStream = new ByteArrayInputStream(dataString.getBytes(StandardCharsets.UTF_8))) {
            Certificate certificate = certificateFactory.generateCertificate(dataStream);
            return certificate.getPublicKey();
        } catch (IOException e) {
            log.error("Get the public key failed", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Get the public key failed");
        }
    }

    /**
     * parsing token from request
     *
     * @return TokenBo
     */
    public TokenBo parsingTokenFromRequest() {
        String token = JwtTokenUtils.parsingTokenFromRequest();
        if (token == null) {
            return null;
        }
        PublicKey publicKey;
        try {
            publicKey = getTokenVerifyPublicKey();
        } catch (CertificateException e) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "get token verify public key failed", e);
        }
        return JwtTokenUtils.parseToken(token, publicKey);
    }
}
