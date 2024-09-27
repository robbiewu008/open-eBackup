package openbackup.data.access.framework.core.security.journal;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Logging Context
 *
 * @author l00272247
 * @since 2021-12-17
 */
public class LoggingContext {
    /**
     * EVALUATIONS
     */
    public static final String EVALUATIONS = "evaluations";

    /**
     * PARAMS
     */
    public static final String PARAMS = "params";

    /**
     * CONTEXT_REGISTRATIONS
     */
    public static final String CONTEXT_REGISTRATIONS = "contextRegistrations";

    private final List<?> args;
    private final Map<String, Object> data;

    /**
     * constructor
     *
     * @param args args
     */
    public LoggingContext(List<?> args) {
        this.args = args;
        data = new HashMap<>();
    }

    public List<?> getArgs() {
        return args;
    }

    /**
     * set value by field
     *
     * @param field field
     * @param value value
     * @param <T> template type T
     * @return context object
     */
    public <T> LoggingContext set(String field, T value) {
        data.put(field, value);
        return this;
    }

    /**
     * get value by field
     *
     * @param field field
     * @param <T> template type
     * @return value
     */
    public <T> T get(String field) {
        return (T) data.get(field);
    }
}
