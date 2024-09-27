/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.copy.model;

import lombok.Data;

/**
 * 更新副本扩展参数对象
 *
 * @author: y00559272
 * @version: [OceanProtect X8000 1.2.1]
 * @since: 2022/8/5
 **/
@Data
public class UpdateCopyPropertiesRequest {
    /**
     * 需要更新扩展参数的key值
     */
    private String key;

    /**
     * 需要更新的目标值
     */
    private String value;
}
