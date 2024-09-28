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
package com.huawei.emeistor.console.bean;

import lombok.Getter;
import lombok.Setter;

/**
 * OptAuth
 *
 */
@Setter
@Getter
public class OptAuthBo {
    // <变量的意义、目的、功能和可能被用到的地方>
    private static final long serialVersionUID = 4173890973126233693L;

    private long optId = 0L;

    private String optName = "";

    private String url = "";

    // 标识该操作是否是主
    private boolean isMaster = false;

    // 菜单状态（全选，半选，不选）
    private long listOperationStatus = 0L;

    private String serialNumber = "";
}
