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
package openbackup.system.base.config;

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.infrastructure.InfrastructureService;
import openbackup.system.base.service.ConfigMapServiceImpl;
import openbackup.system.base.util.KeyToolUtil;

import lombok.extern.slf4j.Slf4j;

import org.redisson.Redisson;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.Codec;
import org.redisson.codec.FstCodec;
import org.redisson.codec.JsonJacksonCodec;
import org.redisson.config.BaseConfig;
import org.redisson.config.ClusterServersConfig;
import org.redisson.config.Config;
import org.redisson.config.SingleServerConfig;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Lazy;

import java.io.File;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.List;
import java.util.Optional;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * RedissonClient https config
 *
 */
@Slf4j
@Configuration
public class RedissonClientConfig {
    /**
     * Redis密码在secret中的key
     */
    private static final String REDIS_AUTH_KEY = "redis.password";

    private static final String CLUSTER_CONFIG_MAP = "multicluster-conf";

    private static final String REDIS_CLUSTER_KEY = "REDIS_CLUSTER";

    private static final Pattern SERVER_ADDRESS_PATTERN = Pattern.compile(
        "(?<protocol>rediss?)://(?<address>[a-zA-Z0-9._-]+):(?<port>[0-9]+)");

    /**
     * 循环查询Redis密码的时间间隔
     */
    private static final int SLEEP_TIME = 1000;

    /**
     * Redis服务端URL
     */
    @Value("${spring.redis.url}")
    private String serverAddress;

    @Value("${spring.redis.cluster.nodes}")
    private List<String> clusterServerAddress;

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

    @Autowired
    private KeyToolUtil keyToolUtil;

    @Autowired
    private ConfigMapServiceImpl configMapService;

    @Autowired
    private InfrastructureService infrastructureService;

    /**
     * 注册 RedissonClient
     *
     * @return RedissonClient redisson客户端
     * @throws MalformedURLException URL转换异常
     * @throws InterruptedException 中断异常
     */
    @Lazy
    @Bean
    public RedissonClient redissonClient() throws InterruptedException, MalformedURLException {
        return redissonClient(new FstCodec());
    }

    /**
     * 获取使用json编码的redissonClient
     *
     * @return RedissonClient RedissonClient
     * @throws InterruptedException URL转换异常
     * @throws MalformedURLException 中断异常
     */
    public RedissonClient redissonClientJsonCodec() throws InterruptedException, MalformedURLException {
        return redissonClient(new JsonJacksonCodec());
    }

    private RedissonClient redissonClient(Codec codec) throws InterruptedException, MalformedURLException {
        String password;
        do {
            password = configMapService.getValueFromSecretByKey(REDIS_AUTH_KEY);
            if (VerifyUtil.isEmpty(password)) {
                log.error("NA. with redis auth info.");
                Thread.sleep(SLEEP_TIME); // 死循环中降低CPU占用
            }
        } while (VerifyUtil.isEmpty(password));
        Config config = new Config();
        config.setCodec(codec);
        if (isRedisCluster()) {
            ClusterServersConfig serverConfig = config.useClusterServers();
            serverConfig.setNodeAddresses(
                clusterServerAddress.stream().map(this::convertServerAddress).collect(Collectors.toList()));
            serverConfig.setMasterConnectionPoolSize(100);
            serverConfig.setMasterConnectionPoolSize(50);
            setCommonServerConfig(serverConfig, password);
        } else {
            SingleServerConfig serverConfig = config.useSingleServer();
            serverConfig.setAddress(convertServerAddress(serverAddress));
            serverConfig.setConnectionPoolSize(100);
            serverConfig.setConnectionPoolSize(50);
            setCommonServerConfig(serverConfig, password);
        }
        return Redisson.create(config);
    }

    private boolean isRedisCluster() {
        Optional<JSONObject> redisCluster = infrastructureService.getConfigMapValue(CLUSTER_CONFIG_MAP,
            REDIS_CLUSTER_KEY);
        log.info("get redis cluster established: {}", redisCluster.map(JSONObject::toString).orElse("null"));
        return redisCluster.map(jsonObject -> jsonObject.getBoolean(REDIS_CLUSTER_KEY, false)).orElse(false);
    }

    private void setCommonServerConfig(BaseConfig<?> serverConfig, String password) throws MalformedURLException {
        URL url = new File(keyStoreFile).toURI().toURL();
        serverConfig.setSslKeystore(url);
        String pwd = keyToolUtil.getKeyStorePassword(keyStorePwdFile);
        serverConfig.setSslKeystorePassword(pwd);
        serverConfig.setSslTruststore(url);
        serverConfig.setSslTruststorePassword(pwd);

        // 不进行域名校验
        serverConfig.setSslEnableEndpointIdentification(false);
        serverConfig.setPassword(password);
        serverConfig.setPingConnectionInterval(30000);
        serverConfig.setTimeout(10000);
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
}
