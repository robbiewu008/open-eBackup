package openbackup.system.base.common.aspect;

import org.springframework.stereotype.Component;

/**
 * Number Converter
 *
 * @author l00272247
 * @since 2021-06-10
 */
@Component
public class NumberConverter extends AbstractConverter {
    /**
     * constructor
     */
    public NumberConverter() {
        super("number");
    }

    /**
     * data cast
     *
     * @param data data
     * @return result
     */
    @Override
    protected Object cast(Object data) {
        if (data == null || data instanceof Number) {
            return data;
        }
        if (data instanceof CharSequence) {
            String text = data.toString();
            if (text.matches("\\d+")) {
                return Long.valueOf(text);
            } else if (text.matches("(\\d+)?\\.\\d+")) {
                return Double.valueOf(text);
            } else {
                return null;
            }
        }
        return null;
    }
}
