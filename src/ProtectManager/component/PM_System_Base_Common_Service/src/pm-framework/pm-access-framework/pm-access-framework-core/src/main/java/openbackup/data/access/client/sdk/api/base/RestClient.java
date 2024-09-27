package openbackup.data.access.client.sdk.api.base;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Indexed;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * RestClient
 *
 * @author l00272247
 * @since 2020-08-12
 */
@Target({ElementType.FIELD, ElementType.ANNOTATION_TYPE})
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Indexed
@Autowired
public @interface RestClient {
    /**
     * value
     *
     * @return value
     */
    String[] value() default {};

    /**
     * enhancer
     *
     * @return enhancer
     */
    Class<? extends RequestEnhanceService> enhancer() default RequestEnhanceService.class;
}
