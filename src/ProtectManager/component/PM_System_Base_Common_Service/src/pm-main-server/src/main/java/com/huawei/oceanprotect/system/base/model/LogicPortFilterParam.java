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
package com.huawei.oceanprotect.system.base.model;

import lombok.Getter;
import lombok.Setter;
import openbackup.system.base.common.utils.VerifyUtil;

import org.hibernate.validator.constraints.Length;

/**
 * 查询逻辑端口过滤参数
 *
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
