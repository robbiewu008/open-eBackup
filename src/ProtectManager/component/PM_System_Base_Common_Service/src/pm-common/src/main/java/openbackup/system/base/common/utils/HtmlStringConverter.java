/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 */

package openbackup.system.base.common.utils;

/**
 * 替换特殊字符
 * 把特殊字符的有实体替换成原状
 *
 * @author w00448845
 * @version [CDM Integrated machine]
 * @since 2019-11-05
 */
public final class HtmlStringConverter {
    private HtmlStringConverter() {
    }

    /**
     * 解码
     *
     * @param encodedChar 解码串
     * @return 解码后的值
     */
    public static String decode(String encodedChar) {
        if (encodedChar == null) {
            return "";
        }
        return encodedChar.replaceAll("&quot;", "\"")
            .replaceAll("&#39;", "'")
            .replaceAll("&#96;", "`")
            .replaceAll("&#37;", "%")
            .replaceAll("&lt;", "<")
            .replaceAll("&gt;", ">")
            .replaceAll("&#40;", "\\\\(")
            .replaceAll("&#41;", "\\\\)")
            .replaceAll("&#58;", ":")
            .replaceAll("&#46;", ".");
    }

    /**
     * 编码
     *
     * @param encodedChar 编码串
     * @return 编码后的串
     */
    public static String decodeByLength(String encodedChar) {
        if (encodedChar == null) {
            return "";
        }
        return encodedChar.replaceAll("&quot;", "\"")
            .replaceAll("&#39;", "'")
            .replaceAll("&#96;", "`")
            .replaceAll("&#37;", "%")
            .replaceAll("&lt;", "<")
            .replaceAll("&gt;", ">")
            .replaceAll("&#40;", "(")
            .replaceAll("&#41;", ")")
            .replaceAll("&nbsp;", " ")
            .replaceAll("&#58;", ":")
            .replaceAll("&#46;", ".")
            .replaceAll("&#58;", ":")
            .replaceAll("&#46;", ".");
    }
}