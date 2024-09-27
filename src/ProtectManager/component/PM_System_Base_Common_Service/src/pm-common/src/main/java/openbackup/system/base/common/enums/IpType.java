package openbackup.system.base.common.enums;

/**
 * IpType
 *
 * @author wwx1013713
 * @since 2021-06-29
 */
public enum IpType {
    IPV4("IPV4"),
    IPV6("IPV6"),
    IPV4_SEGMENT("IPV4_SEGMENT"),
    IPV6_SEGMENT("IPV6_SEGMENT");

    private final String value;

    IpType(String value) {
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
