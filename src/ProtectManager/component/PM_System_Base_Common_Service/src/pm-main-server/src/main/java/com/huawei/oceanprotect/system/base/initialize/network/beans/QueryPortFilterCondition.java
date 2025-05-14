/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.beans;

import lombok.Getter;
import lombok.Setter;

/**
 * 从底座查询端口时的过滤条件
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/1/24
 */
@Getter
@Setter
public class QueryPortFilterCondition {
    private String name;

    private String id;
}
