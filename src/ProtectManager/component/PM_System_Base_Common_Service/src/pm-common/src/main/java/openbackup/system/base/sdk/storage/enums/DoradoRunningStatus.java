package openbackup.system.base.sdk.storage.enums;

import com.fasterxml.jackson.annotation.JsonValue;

/**
 * dorado设备信息
 *
 * @author p00511147
 * @since 2020-11-10
 */
public enum DoradoRunningStatus {
    /**
     * 正常
     */
    NORMAL(1),
    /**
     * 未运行
     */
    NOT_RUNNING(3),
    /**
     * 正在上电
     */
    POWERING_ON(12),
    /**
     * 正在下电
     */
    POWERING_OFF(47),
    /**
     * 正在升级
     */
    UPGRADING(51);

    private int type;

    DoradoRunningStatus(int type) {
        this.type = type;
    }

    @JsonValue
    public int getType() {
        return type;
    }
}
