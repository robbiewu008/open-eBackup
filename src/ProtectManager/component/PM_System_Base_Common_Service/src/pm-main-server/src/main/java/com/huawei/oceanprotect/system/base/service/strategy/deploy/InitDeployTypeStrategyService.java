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
package com.huawei.oceanprotect.system.base.service.strategy.deploy;

import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.springframework.web.multipart.MultipartFile;

/**
 * 部署形态策略接口
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-08-20
 */
public interface InitDeployTypeStrategyService {
    /**
     * 根据lld获取初始化网络配置信息
     *
     * @param lld lld
     * @return 网络配置信息
     */
    default InitNetworkBody getInitNetworkBodyByLLD(MultipartFile lld) {
        throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
            "Current device is not support init by lld.");
    }
}
