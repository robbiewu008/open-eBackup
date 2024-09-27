package openbackup.system.base.common.model.repository;

/**
 * 功能描述
 *
 * @author y00413474
 * @since 2020-06-29
 */
public enum RepositoryType {
    /**
     * 本地存储库类型
     */
    LOCAL(1),
    /**
     * S3存储库类型
     */
    S3(2),
    /**
     * 蓝光存储库类型
     */
    BLUE_RAY(3),
    /**
     * 磁带存储库类型
     */
    TAPE(4),
    /**
     * 外部存储库类型
     */
    EXTERNAL(5);

    private final int type;

    RepositoryType(int type) {
        this.type = type;
    }

    public int getType() {
        return type;
    }

    /**
     * 根据传入的值获取对应的枚举值
     *
     * @param value value
     * @return RepositoryType
     */
    public static RepositoryType getValue(int value) {
        for (RepositoryType msgType : RepositoryType.values()) {
            if (value == msgType.type) {
                return msgType;
            }
        }
        return RepositoryType.LOCAL;
    }
}
