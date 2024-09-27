package openbackup.system.base.util;

import java.util.Locale;
import java.util.Objects;
import java.util.Optional;

/**
 * 请求参数过滤处理
 *
 * @author nwx1077006
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-21
 */
public class RequestParamFilterUtil {
    /**
     * 通配符处理
     *
     * @param value 入参
     * @param <T> 类型
     * @return 处理后处理
     */
    public static <T> String escape(T value) {
        return Optional.ofNullable(value)
            .map(Objects::toString)
            .map(str -> str.toLowerCase(Locale.ENGLISH))
            .map(str -> str.replaceAll("\\\\", "\\\\"))
            .map(str -> str.replaceAll("%", "\\\\%"))
            .map(str -> str.replaceAll("\\?", "\\\\?"))
            .map(str -> str.replaceAll("_", "\\\\_"))
            .orElse(null);
    }
}
