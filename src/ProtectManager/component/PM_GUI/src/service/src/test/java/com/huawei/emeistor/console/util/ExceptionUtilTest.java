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
