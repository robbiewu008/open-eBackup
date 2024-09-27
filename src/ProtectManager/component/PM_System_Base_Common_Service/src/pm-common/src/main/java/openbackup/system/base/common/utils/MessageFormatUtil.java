package openbackup.system.base.common.utils;

import java.text.MessageFormat;

/**
 * MessageFormatUtil
 *
 * @author p00171530
 * @version [Lego V100R002C10, 2014-12-18]
 * @since 2019-11-01
 */
public class MessageFormatUtil {
    private MessageFormatUtil() {
    }

    /**
     * format
     *
     * @param pattern pattern
     * @param arguments arguments
     * @return String
     */
    public static synchronized String format(String pattern, Object[] arguments) {
        MessageFormat format = new MessageFormat(pattern);
        return format.format(arguments);
    }
}
