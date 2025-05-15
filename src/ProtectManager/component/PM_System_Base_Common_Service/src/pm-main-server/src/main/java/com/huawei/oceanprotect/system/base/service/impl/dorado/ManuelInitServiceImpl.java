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
package com.huawei.oceanprotect.system.base.service.impl.dorado;

import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.enums.InitType;
import com.huawei.oceanprotect.system.base.service.InitConfigMethodService;

import org.springframework.stereotype.Service;

/**
 * 手动初始化特有方法
 *
 */
@Service
public class ManuelInitServiceImpl implements InitConfigMethodService {
    @Override
    public boolean applicable(InitType initType) {
        return initType.isManualInit();
    }

    @Override
    public void addLogicPort(InitNetworkBody initNetworkBody) {

    }
}
