/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 *
 */

package com.huawei.oceanprotect.system.base.initialize.network.beans;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.FailovergroupMemberType;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

/**
 * 基础端口
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/4/25
 */
public class BasePort {
    /**
     * 查询主端口id
     *
     * @param condition 查询条件
     * @return 主端口id
     */
    public String queryHomePortId(String condition) {
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Port type error.");
    }

    /**
     * 查询主端口名称
     *
     * @param portId 查询条件:portId
     * @return 主端口名称
     */
    public String queryHomePortName(String portId) {
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Port type error.");
    }

    /**
     * 根据当前端口类型转换为漂移组成员端口类型
     *
     * @return 漂移组成员端口类型
     */
    public FailovergroupMemberType convertToFailOverGroupMemberType() {
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Port type error.");
    }
}
