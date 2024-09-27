package openbackup.system.base.sdk.agent.config;

import openbackup.system.base.common.rest.CommonDecoder;
import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.config.FeignClientConfig;

import feign.Feign;
import feign.codec.Decoder;
import feign.codec.ErrorDecoder;
import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Bean;

/**
 * 功能描述
 *
 * @author s00455050
 * @since 2021-08-16
 */
@Slf4j
public class AgentConfiguration {
    @Autowired
    private FeignClientConfig feignClientConfig;

    /**
     * Feign builder feign . builder.
     *
     * @return the feign . builder
     */
    @Bean("AgentConfigBuilder")
    public Feign.Builder feignBuilder() {
        return FeignBuilder.getDefaultRetryableBuilder().client(feignClientConfig.getInternalClient());
    }

    /**
     * errorDecoder
     *
     * @return errorDecoder
     */
    @Bean
    public ErrorDecoder errorDecoder() {
        return CommonDecoder::errorDecode;
    }

    /**
     * docoder
     *
     * @return docoder
     */
    @Bean
    public Decoder decoder() {
        return CommonDecoder.decoder();
    }
}
