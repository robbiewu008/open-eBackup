/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 *
 */

package com.huawei.oceanprotect.system.base.model;

import lombok.Getter;
import lombok.Setter;
import openbackup.system.base.common.utils.VerifyUtil;

import org.hibernate.validator.constraints.Length;

/**
 * 查询逻辑端口过滤参数
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/3/27
 */
@Getter
@Setter
public class LogicPortFilterParam {
    @Length(max = 256)
    private String portName;
    @Length(max = 256)
    private String portId;
    @Length(max = 256)
    private String ethLogicTypeValue;

    /**
     * 查询对象是否为空
     *
     * @return true：为空，false：不为空
     */
    public boolean isEmpty() {
        return VerifyUtil.isEmpty(portName) && VerifyUtil.isEmpty(portId) && VerifyUtil.isEmpty(ethLogicTypeValue);
    }
}
