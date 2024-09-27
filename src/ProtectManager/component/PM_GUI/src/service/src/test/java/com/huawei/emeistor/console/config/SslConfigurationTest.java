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
package com.huawei.emeistor.console.config;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.any;

import com.huawei.emeistor.console.bean.PlaintextVo;
import com.huawei.emeistor.console.util.EncryptorRestClient;

import org.apache.commons.io.FileUtils;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.SpringApplicationRunListener;
import org.springframework.boot.autoconfigure.web.ServerProperties;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.boot.web.server.Ssl;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.util.ReflectionTestUtils;

import java.io.File;

/**
 * {@link SslConfiguration} 测试类
 *
 * @author w00616953
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-28
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({FileUtils.class, SslConfiguration.class})
public class SslConfigurationTest {
    /**
     * 用例名称：生成SslBean成功，且返回值正确
     * 前置条件：无
     * check点：1. 生成Bean成功；2. 返回值正确
     */
    @Test
    public void test() {
        SslConfiguration sslConfiguration = new SslConfiguration();
        Ssl ssl = sslConfiguration.ssl();
        assertThat(ssl).isNotNull();
    }

    /**
     * 用例名称：生成ServerPropertiesBean成功，且返回值正确
     * 前置条件：无
     * check点：1. 生成Bean成功；2. 返回值正确
     */
    @Test
    public void should_returnCorrectValue_when_createServerProperties() throws Exception {
        SslConfiguration spy = PowerMockito.spy(new SslConfiguration());
        Ssl ssl = spy.ssl();
        EncryptorRestClient client = Mockito.mock(EncryptorRestClient.class);
        PlaintextVo plaintextVo = new PlaintextVo();
        plaintextVo.setPlaintext("123");
        Mockito.when(client.decrypt(any())).thenReturn(plaintextVo);
        ReflectionTestUtils.setField(spy, "keyStorePwdFile", "internal_cert.txt");

        PowerMockito.when(spy, "getKeyStoreAuth").thenReturn("123");

        ServerProperties serverProperties = spy.serverProperties(ssl, client);
        assertThat(serverProperties).isNotNull();
        assertThat(serverProperties.getSsl()).isEqualTo(ssl);
        Ssl sslInServer = serverProperties.getSsl();
        assertThat(sslInServer.getClientAuth()).isEqualTo(Ssl.ClientAuth.NONE);
        assertThat(sslInServer.getTrustStorePassword()).isEqualTo("123");
        assertThat(sslInServer.getKeyStorePassword()).isEqualTo("123");
    }
}
