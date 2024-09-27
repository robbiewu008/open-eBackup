package openbackup.system.base.common.enums;

import lombok.Getter;

import java.util.Arrays;

/**
 * OceanStorageEthernetPortQuery
 *
 * @author 存储设备端口类型
 * @since 2021-05-06
 */
@Getter
public enum StoragePortType {
    MANAGEMENT_PORT(2);

    private final int type;

    StoragePortType(int type) {
        this.type = type;
    }

    /**
     * 根据端口类型获取枚举
     *
     * @param type 端口类型
     * @return StoragePortType
     */
    public static StoragePortType getByPort(int type) {
        return Arrays.stream(StoragePortType.values())
            .filter(portType -> portType.type == type)
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }
}
