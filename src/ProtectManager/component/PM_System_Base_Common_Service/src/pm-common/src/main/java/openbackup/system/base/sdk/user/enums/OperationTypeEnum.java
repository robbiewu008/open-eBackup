/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.user.enums;

import lombok.Getter;

/**
 * 功能描述
 *
 * @author x30046484
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-05-17
 */
@Getter
public enum OperationTypeEnum {
    /**
     * 创建
     */
    CREATE("create"),
    /**
     * 删除
     */
    DELETE("delete"),
    /**
     * 修改
     */
    MODIFY("modify"),
    /**
     * 查询详情
     */
    QUERY("query");

    private final String value;

    OperationTypeEnum(String value) {
        this.value = value;
    }
}
