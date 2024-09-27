package openbackup.system.base.common.enums;

import lombok.Getter;

/**
 * 网络类型枚举类，目前业务网络，管理网络
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/6/4
 */
@Getter
public enum NetworkTypeEnum {
    MANAGEMENT(0),
    BUSINESS(1);
    private final int networkType;

    NetworkTypeEnum(int networkType) {
        this.networkType = networkType;
    }
}
