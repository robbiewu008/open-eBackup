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
package openbackup.access.framework.resource.util;

import openbackup.access.framework.resource.util.ResourceCertUtil;

import org.junit.Assert;
import org.junit.Test;

/**
 * 证书资源工具类
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/10/14
 */
public class ResourceCertUtilTest {
    /**
     * 用例名称：检验吊销列表。
     * 前置条件：吊销列表不是错误的数据。
     * check点：返回值为true or false。
     */
    @Test
    public void test_check_crl_is_valid_success() {
        String crl = "-----BEGIN X509 CRL-----\n" +
                "MIIBuDCBoQIBATANBgkqhkiG9w0BAQsFADA1MQswCQYDVQQGEwJDTjEPMA0GA1UE\n" +
                "CgwGSHVhd2VpMRUwEwYDVQQDDAxPY2VhblByb3RlY3QXDTIyMDkyODA4MTQxNloX\n" +
                "DTIyMTAyODA4MTQxNlowKDASAgECFw0yMjA5MjgwODEyNThaMBICAQsXDTIyMDky\n" +
                "ODAzNDQ1NFqgDjAMMAoGA1UdFAQDAgEDMA0GCSqGSIb3DQEBCwUAA4IBAQAuF5Xb\n" +
                "F7ozVYzgLY/d6PTmuOV0cwDR5XPiPlqrhtGIEQ12sQ5dUCZseeXURPC+oXdqO+xu\n" +
                "LPoxP6JGkAw22tvkOqvuM46zZMBbm7s+1KudSSXjPpSglnjAjg+02YJhwMLkI8Tw\n" +
                "uPj8GJOVBOuSSU9bQ+8yxxhURvJ1a9RvzIOBld4YCd7OxwSJ5xhEjJImlXXPe8HY\n" +
                "eleKhc8gmjo5ncfyrH/w/+4GNioEwn9KUH/YF16tQRCl/AHOMpR9wt44UKTjdkd3\n" +
                "IXonoZJ/RBzP1MeUeQXjj/HzoJ0pb2Q/M3o+aS6DPfoYbqGMKOOu3+GCPEs731VO\n" +
                "LDyIh7lI+dgscsZj\n" +
                "-----END X509 CRL-----";
        Assert.assertNotNull(ResourceCertUtil.checkCrlIsValid(crl, "resName"));
    }

    /**
     * 用例名称：检验吊销列表。
     * 前置条件：吊销列表是错误的数据。
     * check点：返回值为true。
     */
    @Test
    public void test_check_crl_is_valid_success_when_crl_is_error() {
        Assert.assertTrue(ResourceCertUtil.checkCrlIsValid("xxxx", "resName"));
    }

    /**
     * 用例名称：检验证书。
     * 前置条件：证书不是错误的数据。
     * check点：返回值为true or false。
     */
    @Test
    public void test_check_cert_is_valid_success() {
        String cert = "-----BEGIN CERTIFICATE-----\n" +
                "MIIDSDCCAjCgAwIBAgIBATANBgkqhkiG9w0BAQsFADA1MQswCQYDVQQGEwJDTjEP\n" +
                "MA0GA1UECgwGSHVhd2VpMRUwEwYDVQQDDAxPY2VhblByb3RlY3QwHhcNMjIwOTE2\n" +
                "MDMxNjAxWhcNMzcwOTEyMDMxNjAxWjA1MQswCQYDVQQGEwJDTjEPMA0GA1UECgwG\n" +
                "SHVhd2VpMRUwEwYDVQQDDAxPY2VhblByb3RlY3QwggEiMA0GCSqGSIb3DQEBAQUA\n" +
                "A4IBDwAwggEKAoIBAQC2Cm+ARbmDsKGgiF5GEC2p46Szk9pdnlSn2RedDFdiXgmE\n" +
                "Rst8ta7Psp6d/lTRlCsrwvQPKBiK+dyKDEAEcMc97KWc73rKDTNFD3CrIiGJCH79\n" +
                "MoWqEotc97dPXwI0Y00M5pH2BOGd68HoPPkhsO5SIIW7xTEexPqh0BLfdwo7JYwJ\n" +
                "a0Yz84pIGv5Sd9ggz6neLgDttS0iAkrqFgFN3gcUX2Y/hkPDbSWz8LU/hD6bdVmm\n" +
                "8l4Ep5j+WKYnibeR8TxCCO31hFupEyO8Adcr4r9B4ActESvrExWzdoCQWHpsIJhi\n" +
                "zDRHqEbFXzzE4vytiUF0GkpOTvCwNnPRiT2IVslBAgMBAAGjYzBhMB0GA1UdDgQW\n" +
                "BBRy3LmPD+cwmWEf7+M/b2ytc3PMDjAfBgNVHSMEGDAWgBRy3LmPD+cwmWEf7+M/\n" +
                "b2ytc3PMDjAPBgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjANBgkqhkiG\n" +
                "9w0BAQsFAAOCAQEATMnmM7t1CsVC0bgTnuICUM/dpagwxCNu8HxAE2xGeHiHxkyj\n" +
                "U9DxtYNQn64WTb/oeB2y2LHzy1/ONpdBu4G1WlCIcqt3OfaKaTyqJ9/nCbw6stBh\n" +
                "ABW3b9iesXYtrRl4icXWLPFKzFrhNSai+2h+E/x2a6f107pwtXSdL4OfvdfZpEU1\n" +
                "TswlF4DnHIEodJvMRGTs+Y097uhFNi3UoZJfWaB4jbT9wCTn07UCwmLJADrYRRE5\n" +
                "Ykg0i/AuTDj2bMuG4zNa44WJAtnHhCS6Pk+3qRM1cFFhbj/clQbmswicIHnb2mIK\n" +
                "Rvo48gRMX3mfLNexu/akUodRekvQxm6YBdDnTQ==\n" +
                "-----END CERTIFICATE-----";
        Assert.assertNotNull(ResourceCertUtil.checkCertificateIsValid(cert, "resName"));
    }

    /**
     * 用例名称：检验证书。
     * 前置条件：证书是错误的数据。
     * check点：返回值为null。
     */
    @Test
    public void test_check_cert_is_valid_success_when_cert_is_error() {
        Assert.assertTrue(ResourceCertUtil.checkCertificateIsValid("xxxx", "resName"));
    }
}
