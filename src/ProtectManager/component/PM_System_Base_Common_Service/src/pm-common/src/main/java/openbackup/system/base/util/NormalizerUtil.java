package openbackup.system.base.util;

import org.springframework.util.StringUtils;

import java.text.Normalizer;

/**
 * 处理服务器注入风险Server-Side Request Forgery工具类
 *
 * @author l00422407
 * @since 2021-01-23
 */
public class NormalizerUtil {
    /**
     * 过滤不安全的特殊字符
     *
     * @param item item
     * @return String
     */
    public static String normalizeForString(String item) {
        if (StringUtils.isEmpty(item)) {
            return "";
        }
        return Normalizer.normalize(item, Normalizer.Form.NFKC);
    }
}

