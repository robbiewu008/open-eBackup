package openbackup.system.base.common.utils;

import openbackup.system.base.common.aspect.StringProperties;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.EmeiStorDefaultExceptionHandler;

import com.alibaba.fastjson.JSONObject;

import java.io.UnsupportedEncodingException;
import java.lang.reflect.Field;
import java.util.AbstractMap;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Optional;
import java.util.function.Function;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * String 工具类
 *
 * @author z90005513
 * @version V100R001C00
 * @since 2019-10-25
 */
public final class StringUtil {
    private static final String BLANK = "";

    private static final String DEFAULT_SPLIT = ":";

    private static final char[] HEX_CHARS = "0123456789abcdef".toCharArray();

    private static final int DEFAULT_TIMES = 3;

    private StringUtil() {}

    /**
     * 将数组按照默认的分隔符":"生成字符串
     *
     * @param array 数组
     * @return String
     */
    public static String arrayToString(Object[] array) {
        return arrayToStrWithSplit(array, DEFAULT_SPLIT);
    }

    /**
     * 将对象按照分隔符"，"生成字符串
     *
     * @param list 对象数组
     * @param func 对象操作函数，用于处理对象的特定函数，要求该函数返回类型为String
     * @param <T> 对象类型
     * @return String组合字符串
     */
    public static <T> String listToString(List<T> list, Function<T, String> func) {
        if (VerifyUtil.isEmpty(list)) {
            return "";
        }
        List<String> transferStrs = list.stream().map(func).collect(Collectors.toList());

        return String.join(",", transferStrs);
    }

    /**
     * 将数组元素用split分隔成字符串形式
     *
     * @param array 数值
     * @param split 分隔符
     * @return String [返回类型说明]
     */
    public static String arrayToStrWithSplit(Object[] array, String split) {
        if (array == null || array.length == 0) {
            return BLANK;
        }

        StringBuilder sb = new StringBuilder();

        for (Object object : array) {
            sb.append(object);
            sb.append(split);
        }

        return sb.substring(0, sb.length() - 1);
    }

    /**
     * 去除字符串左右空格
     *
     * @param str 字符串
     * @return String
     */
    public static String trim(String str) {
        if (str == null) {
            return str;
        }
        return str.trim();
    }

    /**
     * 按字节长度为len的字符串
     *
     * @param str 原始字符串
     * @param len 要截取的字节长度
     * @param encodedMode 编码方式
     * @return 截取后的字符串
     * @throws UnsupportedEncodingException 不支持的编码异常
     */
    public static String substr(String str, int len, String encodedMode) throws UnsupportedEncodingException {
        if (VerifyUtil.isEmpty(str)) {
            return null;
        }
        if (VerifyUtil.isEmpty(encodedMode)) {
            return str.substring(LegoNumberConstant.ZERO, len);
        }

        byte[] newStr = new byte[len];
        byte[] encodedBytes = str.getBytes(encodedMode);
        System.arraycopy(encodedBytes, LegoNumberConstant.ZERO, newStr, LegoNumberConstant.ZERO, len);
        String result = new String(newStr, encodedMode);
        int resLen = result.length();
        if (str.substring(LegoNumberConstant.ZERO, resLen).getBytes(encodedMode).length > len) {
            result = str.substring(LegoNumberConstant.ZERO, resLen - LegoNumberConstant.ONE);
        }
        return result;
    }

    /**
     * 字符串格式化。支持：a${0}b${1}c${2}
     *
     * @param template 模板
     * @param args 参数
     * @param <T> template type T
     * @return 格式化结果
     */
    public static <T> String format(String template, T[] args) {
        Map<String, Object> data = new HashMap<>();
        if (args != null) {
            for (int i = 0, n = args.length; i < n; i++) {
                data.put(String.valueOf(i), args.length);
            }
        }
        return format(template, data);
    }

    /**
     * 字符串格式化。支持格式:a{var1}b{var2}c{var3}
     *
     * @param template 模板
     * @param data 数据
     * @return 格式化结果
     */
    public static String format(String template, Map<String, Object> data) {
        return format(template, data, "\\{([^{}]+)\\}");
    }

    /**
     * 支持使用指定的占位符对字符串进行格式化
     *
     * @param template 模板
     * @param data 数据
     * @param placeHolder 占位符
     * @return 格式化结果
     */
    public static String format(String template, Map<String, Object> data, String placeHolder) {
        Pattern pattern = Pattern.compile(placeHolder);
        Matcher matcher = pattern.matcher(template);
        StringBuffer buffer = new StringBuffer();
        while (matcher.find()) {
            if (matcher.groupCount() < 1) {
                throw new EmeiStorDefaultExceptionHandler("placeHolder is incorrect.");
            }
            matcher.appendReplacement(buffer, "");
            String expr = matcher.group(1);
            Object value;
            if (expr != null) {
                value = ExprUtil.eval(data, expr);
            } else {
                value = null;
            }
            buffer.append(value != null ? value : "");
        }
        matcher.appendTail(buffer);
        return buffer.toString();
    }

    /**
     * split and trim
     *
     * @param string string
     * @param regex regex
     * @return result
     */
    public static List<String> splitrim(String string, String regex) {
        return Arrays.stream(string.split(regex))
                .map(item -> item != null ? item.trim() : null)
                .collect(Collectors.toList());
    }

    /**
     * startsWith
     *
     * @param string string
     * @param prefix prefix
     * @param tokens tokens
     * @return result
     */
    public static boolean startsWith(String string, String prefix, char... tokens) {
        if (string.length() < prefix.length()) {
            return false;
        }
        if (!string.startsWith(prefix)) {
            return false;
        }
        if (string.length() == prefix.length()) {
            return true;
        }
        char code = string.charAt(prefix.length());
        for (char token : tokens) {
            if (code == token) {
                return true;
            }
        }
        return false;
    }

    /**
     * asHex
     *
     * @param buf buf
     * @return String
     */
    public static String asHex(byte[] buf) {
        char[] chars = new char[2 * buf.length];
        for (int i = 0; i < buf.length; ++i) {
            chars[2 * i] = HEX_CHARS[(buf[i] & 0xF0) >>> 4];
            chars[2 * i + 1] = HEX_CHARS[buf[i] & 0x0F];
        }
        return new String(chars);
    }

    /**
     * asciiToHEX
     *
     * @param ascii ascii
     * @return String
     */
    public static String asciiToHEX(String ascii) {
        // Initialize final String
        String hex = "";

        // Make a loop to iterate through
        // every character of ascii string
        for (int i = 0; i < ascii.length(); i++) {
            // take a char from
            // position i of string
            char ch = ascii.charAt(i);

            // cast char to integer and
            // find its ascii value
            int in = ch;

            // change this ascii value
            // integer to hexadecimal value
            String part = Integer.toHexString(in);

            // add this hexadecimal value
            // to final string.
            hex += part;
        }
        // return the final string hex
        return hex;
    }

    /**
     * stringify
     *
     * @param object object
     * @return string
     */
    public static String stringify(Object object) {
        return object != null
                ? (isStringProperties(object) ? object.toString() : JSONObject.toJSONString(object))
                : null;
    }

    private static boolean isStringProperties(Object object) {
        if (object == null) {
            return false;
        }
        if (object.getClass().isPrimitive()) {
            return true;
        }
        if (object instanceof CharSequence) {
            return true;
        }
        return object instanceof StringProperties;
    }

    /**
     * split string with sp
     *
     * @param content content
     * @param sp sp
     * @return result
     */
    public static Map.Entry<String, String> split(String content, String sp) {
        String prefix;
        String suffix;
        int index = content.indexOf(sp);
        if (index == -1) {
            prefix = content;
            suffix = null;
        } else {
            prefix = content.substring(0, index);
            suffix = content.substring(index + 1);
        }
        return new AbstractMap.SimpleEntry<>(prefix, suffix);
    }

    /**
     * 清理字符串
     *
     * @param string 字符串
     */
    public static final void clean(String string) {
        if (string != null) {
            try {
                // 每次拿效率不高，考虑到用的不多，暂时这样
                Field valueField = String.class.getDeclaredField("value");
                valueField.setAccessible(true);

                // 获取字符串中的值
                Object valueObject = valueField.get(string);

                // valueObject为null，无法通过下面的判断
                if (valueObject instanceof char[]) {
                    char[] valueChars = (char[]) valueObject;
                    clean(valueChars);
                }
            } catch (NoSuchFieldException | IllegalAccessException exception) {
                return;
            }
        }
    }

    /**
     * 清理字符数组
     *
     * @param chars 字符数组
     */
    public static void clean(char[] chars) {
        // 字符数组不为空，且字符数组数量大于0
        if (chars != null && chars.length > 0) {
            for (int times = 0; times < DEFAULT_TIMES; times++) {
                for (int index = 0; index < chars.length; index++) {
                    chars[index] = 0;
                }
            }
        }
    }

    /**
     * map camel case to underscore
     *
     * @param text text
     * @return result
     */
    public static String mapCamelCaseToUnderscore(String text) {
        if (text == null) {
            return null;
        }
        return text.replaceAll("([^_A-Z])([A-Z])", "$1_$2").toLowerCase(Locale.ENGLISH);
    }

    /**
     * cast as snake string
     *
     * @param text text
     * @return result
     */
    public static String snake(String text) {
        return mapCamelCaseToUnderscore(replace(text, "_", new String[] {".", " ", "-"}));
    }

    /**
     * replace method
     *
     * @param text text
     * @param replacement replacement
     * @param candidates candidates
     * @return result
     */
    public static String replace(String text, String replacement, String[] candidates) {
        if (text == null) {
            return null;
        }
        String result = text;
        for (String candidate : candidates) {
            while (result.contains(candidate)) {
                result = result.replace(candidate, replacement);
            }
        }
        return result;
    }

    /**
     * cast text to lower case
     *
     * @param text text
     * @return result
     */
    public static String lower(String text) {
        return Optional.ofNullable(text).map(str -> str.toLowerCase(Locale.ENGLISH)).orElse(null);
    }

    /**
     * to char
     *
     * @param code code
     * @return char
     */
    public static char toChar(int code) {
        return (char) code;
    }
}
