package openbackup.system.base.util;

import openbackup.system.base.common.validator.constants.RegexpConstants;

import java.util.UUID;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * ID 生成器工具类
 *
 * @author w00607005
 * @since 2023-07-06
 */
public class IdUtil {
    /**
     * 获取随机UUID
     *
     * @return 随机UUID
     */
    public static String randomUUID() {
        return UUID.randomUUID().toString();
    }

    /**
     * 简化后的UUID，去掉横线
     *
     * @return 去掉横线的UUID
     */
    public static String simpleUUID() {
        return UUID.randomUUID().toString().replaceAll("\\-", "");
    }

    /**
     * 校验传参是否uuid格式
     *
     * @param uuid uuid
     * @return 是否uuid
     */
    public static Boolean isUUID(String uuid) {
        Matcher noSeparatorUuidMatcher = Pattern.compile(RegexpConstants.UUID_N0_SEPARATOR).matcher(uuid);
        Matcher uuidMatcher = Pattern.compile(RegexpConstants.UUID).matcher(uuid);
        if (noSeparatorUuidMatcher.find() || uuidMatcher.find()) {
            return true;
        }
        return false;
    }
}
