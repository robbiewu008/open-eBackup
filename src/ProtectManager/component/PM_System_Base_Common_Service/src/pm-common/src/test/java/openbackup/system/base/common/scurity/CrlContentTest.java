/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.common.scurity;

import openbackup.system.base.common.exception.LegoCheckedException;
import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import openbackup.system.base.common.scurity.CrlContent;
import sun.security.x509.X509CRLImpl;
import sun.security.x509.X509CertImpl;

import java.security.cert.X509CRL;
import java.security.cert.X509Certificate;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * CrlContent Test
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/8/2
 */
public class CrlContentTest {
    /**
     * 用例场景：检验服务端证书是否已被吊销
     * 前置条件：服务端证书链中存在一个证书被吊销了
     * 检查点: 报错
     */
    @Test
    public void should_throw_LegoCheckedException_when_check_when_if_server_cert_is_revoked() {
        X509Certificate[] serverCertChain = mockServerChain();
        CopyOnWriteArrayList<X509CRL> crlList = mockCrlList();

        PowerMockito.when(crlList.get(0).isRevoked(serverCertChain[0])).thenReturn(false);
        PowerMockito.when(crlList.get(0).isRevoked(serverCertChain[1])).thenReturn(false);

        PowerMockito.when(crlList.get(1).isRevoked(serverCertChain[0])).thenReturn(false);
        PowerMockito.when(crlList.get(1).isRevoked(serverCertChain[1])).thenReturn(true);

        Assert.assertThrows("target cert is revoked.", LegoCheckedException.class, () -> CrlContent.checkServerCertIsRevoked(serverCertChain));
    }

    /**
     * 用例场景：检验服务端证书是否已被吊销
     * 前置条件：服务端证书链中所有证书都没有被吊销
     * 检查点: 不报错
     */
    @Test
    public void check_server_cert_is_revoked_success() {
        X509Certificate[] serverCertChain = mockServerChain();
        CopyOnWriteArrayList<X509CRL> crlList = mockCrlList();

        PowerMockito.when(crlList.get(0).isRevoked(serverCertChain[0])).thenReturn(false);
        PowerMockito.when(crlList.get(0).isRevoked(serverCertChain[1])).thenReturn(false);
        PowerMockito.when(crlList.get(1).isRevoked(serverCertChain[0])).thenReturn(false);
        PowerMockito.when(crlList.get(1).isRevoked(serverCertChain[1])).thenReturn(false);
        CrlContent.checkServerCertIsRevoked(serverCertChain);
    }

    /**
     * 用例场景：检验当错误对象的是证书吊销的错误信息时，抛出对应的异常
     * 前置条件：1、错误对象的信息是证书吊销的错误信息
     * 检查点: 报错LegoCheckedException
     */
    @Test
    public void check_is_error_by_cert_revoked_success() {
        Throwable throwable = new Throwable("target cert is revoked.");
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class, () -> CrlContent.checkIsErrorByCertRevoked(throwable));
        Assert.assertTrue(legoCheckedException.getMessage().contains("target cert is revoked."));
    }

    private CopyOnWriteArrayList<X509CRL> mockCrlList() {
        CopyOnWriteArrayList<X509CRL> crlList = new CopyOnWriteArrayList<>();
        crlList.add(PowerMockito.mock(X509CRLImpl.class));
        crlList.add(PowerMockito.mock(X509CRLImpl.class));
        CrlContent.refreshCrlList(crlList);
        return crlList;
    }

    private X509Certificate[] mockServerChain() {
        X509Certificate[] serverCertChain = new X509Certificate[2];
        serverCertChain[0] = PowerMockito.mock(X509CertImpl.class);
        serverCertChain[1] = PowerMockito.mock(X509CertImpl.class);
        return serverCertChain;
    }
}
