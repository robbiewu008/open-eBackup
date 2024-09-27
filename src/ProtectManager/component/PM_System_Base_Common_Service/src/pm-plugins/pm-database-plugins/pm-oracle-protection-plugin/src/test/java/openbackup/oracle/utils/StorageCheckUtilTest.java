/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.oracle.utils;

import openbackup.oracle.bo.OracleStorage;
import com.huawei.oceanprotect.system.base.cert.common.constants.CertErrorCode;

import openbackup.oracle.utils.StorageCheckUtil;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述
 *
 * @author l30023229
 * @since 2023-12-04
 */
public class StorageCheckUtilTest {
    private static final String CERTIFICATION = "-----BEGIN CERTIFICATE-----\n" +
        "MIIDnTCCAoWgAwIBAgIJAJr9YoOhto+NMA0GCSqGSIb3DQEBCwUAMGUxCzAJBgNV\n" +
        "BAYTAkNOMRAwDgYDVQQIDAdTaGFhbnhpMQ4wDAYDVQQHDAVYaSdhbjEPMA0GA1UE\n" +
        "CgwGSHVhd2VpMQswCQYDVQQLDAJJVDEWMBQGA1UEAwwNVG9tY2F0IFNlcnZlcjAe\n" +
        "Fw0xNDAxMTUwMzAwMDJaFw0yNDAxMTMwMzAwMDJaMGUxCzAJBgNVBAYTAkNOMRAw\n" +
        "DgYDVQQIDAdTaGFhbnhpMQ4wDAYDVQQHDAVYaSdhbjEPMA0GA1UECgwGSHVhd2Vp\n" +
        "MQswCQYDVQQLDAJJVDEWMBQGA1UEAwwNVG9tY2F0IFNlcnZlcjCCASIwDQYJKoZI\n" +
        "hvcNAQEBBQADggEPADCCAQoCggEBAOLZY6OUxQsFoeTzDnUROamzvepD0OcKOZlW\n" +
        "KrTr7kx53qwTko8eci5T2PH47bLGr/6FQQfILAXeyUj+hrUqe6hs0Vkvz2PA0J9f\n" +
        "0I+mMINjPqATf4zBNfuHVJ97khfFtehbk/Rq1APgIgNwVIIh6HPLH0vI5GueAwKx\n" +
        "0vZ58QNxuh1KBHs3zGX/ommEwD4NBsaAqma4QwO33PppvF5H5C3oVE7ew8Colbr4\n" +
        "a194/GjeNkKKpPp6FiEnxMC2WdbXA0A5wpYVKzjzPw3Kx7sKVyGANktmImTxtw4G\n" +
        "bE6gVJgskWNj9nM9Q6dx0ojuMvHEbt5Z5DOmOsp0HfZ7qpy3E4MCAwEAAaNQME4w\n" +
        "HQYDVR0OBBYEFLLXu8N2g/GDu/N16rP6za6QFSpJMB8GA1UdIwQYMBaAFLLXu8N2\n" +
        "g/GDu/N16rP6za6QFSpJMAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEB\n" +
        "AFtPuFAkGOxSxjmnMMeDIJI2FYJdzVzgakd5W3jD+wZJKvbCICxLue+04mGgfLQj\n" +
        "w43SwbNlBjQfaeKAYTgTv33hFLlUEX1a2K1hMzCdmDV4DEcbaU7b0NDSHhLiOvsK\n" +
        "amV0so6qXECBNfRAnigBgMdjN3Lx0xxnclWu70FzBv8UU8HLambUBqSPoVp6k4ox\n" +
        "ceXyK1JQRyiBAQS6k/0GBSa29SevWnqLCEbR5+d0vx81l9xO78FUrb+GTjgv6rmb\n" +
        "OcBxYCQDisK4NvAaD/6s/UDGixwp1v9GQIYeahyuJTlAsyJ+30unoHSrqfI+hCtl\n" +
        "PvYwf4krgonpouUd0zC+RQA=\n" +
        "-----END CERTIFICATE-----";

    private static final String OVER_TIME_CRL = "-----BEGIN X509 CRL-----\n" +
        "MIIBtDCBnQIBATANBgkqhkiG9w0BAQsFADBFMQswCQYDVQQGEwJDTjEKMAgGA1UE\n" +
        "CAwBLjEKMAgGA1UEBwwBLjEPMA0GA1UECgwGaHVhd2VpMQ0wCwYDVQQDDARyb290\n" +
        "Fw0yMjA3MjcwNjUwNDdaFw0yMjA4MjYwNjUwNDdaMBQwEgIBBRcNMjIwNzI3MDY1\n" +
        "MDE1WqAOMAwwCgYDVR0UBAMCAQAwDQYJKoZIhvcNAQELBQADggEBAHEd0FvGY/Yx\n" +
        "vMtvZGXcNCSGO9Ij21Gx27UQ83NBEKCd1ndOc/yVhMbctUOeox1obs1Qv/M58A4U\n" +
        "4/5cwInBBtS5BfNPL4cfwQwO1pvhftpAF8JPB9+D9cCaoVl6B6G5WzQN6Kp9p87l\n" +
        "lvKEZo61Xq4VqXkDgyMuXK0Cenf9ZqG7/0wdq+p6gdVF0qabYjSVnGF1zZTFgKkd\n" +
        "k3RaPvdbIO8xjno2p6lUVnrptNRQH+KOszlOBRGs/Ry0YrPSDK+otul2ci6+jISU\n" +
        "DEbSNcUVQFRax3O/E4sez4W4/5XZYp4G2b8tr8OkU9yLxOgpyZDDRoFLtEQY4xvF\n" +
        "04dn+s/K08M=\n" +
        "-----END X509 CRL-----\n";
    @Test
    public void check_ip_success(){
        StorageCheckUtil.checkIp("127.0.0.1,127.0.0.1", "");
    }

    @Test(expected = LegoCheckedException.class)
    public void check_ip_fail(){
        StorageCheckUtil.checkIp("3333.0.0.1", "");
    }

    @Test
    public void check_port_success(){
        StorageCheckUtil.checkPort(8088, "");
    }

    @Test(expected = LegoCheckedException.class)
    public void check_port_fail(){
        StorageCheckUtil.checkPort(852363, "");
    }

    @Test
    public void check_cert_success(){
        OracleStorage oracleStorage = new OracleStorage();
        oracleStorage.setEnableCert("0");
        StorageCheckUtil.checkCert(oracleStorage);

        oracleStorage.setEnableCert("1");
        oracleStorage.setCertification("");
        try {
            StorageCheckUtil.checkCert(oracleStorage);
        } catch (LegoCheckedException e) {
            Assert.assertEquals(CertErrorCode.CERT_FORMAT_INVALID, e.getErrorCode());
        }

        oracleStorage.setEnableCert("1");
        oracleStorage.setCertification(CERTIFICATION);
        oracleStorage.setRevocationList(OVER_TIME_CRL);
        try {
            StorageCheckUtil.checkCert(oracleStorage);
        } catch (LegoCheckedException e) {
            Assert.assertEquals(CertErrorCode.CRL_IS_INVALID, e.getErrorCode());
        }
    }
}
