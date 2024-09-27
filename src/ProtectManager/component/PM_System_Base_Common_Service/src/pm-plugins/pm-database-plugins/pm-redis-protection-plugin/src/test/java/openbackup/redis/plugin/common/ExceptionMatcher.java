package openbackup.redis.plugin.common;

import openbackup.system.base.common.exception.LegoCheckedException;

import org.hamcrest.Description;
import org.junit.internal.matchers.TypeSafeMatcher;

/**
 * 自定义异常匹配器
 *
 * @author x30028756
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/06/16
 */
public class ExceptionMatcher extends TypeSafeMatcher<LegoCheckedException> {
    private long code;

    private String message;

    public ExceptionMatcher(long code, String message) {
        this.code = code;
        this.message = message;
    }

    @Override
    public boolean matchesSafely(LegoCheckedException item) {
        return item.getErrorCode() == code && item.getMessage().equals(message);
    }

    @Override
    public void describeTo(Description description) {
        description.appendText("expects code ")
            .appendValue(code)
            .appendText(",expects message ")
            .appendValue(message);
    }
}