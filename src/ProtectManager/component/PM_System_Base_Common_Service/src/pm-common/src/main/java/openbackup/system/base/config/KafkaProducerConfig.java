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

import openbackup.system.base.service.ConfigMapServiceImpl;
import openbackup.system.base.util.KeyToolUtil;

import org.apache.kafka.clients.CommonClientConfigs;
import org.apache.kafka.clients.producer.ProducerConfig;
import org.apache.kafka.common.config.SaslConfigs;
import org.apache.kafka.common.config.SslConfigs;
import org.apache.kafka.common.security.auth.SecurityProtocol;
import org.apache.kafka.common.serialization.StringSerializer;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.autoconfigure.kafka.KafkaProperties;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.kafka.annotation.EnableKafka;
import org.springframework.kafka.core.DefaultKafkaProducerFactory;
import org.springframework.kafka.core.KafkaTemplate;
import org.springframework.kafka.core.ProducerFactory;

import java.util.Locale;
import java.util.Map;

/**
 * get the kafka properties from spring &
 * return kafka templete as  bean
 *
 * @author y30000858
 * @since 2020-07-30
 */
@Configuration
@EnableKafka
public class KafkaProducerConfig {
    /**
     * Redis密码在secret中的key
     */
    public static final String KAFKA_AUTH_KEY = "kafka.password";

    @Value("${spring.kafka.bootstrap-servers:infrastructure-zk-kafka:9092}")
    private String servers;

    @Value("${spring.kafka.producer.retries}")
    private int retries;

    @Value("${spring.kafka.producer.batch-size}")
    private int batchSize;

    @Value("${spring.kafka.producer.properties.linger.ms}")
    private int linger;

    @Value("${spring.kafka.producer.buffer-memory}")
    private int bufferMemory;

    @Value("${spring.kafka.ssl.protocol:TLSv1.2}")
    private String protocol;

    @Value("${spring.kafka.ssl.key-store-location:/opt/OceanProtect/infrastructure/cert/internal/internal.ks}")
    private String keyStore;

    @Value("${spring.kafka.ssl.key-store-type:PKCS12}")
    private String keyStoreType;

    @Value("${spring.kafka.properties.sasl.mechanism:PLAIN}")
    private String mechanism;

    @Value("${spring.kafka.producer.properties.max.request.size}")
    private String maxRequestSize;

    /**
     * KeyStore密钥文件
     */
    @Value("${server.ssl.key-store-password-file:/opt/OceanProtect/infrastructure/cert/internal/internal_cert}")
    private String keyStorePwdFile;

    @Autowired
    private KeyToolUtil keyToolUtil;

    @Autowired
    private KafkaProperties kafkaProperties;

    @Autowired
    private ConfigMapServiceImpl configMapService;

    /**
     * set the key-value map for kafka templete from spring
     *
     * @return map
     */
    public Map<String, Object> producerConfigs() {
        Map<String, Object> props = kafkaProperties.getSsl().buildProperties();
        props.put(ProducerConfig.BOOTSTRAP_SERVERS_CONFIG, servers);
        props.put(ProducerConfig.RETRIES_CONFIG, retries);
        props.put(ProducerConfig.BATCH_SIZE_CONFIG, batchSize);
        props.put(ProducerConfig.LINGER_MS_CONFIG, linger);
        props.put(ProducerConfig.MAX_REQUEST_SIZE_CONFIG, maxRequestSize);
        props.put(ProducerConfig.BUFFER_MEMORY_CONFIG, bufferMemory);
        props.put(ProducerConfig.KEY_SERIALIZER_CLASS_CONFIG, StringSerializer.class);
        props.put(ProducerConfig.VALUE_SERIALIZER_CLASS_CONFIG, StringSerializer.class);

        // SSL相关配置
        String password = keyToolUtil.getKeyStorePassword(keyStorePwdFile);
        props.put(SslConfigs.SSL_KEYSTORE_PASSWORD_CONFIG, password);
        props.put(SslConfigs.SSL_TRUSTSTORE_PASSWORD_CONFIG, password);
        props.put(SslConfigs.SSL_PROTOCOL_CONFIG, protocol);
        props.put(CommonClientConfigs.SECURITY_PROTOCOL_CONFIG, SecurityProtocol.SASL_SSL.name);
        props.put(SslConfigs.SSL_KEYSTORE_LOCATION_CONFIG, keyStore);
        props.put(SslConfigs.SSL_KEYSTORE_TYPE_CONFIG, keyStoreType);
        props.put(SslConfigs.SSL_TRUSTSTORE_LOCATION_CONFIG, keyStore);
        props.put(SslConfigs.SSL_TRUSTSTORE_TYPE_CONFIG, keyStoreType);
        props.put(SslConfigs.SSL_ENDPOINT_IDENTIFICATION_ALGORITHM_CONFIG, "");
        props.put(SaslConfigs.SASL_MECHANISM, mechanism);
        String kafkaPassword = configMapService.getValueFromSecretByKey(KAFKA_AUTH_KEY);
        String jaasConfig = String.format(Locale.ENGLISH,
            "org.apache.kafka.common.security.plain.PlainLoginModule required username='kafka_usr' password='%s';",
            kafkaPassword);
        props.put(SaslConfigs.SASL_JAAS_CONFIG, jaasConfig);
        return props;
    }

    /**
     * get the properties
     *
     * @return ProducerFactory
     */
    public ProducerFactory<String, String> producerFactory() {
        return new DefaultKafkaProducerFactory<>(producerConfigs());
    }

    /**
     * configuration with kafka
     *
     * @return KafkaTemplate as a bean
     */
    @Bean
    public KafkaTemplate<String, String> kafkaTemplate() {
        return new KafkaTemplate<>(producerFactory());
    }
}
