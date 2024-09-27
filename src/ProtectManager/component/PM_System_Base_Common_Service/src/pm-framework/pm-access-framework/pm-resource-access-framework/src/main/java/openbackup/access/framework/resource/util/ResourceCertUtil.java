/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.access.framework.resource.util;

import com.huawei.oceanprotect.system.base.cert.util.CertFileUtil;
import com.huawei.oceanprotect.system.base.cert.util.CertUtil;

import lombok.extern.slf4j.Slf4j;

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

/**
 * 证书资源工具类
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/10/13
 */
@Slf4j
public class ResourceCertUtil {
    /**
     * 检验吊销列表是否有效
     *
     * @param revocationList 吊销列表
     * @param resourceName   资源名称
     * @return 是否有效
     */
    public static boolean checkCrlIsValid(String revocationList, String resourceName) {
        CRL crl;
        try (ByteArrayInputStream revocationStream =
            new ByteArrayInputStream(revocationList.getBytes(StandardCharsets.UTF_8))) {
            CertificateFactory certificateFactory = CertUtil.createCertificateFactory();
            crl = certificateFactory.generateCRL(revocationStream);
        } catch (CRLException | IOException e) {
            log.error("Error Crl in env: {}", resourceName);
            return true;
        }
        if (crl instanceof X509CRL) {
            boolean isValidity = CertFileUtil.getCrlValidity((X509CRL) crl);
            log.info("Resource Crl: {} validity is: {}", resourceName, isValidity);
            return isValidity;
        } else {
            log.error("Crl is not X509 in env: {}", resourceName);
            return true;
        }
    }

    /**
     * 检验证书是否有效
     *
     * @param certification 证书
     * @param resourceName  资源名称
     * @return 是否有效
     */
    public static boolean checkCertificateIsValid(String certification, String resourceName) {
        Certificate certificate;
        try (ByteArrayInputStream certStream =
            new ByteArrayInputStream(certification.getBytes(StandardCharsets.UTF_8))) {
            CertificateFactory certificateFactory = CertUtil.createCertificateFactory();
            certificate = certificateFactory.generateCertificate(certStream);
        } catch (CertificateException | IOException e) {
            log.error("Error certification in env: {}", resourceName);
            return true;
        }
        if (certificate instanceof X509Certificate) {
            X509Certificate x509Certificate = (X509Certificate) certificate;
            boolean isValidity = CertUtil.checkValidity(x509Certificate);
            log.info("Resource Cert: {} validity is: {}", resourceName, isValidity);
            return isValidity;
        } else {
            log.error("Certification is not X509 in env: {}", resourceName);
            return true;
        }
    }
}
