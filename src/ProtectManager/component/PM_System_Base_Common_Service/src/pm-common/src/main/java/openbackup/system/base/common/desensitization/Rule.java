package openbackup.system.base.common.desensitization;

/**
 * 敏感词过滤规则
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/3/22
 **/
public class Rule {
    /**
     * 过滤规则
     */
    private String rule;

    /**
     * 过滤规则对应的敏感词配置
     */
    private String keys;

    public String getRule() {
        return rule;
    }

    public void setRule(String rule) {
        this.rule = rule;
    }

    public String getKeys() {
        return keys;
    }

    public void setKeys(String keys) {
        this.keys = keys;
    }
}
