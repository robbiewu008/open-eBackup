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
package com.huawei.oceanprotect.system.base.controller.log.context;

import lombok.Getter;
import lombok.Setter;

/**
 * 保存初始化配置上下文结果
 *
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/1/27
 */
@Getter
@Setter
public class InitConfigContextResult {
    private String role;

    private String homePortType;

    private String homePortId;
}
