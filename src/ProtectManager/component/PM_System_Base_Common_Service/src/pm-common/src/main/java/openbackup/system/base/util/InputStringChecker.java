package openbackup.system.base.util;

import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.Data;

import org.apache.commons.lang3.StringUtils;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.validation.ConstraintValidator;
import javax.validation.ConstraintValidatorContext;

/**
 * 输入参数校验实现类
 *
 * @author w00426202
 * @since 2023-05-11
 */
@Data
public class InputStringChecker implements ConstraintValidator<CheckInputString, String> {
    /**
     * 字符通用校验类型
     */
    private static final String[] CHARACTERS_TYPE_COMMON = new String[] {
        "&", "<", ">", "\"", "'", ":", "[", "]", "$", "(", ")", "%", "+", "/", "\\", "#", "`", "*", ",", "-", ";", "=",
        "^", "|"
    };

    /**
     * URL校验类型
     */
    private static final String[] CHARACTERS_TYPE_URL = new String[] {
        "<", ">", "\"", "'", "[", "]", "$", "(", ")", "+", "\\", "#", "`", "*", ",", ";", "^", "|"
    };

    /**
     * name校验类型
     */
    private static final String[] CHARACTERS_TYPE_NAME = new String[] {
        "&", "<", ">", "\"", "'", ":", "$", "%", "\\", "#", "`", "*", ",", ";", "=", "^", "|"
    };

    /**
     * 评论校验类型
     */
    private static final String[] CHARACTERS_TYPE_REMARK = new String[] {
        "&", "<", ">", "\"", "'", "[", "]", "$", "%", "+", "/", "\\", "#", "`", "*", ":", "=", "^", "|"
    };

    /**
     * 通用特殊字符校验类型
     */
    private static final String[] CHARACTERS_TYPE_SPECIAL_COMMON = new String[] {"<", ">", "$", "#", "`", "*", "^"};

    /**
     * 字符类型
     */
    private int type;

    /**
     * 是否可以为blank
     */
    private boolean canBlank;

    /**
     * 是否为null
     */
    private boolean canNull;

    /**
     * 最小长度
     */
    private int minLength;

    /**
     * 最大长度
     */
    private int maxLength;

    /**
     * 通过正则校验字符串，并且通过maxLength比对中英文字符串最大值，中文作为两个英文字符
     */
    private String regexp;

    private Pattern patternLen = Pattern.compile(RegexpConstants.CHINESE_CHARACTER_LEN_VALID);

    @Override
    public void initialize(CheckInputString arg0) {
        type = arg0.type();
        canNull = arg0.canNull();
        canBlank = arg0.canBlank();
        minLength = arg0.minLen();
        maxLength = arg0.maxLen();
        regexp = arg0.regexp();
    }

    @Override
    public boolean isValid(String strValue, ConstraintValidatorContext context) {
        // 如果为NULL，就校验是否能为NULL
        // 如果canBlank 为 false , 字符串不能为null
        if (strValue == null) {
            return canNull && canBlank;
        }

        // 如果字符串是空串或者全是空格
        if (StringUtils.isBlank(strValue)) {
            // 先校验长度要求，再校验是否能blank
            return canBlank && strValue.length() <= maxLength;
        }

        // 中文替换成两个英文字符校验
        // 中文标点也替换成两个英文字符串"aa"，然后在计算长度
        // ？！“”￥‘’（），—、：；。《》【】…
        String newText = patternLen.matcher(strValue).replaceAll("aa");
        if (newText.length() > maxLength || newText.length() < minLength) {
            return false;
        }

        if (type > 0) {
            return checkBlackList(strValue);
        }

        return regexMatch(strValue, regexp, minLength, maxLength);
    }

    /**
     * 黑名单校验
     *
     * @param strValue strValue
     * @return 是否通过黑名单校验
     */
    private boolean checkBlackList(String strValue) {
        for (String specialCharacter : getSpecialCharacters(type)) {
            if (strValue.contains(specialCharacter)) {
                return false;
            }
        }
        return true;
    }

    /**
     * 获取特殊字符数组
     *
     * @param strType 特殊字符校验类型
     * @return 特殊字符数组
     */
    private String[] getSpecialCharacters(int strType) {
        switch (type) {
            case TypeMode.COMMON:
                return CHARACTERS_TYPE_COMMON;
            case TypeMode.URL:
                return CHARACTERS_TYPE_URL;
            case TypeMode.NAME:
                return CHARACTERS_TYPE_NAME;
            case TypeMode.COMMON_LOCAL_REMARK:
                return CHARACTERS_TYPE_REMARK;
            case TypeMode.SPECIAL_CHARACTERS_COMMON:
                return CHARACTERS_TYPE_SPECIAL_COMMON;
            default:
                return CHARACTERS_TYPE_COMMON;
        }
    }

    /**
     * 正则表达式匹配（只适合里面包含字母的校验） 长度以一个中文字符算两个
     *
     * @param text 需要匹配的字符串
     * @param inputRegexp 正则字符串
     * @param inputMinLength 最小长度
     * @param inputMaxLength 最大长度
     * @return 是否匹配
     */
    private boolean regexMatch(String text, String inputRegexp, int inputMinLength, int inputMaxLength) {
        // 如果正则要求是空字符串，就不做校验，返回true
        if (inputRegexp.length() == 0) {
            return true;
        }

        // 正则表达式判断
        Pattern pattern = Pattern.compile(inputRegexp);
        Matcher matcher = pattern.matcher(text);

        if (!matcher.matches()) {
            return false;
        }

        return true;
    }
}

