package openbackup.data.protection.access.provider.sdk.enums;

/**
 * 仓库挂载类型Enum
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-11
 */
public enum MountTypeEnum {
    /**
     * 非全路径挂载
     */
    NON_FULL_PATH_MOUNT("0"),

    /**
     * 全路径挂载
     */
    FULL_PATH_MOUNT("1");

    private final String mountType;

    MountTypeEnum(String mountType) {
        this.mountType = mountType;
    }

    /**
     * getter
     *
     * @return mountType
     */
    public String getMountType() {
        return mountType;
    }
}
