package openbackup.system.base.common.validator;

import openbackup.system.base.common.errors.ErrorConstant;
import openbackup.system.base.common.exception.EmeiStorBadRequestException;
import openbackup.system.base.common.utils.HtmlStringConverter;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import java.text.Normalizer;
import java.text.Normalizer.Form;
import java.util.regex.Pattern;

/**
 * 后台验证工具类，和前台界面的验证规则保持一致
 * 名称类字段：名称只能以字母、中文或_开头，由数字、字母、中文、_和-组成  。长度32字符
 * 描述类字段：长度256个字符
 * 其他文本框长度都是32位，但是有特殊要求的要加特殊验证。
 * 邮箱地址，url地址，端口，电话，日期， email的格式， 可以参考UnifyValid.js 里面的正则
 * 拉框里面的字段另外单独给出，按照具体的使用场景。
 *
 * @author w00448845
 * @version [CDM Integrated machine]
 * @since 2019-11-05
 */
public final class Validator {
    private static final int MAX_PORT = 65536;

    private Validator() {}

    /**
     * 判断字段是否为空 符合返回true
     *
     * @param str str
     * @return boolean
     */
    public static synchronized boolean stringIsNull(String str) {
        return str == null || str.trim().isEmpty();
    }

    /**
     * 匹配是否符合正则表达式pattern 匹配返回true
     *
     * @param str     匹配的字符串
     * @param pattern 匹配模式
     * @return boolean
     */
    public static boolean regExpCheck(String str, String pattern) {
        if (str == null || str.trim().isEmpty()) {
            throw new EmeiStorBadRequestException(ErrorConstant.ErrorCode.INVALID_PARAMS);
        }

        if (!Pattern.compile(pattern).matcher(Normalizer.normalize(str, Form.NFKC)).matches()) {
            throw new EmeiStorBadRequestException(ErrorConstant.ErrorCode.INVALID_PARAMS);
        }
        return true;
    }

    /**
     * 验证字符串的长度
     *
     * @param str    str
     * @param length 长度
     * @return boolean
     */
    public static boolean checkStringLength(String str, int length) {
        if (str == null) {
            return true;
        }
        String convertStr = HtmlStringConverter.decodeByLength(str);
        return convertStr.length() <= length;
    }

    /**
     * 匹配数字 0-9的数字
     *
     * @param str str
     * @return [参数说明]
     */
    public static boolean isNum(String str) {
        return regExpCheck(str, RegexpConstants.INTEGER_NEGATIVE);
    }

    /**
     * 判断是否是合法的排序字段
     *
     * @param str str
     * @return [参数说明]
     */
    public static boolean canOrderBy(String str) {
        if (str == null || str.trim().isEmpty()) {
            return true;
        }
        String normalStr = Normalizer.normalize(str, Form.NFKC);
        boolean isLegal =
                Pattern.compile(RegexpConstants.STR_ENG_NUM_DOWN).matcher(normalStr).matches()
                        && !Pattern.compile(RegexpConstants.INTEGER_NEGATIVE).matcher(normalStr).matches();
        if (!isLegal) {
            throw new EmeiStorBadRequestException(ErrorConstant.ErrorCode.INVALID_PARAMS);
        }
        return true;
    }

    /**
     * 匹配时间
     *
     * @param str str
     * @return [参数说明]
     */
    public static boolean validateTime(String str) {
        return regExpCheck(str, RegexpConstants.TIME_FORMAT);
    }

    /**
     * 邮件格式 a@b
     *
     * @param str str
     * @return [参数说明]
     */
    public static boolean isEmail(String str) {
        return regExpCheck(str, RegexpConstants.EMAIL);
    }

    /**
     * 是否是端口
     *
     * @param port port
     * @return boolean
     */
    public static boolean isPort(int port) {
        return port > 0 && port < MAX_PORT;
    }

    /**
     * isDefaultName
     *
     * @param defaultName defaultName
     * @return boolean
     */
    public static boolean isDefaultName(String defaultName) {
        if (VerifyUtil.isEmpty(defaultName)) {
            return false;
        }
        String convertStr = HtmlStringConverter.decodeByLength(defaultName);

        if (convertStr.contains("'") || convertStr.indexOf('"') != -1 || convertStr.contains("<")) {
            return false;
        }
        return !convertStr.contains(">") && !convertStr.contains("#") && !convertStr.contains("&");
    }

    /**
     * 检查时间是否合法
     *
     * @param createTimeStart void
     */
    public static void checkLongTime(Long createTimeStart) {
        if (VerifyUtil.isEmpty(createTimeStart) || createTimeStart < 0) {
            throw new EmeiStorBadRequestException(ErrorConstant.ErrorCode.INVALID_PARAMS);
        }
    }
}
