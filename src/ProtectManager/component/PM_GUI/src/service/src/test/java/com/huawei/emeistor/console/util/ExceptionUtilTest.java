package com.huawei.emeistor.console.util;

import org.apache.http.HttpException;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.modules.junit4.PowerMockRunner;

import java.io.FileNotFoundException;
import java.security.cert.CertificateException;

import javax.net.ssl.SSLHandshakeException;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-09-14
 */
@RunWith(PowerMockRunner.class)
public class ExceptionUtilTest {

    @Test
    public void test_is_special_exception() {
        Assert.assertTrue(ExceptionUtil.isSpecialException(new ClassNotFoundException()));
        Assert.assertFalse(ExceptionUtil.isSpecialException(new HttpException()));
    }

    @Test
    public void test_get_error_message_success(){
        FileNotFoundException r = new FileNotFoundException();
        Exception errorMessage = ExceptionUtil.getErrorMessage(r);
        Assert.assertNotNull(errorMessage);
    }

    @Test
    public void test_get_error_message_success_when_throwable_is_null(){
        Exception errorMessage = ExceptionUtil.getErrorMessage(null);
        Assert.assertNotNull(errorMessage);
    }

    @Test
    public void test_deep_error_message(){
        SSLHandshakeException sslHandshakeException = new SSLHandshakeException("HandShake Error");
        CertificateException certificateException = new CertificateException(sslHandshakeException);
        Exception errorMessage = ExceptionUtil.getErrorMessage(certificateException);
        Assert.assertNotNull(errorMessage);
    }

    @Test
    public void test_is_valid_success(){
        Assert.assertTrue(ExceptionUtil.isValidClass("com.huawei.emeistor.console.util.ExceptionUtil"));
        Assert.assertFalse(ExceptionUtil.isValidClass("com.test.Test"));
    }
}
