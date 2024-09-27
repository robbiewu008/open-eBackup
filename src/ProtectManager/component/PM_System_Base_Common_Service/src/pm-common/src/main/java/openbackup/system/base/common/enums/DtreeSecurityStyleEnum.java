package openbackup.system.base.common.enums;

/**
 * Dtree安全模式
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-22
 */
public enum DtreeSecurityStyleEnum {
    NATIVE("1"),

    NTFS("2"),

    UNIX("3"),

    MIXED("4");

    private final String style;

    DtreeSecurityStyleEnum(String style) {
        this.style = style;
    }

    public String getStyle() {
        return style;
    }
}
