package openbackup.system.base.common.enums;

/**
 * The AlarmLang
 *
 * @author g30003063
 * @since 2022-08-09
 */
public enum AlarmLang {
    EN(1, "en"),
    ZH(2, "zh");

    private final int value;
    private final String key;

    AlarmLang(final int value, final String key) {
        this.value = value;
        this.key = key;
    }

    /**
     * 获取语言value
     * 用于DM查询告警对象场景
     *
     * @return 语言信息
     */
    public int getValue() {
        return value;
    }

    /**
     * 获取语言Key
     * 用于DM查询告警场景
     *
     * @return 语言信息
     */
    public String getKey() {
        return key;
    }
}