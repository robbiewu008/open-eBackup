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

import com.huawei.emeistor.console.config.datasource.DataSourceConfigService;
import com.huawei.emeistor.console.config.datasource.DataSourceRes;
import com.huawei.emeistor.console.config.datasource.JSONArray;
import com.huawei.emeistor.console.config.datasource.JSONObject;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.util.EncryptorRestClient;
import com.huawei.emeistor.console.util.VerifyUtil;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.io.FileUtils;
import org.redisson.Redisson;
import org.redisson.api.RedissonClient;
import org.redisson.codec.FstCodec;
import org.redisson.config.BaseConfig;
import org.redisson.config.ClusterServersConfig;
import org.redisson.config.Config;
import org.redisson.config.SingleServerConfig;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.util.StringUtils;

import java.io.File;
import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.MalformedURLException;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * RedissonClient https config
 *
 * @author xwx1016404
 * @since 2021-12-28
 */
@Slf4j
@Configuration
public class RedissonClientConfig {
    /**
     * 循环查询Redis密码的时间间隔
     */
    private static final int SLEEP_TIME = 1000;

    private static final Pattern SERVER_ADDRESS_PATTERN = Pattern.compile(
        "(?<protocol>rediss?)://(?<address>[a-zA-Z0-9._-]+):(?<port>[0-9]+)");

    private static final String REDIS_CONFIG_KEY = "REDIS_CLUSTER";

    /**
     * Redis服务端URL
     */
    @Value("${spring.redis.url}")
    private String serverAddress;

    /**
     * Redis cluster 服务端URL
     */
    @Value("${spring.redis.cluster.nodes}")
    private List<String> serverNodes;

    /**
     * KeyStore文件
     */
    @Value("${spring.redis.key-store}")
    private String keyStoreFile;

    /**
     * KeyStore密钥文件
     */
    @Value("${spring.redis.key-store-password-file}")
    private String keyStorePwdFile;

    /**
     * Redisson客户端netty线程池线程数量
     */
    @Value("${spring.redis.nettyThreads}")
    private int nettyThreads;

    @Autowired
    private EncryptorRestClient encryptorRestClient;

    @Autowired
    private DataSourceConfigService dataSourceConfigService;

    /**
     * 注册 RedissonClient
     *
     * @return RedissonClient redisson客户端
     * @throws MalformedURLException URL转换异常
     * @throws InterruptedException 中断异常
     */
    @Bean
    @ExterAttack
    public RedissonClient redissonClient() throws InterruptedException, MalformedURLException {
        String password;
        do {
            password = encryptorRestClient.getRedisAuthFromSecret();
            if (StringUtils.isEmpty(password)) {
                log.error("NA. with redis auth info.");
                Thread.sleep(SLEEP_TIME); // 死循环中降低CPU占用
            }
        } while (StringUtils.isEmpty(password));
        Config config = new Config();
        config.setCodec(new FstCodec());
        config.setNettyThreads(nettyThreads);
        if (isCluster()) {
            log.info("redis server is cluster");
            ClusterServersConfig serversConfig = config.useClusterServers();
            List<String> nodes = serverNodes.stream().map(this::convertServerAddress).collect(Collectors.toList());
            serversConfig.setNodeAddresses(nodes);
            setCommonConfig(serversConfig, password);
        } else {
            log.info("redis server is single instance");
            SingleServerConfig serverConfig = config.useSingleServer();
            serverConfig.setAddress(convertServerAddress(serverAddress));
            setCommonConfig(serverConfig, password);
        }
        return Redisson.create(config);
    }

    private void setCommonConfig(BaseConfig<?> serverConfig, String password) throws MalformedURLException {
        URL url = new File(keyStoreFile).toURI().toURL();
        serverConfig.setSslKeystore(url);
        String pwd = encryptorRestClient.decrypt(getKeyStoreAuth()).getPlaintext();
        serverConfig.setSslKeystorePassword(pwd);
        serverConfig.setSslTruststore(url);
        serverConfig.setSslTruststorePassword(pwd);

        // 不进行域名校验
        serverConfig.setSslEnableEndpointIdentification(false);
        serverConfig.setPassword(password);
        serverConfig.setPingConnectionInterval(30000);
        serverConfig.setTimeout(10000);
    }

    private String getKeyStoreAuth() {
        String authInfo = "";
        try {
            authInfo = FileUtils.readFileToString(new File(keyStorePwdFile), StandardCharsets.UTF_8);
        } catch (IOException ioException) {
            log.error("wrong with get redis auth info from system base");
        }
        return authInfo;
    }

    /**
     * 将域名转为IP
     *
     * @param serverAddress Redis地址
     * @return 转换后的地址
     */
    private String convertServerAddress(String serverAddress) {
        Matcher matcher = SERVER_ADDRESS_PATTERN.matcher(serverAddress);
        if (matcher.find()) {
            String address = matcher.group("address");
            int port = Integer.parseInt(matcher.group("port"));
            String protocol = matcher.group("protocol");
            InetSocketAddress inetSocketAddress = new InetSocketAddress(address, port);
            InetAddress address1 = inetSocketAddress.getAddress();
            String convertedAddress = protocol + "://" + address1.getHostAddress() + ":" + port;
            log.info("Redis server address convert from: {} to: {}", serverAddress, convertedAddress);
            return convertedAddress;
        }
        return serverAddress;
    }

    private boolean isCluster() {
        try {
            DataSourceRes redisClusterInfo = dataSourceConfigService.getRedisClusterInfo();
            log.info("get gauss information from infra.");
            if (VerifyUtil.isEmpty(redisClusterInfo.getData())) {
                return false;
            }
            JSONArray configMapEntry = redisClusterInfo.getData();
            for (int i = 0; i < configMapEntry.size(); i++) {
                JSONObject entry = configMapEntry.getJSONObject(i);
                if (!entry.containsKey(REDIS_CONFIG_KEY)) {
                    continue;
                }
                return entry.getBoolean(REDIS_CONFIG_KEY);
            }
            return false;
        } catch (FeignException exception) {
            log.error("wrong with get password from inf.");
            return false;
        }
    }
}