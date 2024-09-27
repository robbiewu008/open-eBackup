/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model;

import lombok.Getter;
import lombok.Setter;

/**
 * 调cmdb接口查询ip时封装的请求参数 Simple
 *
 * @author z00664377
 * @since 2023-08-04
 */
@Getter
@Setter
public class Simple {
    private String name;
    private String value;
}
