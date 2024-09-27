package openbackup.system.base.common.utils;

import openbackup.system.base.common.constants.IsmConstant;
import openbackup.system.base.common.exception.EmeiStorDefaultExceptionHandler;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.extern.slf4j.Slf4j;

import java.net.URI;
import java.net.URISyntaxException;
import java.text.Normalizer;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * 参数校验工具类
 * 约定：validate开头的方法,在参数不满足要求时,会抛出LegoCheckedException异常,其余方法,返回boolean结果
 *
 * @author l90006153
 * @version [OceanStor ReplicationDirector V100R003C10, 2015-1-9]
 * @since 2019-10-25
 */
@Slf4j
public class ValidateUtil {
    private ValidateUtil() {}

    /**
     * 指定正则表达式，验证指定字符串是否匹配
     * 如果不匹配，会抛出异常
     *
     * @param regex 正则表达式，不允许为null，否则会抛异常
     * @param str   字符串，不允许为null，否则会抛异常
     */
    public static void validate(String regex, String str) {
        if (str == null || regex == null) {
            throw new EmeiStorDefaultExceptionHandler(null);
        }

        if (!match(regex, str)) {
            throw new EmeiStorDefaultExceptionHandler(null);
        }
    }

    /**
     * 指定正则表达式，验证指定字符串是否匹配
     *
     * @param regex 正则表达式，不允许为null，否则会抛异常
     * @param str   字符串，不允许为null，否则会抛异常
     * @return boolean 匹配结果，true/false
     */
    public static boolean match(String regex, String str) {
        if (regex == null) {
            throw new IllegalArgumentException("Regular expression can not be null.");
        }

        if (str == null) {
            throw new IllegalArgumentException("The input string can not be null.");
        }

        Pattern pat = Pattern.compile(regex);
        // 红线要求：进行正则表达式的match之前，都必须经过归一化处理
        String normalize = Normalizer.normalize(str, Normalizer.Form.NFKC);
        Matcher mat = pat.matcher(normalize);
        return mat.matches();
    }

    /**
     * 指定正则表达式，验证指定字符串匹配的字符数量
     *
     * @param regex 正则表达式，不允许为null，否则会抛异常
     * @param str   字符串，不允许为null，否则会抛异常
     * @return boolean 匹配结果，true/false
     */
    public static int matchSize(String regex, String str) {
        if (regex == null) {
            throw new IllegalArgumentException("Regular expression can not be null.");
        }

        if (str == null) {
            throw new IllegalArgumentException("The input string can not be null.");
        }

        Pattern pat = Pattern.compile(regex);
        // 红线要求：进行正则表达式的match之前，都必须经过归一化处理
        String normalize = Normalizer.normalize(str, Normalizer.Form.NFKC);
        Matcher mat = pat.matcher(normalize);
        int retMatch = 0;
        while (mat.find()) {
            retMatch++;
        }
        return retMatch;
    }

    /**
     * 指定正则表达式，验证指定字符串匹配的字符数量
     *
     * @param str   字符串，不允许为null，否则会抛异常
     * @return boolean 匹配结果，true/false
     */
    public static int matchSize(String str) {
        List<String> matchStrings = new ArrayList<>();
        matchStrings.add(RegexpConstants.LOWERCASE_LETTERS);
        matchStrings.add(RegexpConstants.UPPERCASE_LETTERS);
        matchStrings.add(RegexpConstants.NUM);
        matchStrings.add(RegexpConstants.SPECIAL_CHARACTERS);
        matchStrings.add(RegexpConstants.WHITESPACE);
        int matchSize = 0;
        int matchCount = 0;
        for (String re : matchStrings) {
            int size = matchSize(re, str);
            if (size > 0) {
                matchCount++;
            }
            matchSize += size;
        }
        if (matchCount < RegexpConstants.MATCH_TIMES) {
            log.error("The input pwd should match 4 type regePattens, but now is [{}] type.", matchCount);
            throw new IllegalArgumentException("The input pwd should match 4 type regePattens.");
        }
        return matchSize;
    }

    /**
     * 检查字符串长度
     *
     * @param str 字符串，不允许为null，否则会抛异常
     * @param min 最小长度
     * @param max 最大长度，必须大于或等于min
     * @return boolean 检查结果，true/false
     */
    public static boolean checkLength(String str, int min, int max) {
        if (str == null) {
            throw new IllegalArgumentException("The input string can not be null.");
        }
        if (min < 0 || min > max) {
            throw new IllegalArgumentException("The range is error.");
        }

        int length = str.length();
        return length >= min && length <= max;
    }

    /**
     * 校验url是否合法
     *
     * @param urlString urlString
     * @return 是否合法
     */
    public static boolean isValidUrl(String urlString) {
        URI uri = null;
        try {
            uri = new URI(urlString);
        } catch (URISyntaxException e) {
            return false;
        }
        if (uri.getHost() == null) {
            return false;
        }
        return uri.getScheme().equalsIgnoreCase("http") || uri.getScheme().equalsIgnoreCase("https");
    }

    /**
     * 检查端口范围
     *
     * @param port 端口值
     * @return boolean 检查结果，true/false
     */
    public static boolean checkPort(long port) {
        return port >= IsmConstant.PORT_MIN && port <= IsmConstant.PORT_MAX;
    }
}
