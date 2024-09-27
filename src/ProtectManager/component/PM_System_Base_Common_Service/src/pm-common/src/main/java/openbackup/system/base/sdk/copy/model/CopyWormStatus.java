package openbackup.system.base.sdk.copy.model;

/**
 * 副本Worm状态
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-10-20
 */
public enum CopyWormStatus {
    /**
     * 普通副本
     */
    UNSET(1),
    /**
     * 未设置WORM
     */
    SETTING(2),
    /**
     * WORM设置成功
     */
    SET_SUCCESS(3),
    /**
     * WORM设置失败
     */
    SET_FAILED(4);

    private final int status;

    CopyWormStatus(int status) {
        this.status = status;
    }

    /**
     * 获取status
     *
     * @return status
     */
    public int getStatus() {
        return status;
    }
}
