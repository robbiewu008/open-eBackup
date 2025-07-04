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
package openbackup.oracle.utils;

import com.huawei.oceanprotect.system.base.cert.common.constants.CertErrorCode;
import com.huawei.oceanprotect.system.base.cert.util.CertFileUtil;
import com.huawei.oceanprotect.system.base.cert.util.CertUtil;

import lombok.extern.slf4j.Slf4j;
import openbackup.oracle.bo.OracleStorage;
import openbackup.oracle.constants.OracleConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import org.apache.commons.lang3.StringUtils;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.security.cert.CRL;
import java.security.cert.CRLException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509CRL;
import java.security.cert.X509Certificate;
import java.util.Objects;
import java.util.Optional;
import java.util.regex.Pattern;

/**
 * 功能描述
 *
 */
@Slf4j
public class StorageCheckUtil {
    private StorageCheckUtil() {
    }

    /**
     * 校验IP
     *
     * @param ipList ip集合
     * @param name 资源名称
     */
    public static void checkIp(String ipList, String name) {
        if (StringUtils.isBlank(ipList)) {
            log.error("param ip is invalid, env:{}", name);
            throw new LegoCheckedException(CommonErrorCode.IP_PORT_ERROR, "Invalid param ip.");
        }
        String[] ips = ipList.split(",");
        for (String ip : ips) {
            if (!Pattern.matches(RegexpConstants.IP_V4V6_ADDRESS, ip)) {
                log.error("param ip is invalid, env:{}, ip:{}", name, ip);
                throw new LegoCheckedException(CommonErrorCode.IP_PORT_ERROR, "Invalid param ip.");
            }
        }
    }

    /**
     * 校验端口
     *
     * @param port port
     * @param name 资源名称
     */
    public static void checkPort(int port, String name) {
        if (port < LegoNumberConstant.ZERO || port > OracleConstants.MAX_PORT) {
            log.error("param port is invalid, env:{}, port:{}", name, port);
            throw new LegoCheckedException(CommonErrorCode.IP_PORT_ERROR, "Invalid param port.");
        }
    }

    /**
     * 校验oracle storage证书信息
     *
     * @param storage 存储资源信息
     */
    public static void checkCert(OracleStorage storage) {
        if (!OracleConstants.ENABLE.equals(storage.getEnableCert())) {
            log.debug("FC(name: {}) not enable cert, enableCert: {}.");
            return;
        }
        X509Certificate certificate = parseCertificate(storage.getCertification());
        Optional<X509CRL> optCrl = parseCrl(storage.getRevocationList());
        optCrl.ifPresent(crl -> checkCertAndCrlMatch(certificate, crl));
    }

    private static X509Certificate parseCertificate(String certification) {
        if (VerifyUtil.isEmpty(certification)) {
            throw new LegoCheckedException(CertErrorCode.CERT_FORMAT_INVALID, "certification is empty");
        }
        if (certification.length() > OracleConstants.CERT_MAX_BYTE_SIZE) {
            throw new LegoCheckedException(CommonErrorCode.FILE_SIZE_VALIDATE_FAILED, new String[] {"1MB"},
                "upload file is too large.");
        }
        Certificate certificate;
        try (ByteArrayInputStream certStream = new ByteArrayInputStream(
            certification.getBytes(StandardCharsets.UTF_8))) {
            CertificateFactory certificateFactory = CertUtil.createCertificateFactory();
            certificate = certificateFactory.generateCertificate(certStream);
        } catch (CertificateException | IOException e) {
            throw new LegoCheckedException(CertErrorCode.CERT_FORMAT_INVALID, "Invalid certification");
        }
        if (certificate instanceof X509Certificate) {
            return (X509Certificate) certificate;
        }
        throw new LegoCheckedException(CertErrorCode.CERT_FORMAT_INVALID, "Certification is not X509");
    }

    private static Optional<X509CRL> parseCrl(String revocationlist) {
        if (VerifyUtil.isEmpty(revocationlist)) {
            return Optional.empty();
        }
        if (revocationlist.length() > OracleConstants.CRL_MAX_BYTE_SIZE) {
            throw new LegoCheckedException(CommonErrorCode.FILE_SIZE_VALIDATE_FAILED, new String[] {"5KB"},
                "upload file is too large.");
        }
        CRL crl;
        try (ByteArrayInputStream revocationStream = new ByteArrayInputStream(
            revocationlist.getBytes(StandardCharsets.UTF_8))) {
            CertificateFactory certificateFactory = CertUtil.createCertificateFactory();
            crl = certificateFactory.generateCRL(revocationStream);
        } catch (CRLException | IOException e) {
            throw new LegoCheckedException(CertErrorCode.CRL_FORMAT_INVALID, "Invalid crl");
        }
        if (crl instanceof X509CRL) {
            CertFileUtil.checkCrlIsValid((X509CRL) crl);
            return Optional.of((X509CRL) crl);
        }
        throw new LegoCheckedException(CertErrorCode.CRL_FORMAT_INVALID, "Crl is not X509");
    }

    private static void checkCertAndCrlMatch(X509Certificate certificate, X509CRL crl) {
        String certName = certificate.getIssuerX500Principal().getName();
        String crlName = crl.getIssuerX500Principal().getName();
        if (!Objects.equals(certName, crlName)) {
            throw new LegoCheckedException(CertErrorCode.CRL_ISSUER_INCONSISTENT_CERT_ISSUER, "cert no match with crl");
        }
    }
}
