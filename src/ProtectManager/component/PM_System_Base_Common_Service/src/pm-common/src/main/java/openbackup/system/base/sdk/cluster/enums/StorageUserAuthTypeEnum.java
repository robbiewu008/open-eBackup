/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2024. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.enums;

import lombok.Getter;

/**
 * 存储单元组用户授权类型枚举类
 *
 * @author w00639094
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-20
 */
@Getter
public enum StorageUserAuthTypeEnum {
    UN_AUTH(0),
    UNIT(1),
    DISTRIBUTION(2);

    private final int authType;

    StorageUserAuthTypeEnum(int authType) {
        this.authType = authType;
    }
}
