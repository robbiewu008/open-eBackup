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
package com.huawei.oceanprotect.system.base.service;

import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.enums.InitType;

import openbackup.system.base.util.Applicable;

/**
 * 初始化配置方式服务
 *
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/4/5
 */
public interface InitConfigMethodService extends Applicable<InitType> {
    /**
     * 添加逻辑端口
     *
     * @param initNetworkBody 请求参数
     */
    void addLogicPort(InitNetworkBody initNetworkBody);
}
