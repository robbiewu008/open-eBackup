package openbackup.system.base.sdk.storage;

import openbackup.system.base.common.model.storage.StorageResponse;
import openbackup.system.base.common.model.storage.StorageSession;
import openbackup.system.base.common.rest.FeignBuilder;

import feign.Client;
import feign.Contract;
import feign.Feign;
import feign.FeignException;
import feign.RequestInterceptor;
import feign.RequestTemplate;
import feign.Response;
import feign.codec.Decoder;
import lombok.extern.slf4j.Slf4j;

import org.apache.hc.client5.http.ssl.NoopHostnameVerifier;
import org.apache.hc.core5.ssl.SSLContextBuilder;
import org.springframework.beans.factory.ObjectFactory;
import org.springframework.boot.autoconfigure.http.HttpMessageConverters;
import org.springframework.cloud.openfeign.support.ResponseEntityDecoder;
import org.springframework.cloud.openfeign.support.SpringDecoder;
import org.springframework.context.annotation.Bean;
import org.springframework.http.MediaType;
import org.springframework.http.converter.json.MappingJackson2HttpMessageConverter;

import java.io.IOException;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import javax.net.ssl.SSLContext;

/**
 * 功能描述
 *
 * @author y00413474
 * @version [BCManager 8.0.0]
 * @since 2020-06-19
 */
@Slf4j
public class DoradoFeignConfiguration implements RequestInterceptor {
    /**
     * 自定义request拦截
     *
     * @return Feign.Builder
     */
    @Bean("DoradoFeignBuilder")
    public Feign.Builder feignBuilder() {
        Client client;
        try {
            SSLContext context = new SSLContextBuilder().loadTrustMaterial(null, (chain, authType) -> true).build();
            client = new Client.Default(context.getSocketFactory(), new NoopHostnameVerifier());
        } catch (Exception e) {
            log.error("Create feign client with SSL config failed", e);
            client = new Client.Default(null, null);
        }
        return FeignBuilder.getDefaultRetryableBuilder().client(client);
    }

    /**
     * 将契约改为feign原生的默认契约。这样就可以使用feign自带的注解了。
     * 用feign.Contract.Default替换SpringMvcContract契约
     *
     * @return 默认的feign契约
     */
    @Bean
    public Contract feignContract() {
        // Spring Cloud Netflix默认的SpringMvcController将替换为feign.Contract.Default。
        return new Contract.Default();
    }

    /**
     * Interceptor
     *
     * @param template RequestTemplate
     */
    @Override
    public void apply(RequestTemplate template) {
    }

    /**
     * 自定义MyDecoder
     *
     * @author y00413474
     * @version [BCManager 8.0.0]
     * @since 2020-06-19
     */
    private class MyDecoder extends SpringDecoder {
        /**
         * MyDecoder
         *
         * @param messageConverters messageConverters
         */
        MyDecoder(ObjectFactory<HttpMessageConverters> messageConverters) {
            super(messageConverters);
        }

        @Override
        public Object decode(Response response, Type type) throws IOException, FeignException {
            Object object = super.decode(response, type);
            if (object instanceof StorageResponse) {
                StorageResponse<?> storageResponse = (StorageResponse<?>) object;
                if (storageResponse.getData() instanceof StorageSession) {
                    StorageSession storageSession = (StorageSession) storageResponse.getData();
                    Collection<String> list = response.headers().get("Set-Cookie");
                    if (list != null && !list.isEmpty()) {
                        storageSession.setCookie(list.iterator().next());
                    }
                }
            }
            return object;
        }
    }

    /**
     * feignDecoder
     *
     * @return feignDecoder
     */
    @Bean
    public Decoder feignDecoder() {
        return new ResponseEntityDecoder(new MyDecoder(feignHttpMessageConverter()));
    }

    private ObjectFactory<HttpMessageConverters> feignHttpMessageConverter() {
        final HttpMessageConverters httpMessageConverters =
            new HttpMessageConverters(new PhpMappingJackson2HttpMessageConverter());
        return () -> httpMessageConverters;
    }

    /**
     * 功能描述
     *
     * @author y00413474
     * @version [BCManager 8.0.0]
     * @since 2020-06-19
     */
    private static class PhpMappingJackson2HttpMessageConverter extends MappingJackson2HttpMessageConverter {
        PhpMappingJackson2HttpMessageConverter() {
            List<MediaType> mediaTypes = new ArrayList<>();
            mediaTypes.add(MediaType.valueOf(MediaType.TEXT_HTML_VALUE + ";charset=UTF-8"));
            mediaTypes.add(MediaType.valueOf(MediaType.TEXT_PLAIN_VALUE + ";charset=UTF-8"));
            setSupportedMediaTypes(mediaTypes);
        }
    }
}
