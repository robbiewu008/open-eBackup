/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package com.huawei.oceanprotect.system.base.initialize.network.beans;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.FailovergroupMemberType;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

/**
 * 基础端口
 *
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
