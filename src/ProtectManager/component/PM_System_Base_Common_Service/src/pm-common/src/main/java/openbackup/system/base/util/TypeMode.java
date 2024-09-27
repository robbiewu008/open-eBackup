package openbackup.system.base.util;

/**
 * 输入参数类型
 *
 * @author w00426202
 * @since 2023-05-11
 */
public interface TypeMode {
    /**
     * 通用校验
     */
    int COMMON = 1;

    /**
     * url格式校验
     */
    int URL = 2;

    /**
     * 输入姓名校验
     */
    int NAME = 3;

    /**
     * 原有黑名单中去除部分字符 去掉逗号，分号，中横线，圆括号
     */
    int COMMON_LOCAL_REMARK = 4;

    /**
     * 通用特殊字符校验
     */
    int SPECIAL_CHARACTERS_COMMON = 5;
}
