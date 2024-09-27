package openbackup.system.base.common.enums;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 设备类型
 *
 * @author g00500588
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/8/2
 */
@Getter
@AllArgsConstructor
public enum StorageDeviceTypeEnum {
    DORADO("dorado"),
    PACIFIC("pacific");

    private String type;
}