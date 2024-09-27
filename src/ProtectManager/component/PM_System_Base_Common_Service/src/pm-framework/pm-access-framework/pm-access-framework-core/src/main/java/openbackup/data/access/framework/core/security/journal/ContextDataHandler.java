package openbackup.data.access.framework.core.security.journal;

import java.util.List;
import java.util.function.Consumer;

/**
 * Define Cache Callback
 *
 * @author l00272247
 * @since 2021-12-14
 */
public interface ContextDataHandler {
    /**
     * handler method
     *
     * @param loggingContexts logging context
     * @param callback callback
     * @return result
     * @throws Throwable throwable
     */
    Object handle(List<LoggingContext> loggingContexts, Consumer<Object> callback) throws Throwable;
}
