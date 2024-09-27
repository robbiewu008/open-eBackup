package openbackup.data.protection.access.provider.sdk.enums;

/**
 * 副本类型枚举类
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-11
 */
public enum CopyFormatEnum {
    /**
     * 快照格式:原生格式
     */
    INNER_SNAPSHOT(0),

    /**
     * 目录格式:非原生格式
     */
    INNER_DIRECTORY(1),

    /**
     * 外部格式
     */
    EXTERNAL(2);

    private final int copyFormat;

    CopyFormatEnum(int copyFormat) {
        this.copyFormat = copyFormat;
    }

    /**
     * getter
     *
     * @return copyFormat
     */
    public int getCopyFormat() {
        return copyFormat;
    }
}
