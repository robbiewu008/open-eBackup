package openbackup.system.base.config;

import org.springframework.boot.autoconfigure.jackson.Jackson2ObjectMapperBuilderCustomizer;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import java.util.TimeZone;

/**
 * JacksonConfiguration
 *
 * @author l00272247
 * @since 2020-01-16
 */
@Configuration
public class JacksonConfiguration {
    /**
     * Jackson2ObjectMapperBuilderCustomizer
     *
     * @return Jackson2ObjectMapperBuilderCustomizer
     */
    @Bean
    public Jackson2ObjectMapperBuilderCustomizer getJackson2ObjectMapperBuilderCustomizer() {
        return jacksonObjectMapperBuilder -> jacksonObjectMapperBuilder.timeZone(TimeZone.getDefault());
    }
}
