/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.system.base.util;

import lombok.Data;

import org.apache.commons.lang3.StringUtils;

import java.util.Arrays;
import java.util.List;
import java.util.Objects;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.validation.ConstraintValidator;
import javax.validation.ConstraintValidatorContext;

/**
 * List输入参数校验实现类
 *
 * @author w00426202
 * @since 2023-05-11
 */
@Data
public class ListStringChecker implements ConstraintValidator<CheckInputList, List<String>> {
    private static final Pattern PATTERN_CHINESE = Pattern.compile("[\\u4e00-\\u9fa5]");

    /**
     * & < > " ' : [ ] $ ( ) % + \ / # ` * , - ; = ^ |
     */
    private static final String[] CHARACTERS_TYPE_COMMON = new String[] {
        "&", "<", ">", "\"", "'", ":", "[", "]", "$", "(", ")", "%", "+", "/", "\\", "#", "`", "*", ",", "-", ";", "=",
        "^", "|"
    };

    private static final String[] CHARACTERS_TYPE_SPECIAL_COMMON = new String[] {"<", ">", "$", "#", "`", "*", "^"};

    private int maxSize;

    private int minSize;

    private String strRegexp;

    private boolean shouldSpecialCharCheck;

    private int specialKey;

    private String specialChars;

    private int strMaxLength;

    private int strMinLength;

    private int totalLength;

    @Override
    public void initialize(CheckInputList arg0) {
        maxSize = arg0.maxSize();
        minSize = arg0.minSize();
        strRegexp = arg0.strRegexp();
        shouldSpecialCharCheck = arg0.specialCharCheck();
        specialKey = arg0.specialKey();
        specialChars = arg0.specialChars();
        strMaxLength = arg0.strMaxLength();
        strMinLength = arg0.strMinLength();
        totalLength = arg0.totalLength();
    }

    @Override
    public boolean isValid(List<String> list, ConstraintValidatorContext context) {
        return validateCommon(list);
    }

    /**
     * 校验字符串集合
     *
     * @param list list
     * @return 检查结果
     */
    private boolean validateCommon(List<String> list) {
        // 校验集合长度
        if (!validateListSize(list)) {
            return false;
        }

        // 校验集合中的每个字符串
        if (!validateListStr(list)) {
            return false;
        }

        // 校验集合中字符总长度
        return validateTotalLength(list);
    }

    /**
     * 校验集合中的每个字符串
     *
     * @param list list
     * @return 校验结果
     */
    private boolean validateListStr(List<String> list) {
        if (Objects.isNull(list)) {
            return true;
        }

        boolean isValid = true;
        for (String str : list) {
            isValid = validateStr(str);
            if (!isValid) {
                break;
            }
        }
        return isValid;
    }

    /**
     * 校验集合长度.
     *
     * @param list list
     * @return 校验结果
     */
    private boolean validateListSize(List<String> list) {
        if (Objects.isNull(list)) {
            return minSize < 0;
        }
        int size = list.size();

        return size >= minSize && size <= maxSize;
    }

    /**
     * 校验总长度.
     *
     * @param list list
     * @return 校验结果
     */
    private boolean validateTotalLength(List<String> list) {
        if (Objects.isNull(list)) {
            return true;
        }
        int count = list.stream().filter(str -> !StringUtils.isEmpty(str)).mapToInt(str -> str.length()).sum();
        return count <= totalLength;
    }

    /**
     * 校验字符串.
     *
     * @param str str
     * @return 校验结果
     */
    private boolean validateStr(String str) {
        // 长度校验
        if (!validateLength(str)) {
            return false;
        }

        // 特殊字符校验
        if (!validateSpecialCharCheck(str)) {
            return false;
        }

        // 正则表达式校验
        return validateRegex(str);
    }

    /**
     * 正则表达式校验.
     *
     * @param str str
     * @return 校验结果
     */
    private boolean validateRegex(String str) {
        // 正则表达式为空，返回true
        if (StringUtils.isEmpty(strRegexp)) {
            return true;
        }

        String tempStr = str;
        if (Objects.isNull(str)) {
            tempStr = "";
        }

        if (StringUtils.isEmpty(tempStr) && strMinLength == 0) {
            return true;
        }

        return regexMatch(tempStr, strRegexp);
    }

    /**
     * 特殊字符校验
     *
     * @param str str
     * @return 校验结果
     */
    private boolean validateSpecialCharCheck(String str) {
        if (StringUtils.isEmpty(str)) {
            return true;
        }

        // 不需要特殊字符校验
        if (!shouldSpecialCharCheck) {
            return true;
        }
        String[] specialCharsTmp = null;
        if (StringUtils.isNotBlank(specialChars)) {
            specialCharsTmp = specialChars.split(",");
        } else {
            specialCharsTmp = getSpecialCharacters(specialKey);
        }
        boolean isContainsSpecial = Arrays.asList(specialCharsTmp).stream().anyMatch(c -> str.contains(c));

        // 包含特殊字符校验不通过.
        return !isContainsSpecial;
    }

    /**
     * 验证字符串长度.
     *
     * @param str str
     * @return 校验结果
     */
    private boolean validateLength(String str) {
        if (StringUtils.isEmpty(str)) {
            return strMinLength >= 0;
        }

        // 将一个中文字符替换成两个英文字符计算长度
        String newText = PATTERN_CHINESE.matcher(str).replaceAll("aa");
        int len = newText.length();
        if (strMaxLength != 0 && len > strMaxLength) {
            return false;
        }
        return len >= strMinLength;
    }

    /**
     * 获取特殊字符数组
     *
     * @param key key
     * @return 特殊字符数组
     */
    private String[] getSpecialCharacters(int key) {
        switch (key) {
            case TypeMode.COMMON:
                return CHARACTERS_TYPE_COMMON;
            case TypeMode.SPECIAL_CHARACTERS_COMMON:
                return CHARACTERS_TYPE_SPECIAL_COMMON;
            default:
                return CHARACTERS_TYPE_COMMON;
        }
    }

    /**
     * 正则表达式匹配
     *
     * @param text 需要匹配的字符串
     * @param regexString 正则字符串
     * @return 是否匹配
     */
    private boolean regexMatch(String text, String regexString) {
        // 正则表达式判断
        Pattern pattern = Pattern.compile(regexString);
        Matcher matcher = pattern.matcher(text);
        return matcher.matches();
    }
}
