package openbackup.system.base.kafka;

import openbackup.system.base.util.Applicable;

import java.util.Collections;
import java.util.Map;

/**
 * Message Error Handler
 *
 * @author l00272247
 * @since 2021-04-15
 */
public interface MessageErrorHandler extends Applicable<Throwable> {
    /**
     * retryable exceptions
     *
     * @return retryable exceptions
     */
    default Map<Class<? extends Throwable>, Boolean> retryableExceptions() {
        return Collections.singletonMap(Throwable.class, Boolean.TRUE);
    }

    /**
     * handle error message
     *
     * @param topic topic
     * @param message message
     * @param throwable throwable
     */
    void handle(String topic, String message, Throwable throwable);
}
