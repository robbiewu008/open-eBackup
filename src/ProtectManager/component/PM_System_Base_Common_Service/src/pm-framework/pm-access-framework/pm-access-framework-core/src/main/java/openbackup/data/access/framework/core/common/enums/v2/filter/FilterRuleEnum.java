package openbackup.data.access.framework.core.common.enums.v2.filter;

/**
 * FilterRuleEnum
 *
 * @description: 资源过滤规则枚举类，表示资源按照什么规则进行匹配
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/11/30
 **/
public enum FilterRuleEnum {
    /**
     * 模糊匹配，目标值任意位置包含给定值，都匹配成功
     */
    FUZZY("Fuzzy"),
    /**
     * 精确匹配：目标值与给定值完全相等，才匹配成功
     */
    EXACT("Exact"),
    /**
     * 正则表达式匹配：目标值满足正则表达式，才匹配成功
     */
    REGEX("Regex");

    private String rule;

    FilterRuleEnum(String rule) {
        this.rule = rule;
    }

    public String getRule() {
        return rule;
    }
}
