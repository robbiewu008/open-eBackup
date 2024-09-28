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

import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.util.EncryptorRestClient;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.io.FileUtils;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.autoconfigure.web.ServerProperties;
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.boot.web.server.Ssl;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Primary;

import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

/**
 * 服务证书认证配置类
 *
 */
@Slf4j
@Configuration
public class SslConfiguration {
    /**
     * KeyStore密钥文件
     */
    @Value("${server.ssl.key-store-password-file}")
    private String keyStorePwdFile;

    /**
     * 注入server.ssl配置
     *
     * @return server.ssl配置
     */
    @Bean
    @ConfigurationProperties(prefix = "server.ssl")
    public Ssl ssl() {
        return new Ssl();
    }

    /**
     * Feign服务端配置
     *
     * @param ssl 服务端SSL配置
     * @param encryptorRestClient 解密REST服务
     * @return Feign服务端配置
     */
    @Primary
    @Bean
    public ServerProperties serverProperties(Ssl ssl, EncryptorRestClient encryptorRestClient) {
        // 通信keystore密码
        String pwd = encryptorRestClient.decrypt(getKeyStoreAuth()).getPlaintext();
        ssl.setKeyStorePassword(pwd);

        // 信任库密码
        ssl.setTrustStorePassword(pwd);

        // 关闭客户端证书校验
        ssl.setClientAuth(Ssl.ClientAuth.NONE);

        ServerProperties serverProperties = new ServerProperties();
        serverProperties.setSsl(ssl);
        return serverProperties;
    }

    /**
     * 从文件中读取密钥
     *
     * @return keystore密钥
     */
    @ExterAttack
    private String getKeyStoreAuth() {
        String authInfo = "";
        try {
            authInfo = FileUtils.readFileToString(new File(keyStorePwdFile), StandardCharsets.UTF_8);
        } catch (IOException ioException) {
            log.error("fail to get key store auth info from file, exception: {}", ioException.getMessage());
        }
        return authInfo;
    }
}
