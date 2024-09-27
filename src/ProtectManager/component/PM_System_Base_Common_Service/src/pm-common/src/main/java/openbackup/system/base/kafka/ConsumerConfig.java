package openbackup.system.base.kafka;

import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.ConfigMapServiceImpl;
import openbackup.system.base.util.KeyToolUtil;
import openbackup.system.base.util.MessageTemplate;
import openbackup.system.base.util.ProviderRegistry;

import lombok.extern.slf4j.Slf4j;

import org.apache.kafka.clients.CommonClientConfigs;
import org.apache.kafka.clients.consumer.ConsumerRecord;
import org.apache.kafka.common.config.SaslConfigs;
import org.apache.kafka.common.config.SslConfigs;
import org.apache.kafka.common.security.auth.SecurityProtocol;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.autoconfigure.kafka.ConcurrentKafkaListenerContainerFactoryConfigurer;
import org.springframework.boot.autoconfigure.kafka.KafkaProperties;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Primary;
import org.springframework.kafka.annotation.EnableKafka;
import org.springframework.kafka.config.ConcurrentKafkaListenerContainerFactory;
import org.springframework.kafka.config.KafkaListenerContainerFactory;
import org.springframework.kafka.core.ConsumerFactory;
import org.springframework.kafka.core.DefaultKafkaConsumerFactory;
import org.springframework.kafka.listener.ContainerProperties;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.retry.RetryContext;
import org.springframework.retry.backoff.ExponentialBackOffPolicy;
import org.springframework.retry.support.RetryTemplate;

import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Optional;

/**
 * kafka消费者配置类
 *
 * @author y00559272
 * @since 2021-04-09
 */
@Slf4j
@Configuration
@EnableKafka
public class ConsumerConfig {
    /**
     * Redis密码在secret中的key
     */
    public static final String KAFKA_AUTH_KEY = "kafka.password";

    private static final int CONCURRENCY = 4;

    private static final int HEARTBEAT_TIME = 3000;

    @Value("${spring.kafka.bootstrap-servers:infrastructure-zk-kafka:9092}")
    private String servers;

    @Value("${spring.kafka.properties.session.timeout.ms}")
    private String sessionTime;

    @Value("${spring.kafka.ssl.protocol:TLSv1.2}")
    private String protocol;

    @Value("${spring.kafka.ssl.key-store-location:/opt/OceanProtect/infrastructure/cert/internal/internal.ks}")
    private String keyStore;

    @Value("${spring.kafka.ssl.key-store-type:PKCS12}")
    private String keyStoreType;

    @Value("${spring.kafka.properties.sasl.mechanism:PLAIN}")
    private String mechanism;

    /**
     * KeyStore密钥文件
     */
    @Value("${server.ssl.key-store-password-file:/opt/OceanProtect/infrastructure/cert/internal/internal_cert}")
    private String keyStorePwdFile;

    /**
     * kafka服务端确认的消费超时时间
     */
    @Value("${spring.kafka.properties.max.poll.interval.ms}")
    private int maxPollIntervalMs;

    @Autowired
    private KeyToolUtil keyToolUtil;

    @Autowired
    private ProviderRegistry providerRegistry;

    @Autowired
    private MessageTemplate<String> messageTemplate;

    @Autowired
    private KafkaProperties kafkaProperties;

    @Autowired
    private ConfigMapServiceImpl configMapService;

    /**
     * kafka配置类
     *
     * @return KafkaListenerContainerFactory
     */
    @Primary
    @Bean
    public KafkaListenerContainerFactory<?> batchFactory() {
        ConcurrentKafkaListenerContainerFactory<Integer, String> factory
            = new ConcurrentKafkaListenerContainerFactory<>();
        factory.setConsumerFactory(consumerFactory());
        factory.setBatchListener(true);
        factory.setAutoStartup(false);
        factory.setConcurrency(CONCURRENCY);
        factory.getContainerProperties().setAckMode(ContainerProperties.AckMode.MANUAL_IMMEDIATE);
        return factory;
    }

    /**
     * 消费者工厂配置实例
     *
     * @return 消费者工厂实例
     */
    @Primary
    @Bean
    @ExterAttack
    public ConsumerFactory<Object, Object> consumerFactory() {
        KafkaProperties.Ssl ssl = kafkaProperties.getSsl();
        Map<String, Object> props = ssl.buildProperties();
        props.put(org.apache.kafka.clients.consumer.ConsumerConfig.BOOTSTRAP_SERVERS_CONFIG, servers);
        props.put(org.apache.kafka.clients.consumer.ConsumerConfig.ENABLE_AUTO_COMMIT_CONFIG, false);
        props.put(org.apache.kafka.clients.consumer.ConsumerConfig.ALLOW_AUTO_CREATE_TOPICS_CONFIG, true);
        props.put(org.apache.kafka.clients.consumer.ConsumerConfig.SESSION_TIMEOUT_MS_CONFIG, sessionTime);
        props.put(org.apache.kafka.clients.consumer.ConsumerConfig.MAX_POLL_RECORDS_CONFIG, 1);
        props.put(org.apache.kafka.clients.consumer.ConsumerConfig.AUTO_OFFSET_RESET_CONFIG, "earliest");
        props.put(org.apache.kafka.clients.consumer.ConsumerConfig.KEY_DESERIALIZER_CLASS_CONFIG,
            org.apache.kafka.common.serialization.StringDeserializer.class);
        props.put(org.apache.kafka.clients.consumer.ConsumerConfig.VALUE_DESERIALIZER_CLASS_CONFIG,
            org.apache.kafka.common.serialization.StringDeserializer.class);
        props.put(org.apache.kafka.clients.consumer.ConsumerConfig.HEARTBEAT_INTERVAL_MS_CONFIG, HEARTBEAT_TIME);
        props.put(org.apache.kafka.clients.consumer.ConsumerConfig.MAX_POLL_INTERVAL_MS_CONFIG, maxPollIntervalMs);

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
        return new DefaultKafkaConsumerFactory<>(props);
    }

    /**
     * kafka自定义配置工厂
     *
     * @param factoryConfigurer 当前工厂配置
     * @return 自定listener配置
     */
    @Bean("retryFactory")
    public ConcurrentKafkaListenerContainerFactory<?, ?> kafkaListenerContainerFactory(
        ConcurrentKafkaListenerContainerFactoryConfigurer factoryConfigurer) {
        ConcurrentKafkaListenerContainerFactory<Object, Object> factory
            = new ConcurrentKafkaListenerContainerFactory<>();
        factory.setConcurrency(CONCURRENCY);
        factoryConfigurer.configure(factory, consumerFactory());
        factory.setRetryTemplate(kafkaRetry());
        factory.setRecoveryCallback(this::onRecovery);
        factory.setAutoStartup(false);
        return factory;
    }

    private Optional<Object> onRecovery(RetryContext retryContext) {
        Throwable throwable = retryContext.getLastThrowable();
        Optional.ofNullable(throwable)
            .map(Throwable::getCause)
            .ifPresent(ex -> log.error("Recovery Callback, retry count:[{}]", retryContext.getRetryCount(), ex));

        ConsumerRecord<?, ?> record = getAttribute(retryContext, "record", ConsumerRecord.class).orElse(null);
        if (record == null) {
            log.warn("Consumer config find record is not exists!");
            return Optional.empty();
        }
        String topic = record.topic();
        try {
            handle(record, topic, throwable);
        } catch (Throwable thr) {
            log.error("Handle message error!", thr);
        } finally {
            acknowledge(retryContext, topic);
        }
        return Optional.empty();
    }

    private void handle(ConsumerRecord<?, ?> record, String topic, Throwable throwable) {
        MessageErrorHandler handler = providerRegistry.findProvider(MessageErrorHandler.class, throwable, null);
        if (handler != null) {
            String message = Optional.ofNullable(record.value()).map(Object::toString).orElse("{}");
            handler.handle(topic, message, throwable);
        } else {
            log.info("not found error handler for topic({})", topic);
        }
    }

    private void acknowledge(RetryContext retryContext, String topic) {
        log.info("commit message after retry, topic: {}", topic);
        getAttribute(retryContext, "acknowledgment", Acknowledgment.class).ifPresent(Acknowledgment::acknowledge);
    }

    private <T> Optional<T> getAttribute(RetryContext retryContext, String name, Class<T> type) {
        Object attribute = retryContext.getAttribute(name);
        if (type.isInstance(attribute)) {
            return Optional.of(type.cast(attribute));
        } else {
            return Optional.empty();
        }
    }

    private RetryTemplate kafkaRetry() {
        RetryTemplate retryTemplate = new RetryTemplate();
        ExponentialBackOffPolicy exponentialBackOffPolicy = new ExponentialBackOffPolicy();
        // 最大重试间隔，1分钟
        exponentialBackOffPolicy.setMaxInterval(
            IsmNumberConstant.ONE * IsmNumberConstant.SIXTY * IsmNumberConstant.THOUSAND);
        // 初始重试间隔，1分钟
        exponentialBackOffPolicy.setInitialInterval(
            IsmNumberConstant.ONE * IsmNumberConstant.SIXTY * IsmNumberConstant.THOUSAND);
        // 后续重试间隔=上次重试间隔 * 1
        exponentialBackOffPolicy.setMultiplier(1);
        retryTemplate.setBackOffPolicy(exponentialBackOffPolicy);
        Map<Class<? extends Throwable>, Boolean> retryableExceptions = new HashMap<>();

        // 指定不重试的异常类，业务异常无需重试
        providerRegistry.findProviders(MessageErrorHandler.class)
            .stream()
            .map(MessageErrorHandler::retryableExceptions)
            .forEach(retryableExceptions::putAll);
        retryTemplate.setRetryPolicy(new ExceptionRetryPolicy(IsmNumberConstant.TWO, retryableExceptions, false));
        retryTemplate.setThrowLastExceptionOnExhausted(true);
        return retryTemplate;
    }
}
