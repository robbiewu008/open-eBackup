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

import com.huawei.oceanprotect.system.base.initialize.network.enums.InitNetworkResultCode;

import lombok.Data;
import lombok.NoArgsConstructor;
import lombok.ToString;

import java.util.LinkedList;
import java.util.List;

/**
 * 初始化动作结果
 *
 * @since 2021-01-15
 */
@Data
@ToString
@NoArgsConstructor
public class InitNetworkResult {
    /**
     * 初始化动作描述列表
     */
    private List<InitNetworkResultDesc> network = new LinkedList<>();

    /**
     * 默认构造函数
     *
     * @param code 编码
     * @param desc 描述
     */
    public InitNetworkResult(InitNetworkResultCode code, String desc) {
        network.add(new InitNetworkResultDesc(code, desc));
    }

    /**
     * 增加动作错误
     *
     * @param code 动作结果编码
     * @param desc 动作结果描述
     * @return 自身
     */
    public InitNetworkResult addInitBackActionResult(InitNetworkResultCode code, String desc) {
        network.add(new InitNetworkResultDesc(code, desc));
        return this;
    }

    /**
     * 增加动作结果
     *
     * @param initNetworkResult 动作结果
     * @return 自身
     */
    public InitNetworkResult addInitBackActionResult(InitNetworkResult initNetworkResult) {
        network.addAll(initNetworkResult.getNetwork());
        return this;
    }

    /**
     * 初始化是否OK
     *
     * @return 是否OK
     */
    public boolean isOkay() {
        for (InitNetworkResultDesc desc : network) {
            if (!desc.getCode().isOkay()) {
                return false;
            }
        }
        return true;
    }
}
