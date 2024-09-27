package openbackup.system.base.util;

import openbackup.system.base.common.exception.LegoCheckedException;

import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * 编号获取工具类
 *
 * @author w00607005
 * @since 2023-07-07
 */
public class OrderNoUtil {
    /**
     * 任务流子任务
     */
    private static final String TASK = "TK";

    /**
     * 时间格式化
     */
    private static final String DATE_TIME_PATTERN_SIMPLE = "yyMMddHHmmssSS";

    private static final AtomicInteger NEXT_COUNTER;

    static {
        SecureRandom secureRandom;
        try {
            secureRandom = SecureRandom.getInstanceStrong();
            NEXT_COUNTER = new AtomicInteger(secureRandom.nextInt(100));
        } catch (NoSuchAlgorithmException e) {
            throw LegoCheckedException.cast(e);
        }
    }

    /**
     * 获取任务序列
     *
     * @return taskNo
     */
    public static String getTaskNo() {
        return generateNo(TASK);
    }

    private static String generateNo(String type) {
        String dateTimeStr = LocalDateTime.now().format(DateTimeFormatter.ofPattern(DATE_TIME_PATTERN_SIMPLE));
        return type + dateTimeStr + NEXT_COUNTER.getAndIncrement();
    }
}
