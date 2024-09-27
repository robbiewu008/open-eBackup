package openbackup.data.protection.access.provider.sdk.enums;

/**
 * agent执行任务时的挂载方法
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-07-12
 */
public enum AgentMountTypeEnum {
    /**
     * 普通挂载
     */
    MOUNT("mount"),

    /**
     * fuse挂载（本地盘）
     */
    FUSE("fuse");

    private final String mountType;

    /**
     * agent执行任务时的挂载方法
     *
     * @param mountType 挂载类型
     */
    AgentMountTypeEnum(String mountType) {
        this.mountType = mountType;
    }

    /**
     * 获取枚举值
     *
     * @return 挂载参数
     */
    public String getValue() {
        return mountType;
    }
}
