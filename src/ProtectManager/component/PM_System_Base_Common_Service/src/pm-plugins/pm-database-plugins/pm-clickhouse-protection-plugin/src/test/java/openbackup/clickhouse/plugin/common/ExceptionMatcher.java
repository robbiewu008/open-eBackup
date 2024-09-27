package openbackup.clickhouse.plugin.common;

import openbackup.system.base.common.exception.LegoCheckedException;

import org.hamcrest.Description;
import org.hamcrest.TypeSafeMatcher;

/**
 * 自定义异常匹配器
 *
 * @author q00464130
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-13
 */
public class ExceptionMatcher extends TypeSafeMatcher<LegoCheckedException> {
    private final long errorCode;

    private final String message;

    public ExceptionMatcher(long errorCode, String message) {
        this.errorCode = errorCode;
        this.message = message;
    }

    @Override
    public boolean matchesSafely(LegoCheckedException item) {
        return item.getErrorCode() == errorCode && item.getMessage().equals(message);
    }

    @Override
    public void describeTo(Description description) {
        description.appendText("expects errorCode ")
            .appendValue(errorCode)
            .appendText(",expects message ")
            .appendValue(message);
    }
}
