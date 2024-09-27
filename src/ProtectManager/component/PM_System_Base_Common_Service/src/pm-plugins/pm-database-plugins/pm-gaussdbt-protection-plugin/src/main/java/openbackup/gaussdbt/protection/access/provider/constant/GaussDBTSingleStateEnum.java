package openbackup.gaussdbt.protection.access.provider.constant;

/**
 * GaussDBT单机版状态枚举
 *
 * @author lWX776769
 * @version [DataBackup 1.5.0]
 * @since 2023/7/21
 */
public enum GaussDBTSingleStateEnum {
    /**
     * 正常
     */
    NORMAL("Normal"),

    /**
     * 离线
     */
    OFFLINE("Offline");

    private final String state;

    /**
     * GaussDBTSingleStateEnum
     *
     * @param state 状态类型
     */
    GaussDBTSingleStateEnum(String state) {
        this.state = state;
    }

    /**
     * getter
     *
     * @return 类型
     */
    public String getState() {
        return state;
    }
}
