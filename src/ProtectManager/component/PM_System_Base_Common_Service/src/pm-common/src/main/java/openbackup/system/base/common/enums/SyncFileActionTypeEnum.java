package openbackup.system.base.common.enums;

import lombok.Getter;

/**
 * 同步文件到所有节点的动作枚举类
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.7.0]
 * @since 2024-07-29
 */
@Getter
public enum SyncFileActionTypeEnum {
    /**
     * 添加
     */
    ADD(0),

    /**
     * 删除
     */
    DELETE(1);

    private final int type;

    SyncFileActionTypeEnum(int type) {
        this.type = type;
    }
}
