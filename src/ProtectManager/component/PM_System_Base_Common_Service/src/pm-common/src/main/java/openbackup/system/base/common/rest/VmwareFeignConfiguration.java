package openbackup.system.base.common.rest;

import openbackup.system.base.config.FeignClientConfig;
import openbackup.system.base.service.SensitiveDataEliminateService;

import feign.Feign;
import feign.RequestInterceptor;
import feign.RequestTemplate;
import feign.codec.Decoder;
import feign.codec.ErrorDecoder;
import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Bean;

/**
 * VMware Feign Configuration
 *
 * @author z30047175
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2024-01-12
 */
@Slf4j
public class VmwareFeignConfiguration implements RequestInterceptor {
    @Autowired
    private SensitiveDataEliminateService sensitiveDataEliminateService;

    @Autowired
    private FeignClientConfig feignClientConfig;

    /**
     * 自定义request拦截
     *
     * @return Feign.Builder
     */
    @Bean("vmwareBuilder")
    public Feign.Builder feignBuilder() {
        return FeignBuilder.getVmwareFeignBuilder().client(feignClientConfig.getInternalClient());
    }

    /**
     * decoder
     *
     * @return decoder
     */
    @Bean
    public static Decoder decoder() {
        return CommonDecoder.decoder();
    }

    /**
     * errorDecoder
     *
     * @return errorDecoder
     */
    @Bean
    public ErrorDecoder errorDecoder() {
        return CommonDecoder::retryableErrorDecode;
    }

    /**
     * retryer
     *
     * @return retryer
     */
    @Bean
    public CommonRetryer<Long> retryer() {
        return CommonRetryer.create();
    }

    @Override
    public void apply(RequestTemplate requestTemplate) {
        String url = sensitiveDataEliminateService.eliminateUrl(requestTemplate.url());
        log.debug("send request. method: {}, url: {}.", requestTemplate.method(), url);
    }
}
