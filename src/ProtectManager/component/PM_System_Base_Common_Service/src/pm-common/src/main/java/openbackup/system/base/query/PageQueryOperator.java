package openbackup.system.base.query;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

/**
 * page query condition enum
 *
 * @author h30003246
 * @since 2021-06-07
 */
public enum PageQueryOperator {
    /**
     * 等于
     */
    EQ("=="),

    /**
     * 不等于
     */
    NE("!="),

    /**
     * 大于
     */
    GT(">"),

    /**
     * 大于等于
     */
    GE(">="),

    /**
     * 小于
     */
    LT("<"),

    /**
     * 小于等于
     */
    LE("<="),

    /**
     * 匹配 like "%值“
     */
    LIKE_LEFT("~="),

    /**
     * 匹配 like "值%
     */
    LIKE_RIGHT("=~"),

    /**
     * like
     */
    LIKE("~~"),

    /**
     * in
     */
    IN("in"),

    NOT_IN("not in");

    private final String value;

    PageQueryOperator(String value) {
        this.value = value;
    }

    @JsonValue
    public String getValue() {
        return value;
    }

    /**
     * get page query operation type enum
     *
     * @param str str
     * @return page query operation enum
     */
    @JsonCreator
    public static PageQueryOperator get(String str) {
        return EnumUtil.get(PageQueryOperator.class, PageQueryOperator::getValue, str);
    }
}
