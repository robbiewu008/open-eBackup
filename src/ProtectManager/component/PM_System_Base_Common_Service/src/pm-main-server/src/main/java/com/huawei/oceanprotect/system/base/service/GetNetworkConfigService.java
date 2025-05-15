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

import com.huawei.oceanprotect.system.base.initialize.network.enums.NetworkType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;

import openbackup.system.base.util.Applicable;

/**
 * 处理网络配置的接口
 *
 */
public interface GetNetworkConfigService<T> extends Applicable<String> {
    /**
     * 获取网络配置信息
     *
     * @param service DM 对象
     * @param ipType ip类型
     * @param networkType 网络类型
     * @return 网络平面信息对象
     */
    T getNetworkConfig(DeviceManagerService service, String ipType, NetworkType networkType) ;
}
