package openbackup.system.base.common.enums;

/**
 * hcs复制类型枚举
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/7/28
 */
public enum HcsCopyTypeEnum {
    REGION_COPY("0"),
    HCS_COPY("1");

    private final String value;

    HcsCopyTypeEnum(String value) {
        this.value = value;
    }

    /**
     * 获取类型字符串
     *
     * @return 类型字符串
     */
    public String getValue() {
        return value;
    }
}
