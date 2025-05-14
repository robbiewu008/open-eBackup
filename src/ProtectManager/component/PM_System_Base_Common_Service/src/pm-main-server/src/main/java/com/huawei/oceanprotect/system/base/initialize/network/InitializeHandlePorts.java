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
package com.huawei.oceanprotect.system.base.initialize.network;

import com.huawei.oceanprotect.system.base.dto.dorado.AllPortListResponseDto;
import com.huawei.oceanprotect.system.base.dto.dorado.BondPortDto;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.ethport.EthPort;

import java.util.List;
import java.util.Map;

/**
 * 获取端口列表
 *
 */
public interface InitializeHandlePorts {
    /**
     * 根据过滤条件获取端口列表
     *
     * @param deviceId deviceId
     * @param username username
     * @param filter 过滤条件
     * @return 所有的端口信息
     */
    AllPortListResponseDto getPorts(String deviceId, String username, Map<String, String> filter);

    /**
     * 获取根据传入的以太网端口location地址集合所有的以太网信息
     *
     * @param deviceId deviceId
     * @param username username
     * @param homePortNameList 以太网端口location地址集合
     * @return 以太网信息列表
     */
    List<EthPort> getEthPortList(String deviceId, String username, List<String> homePortNameList);

    /**
     * 获取所有的绑定端口列表
     *
     * @param deviceId deviceId
     * @param username username
     * @return 绑定端口列表
     */
    List<BondPortDto> getBondPort(String deviceId, String username);
}
