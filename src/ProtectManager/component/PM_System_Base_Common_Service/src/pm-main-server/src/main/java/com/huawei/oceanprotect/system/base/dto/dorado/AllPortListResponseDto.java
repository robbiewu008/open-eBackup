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
package com.huawei.oceanprotect.system.base.dto.dorado;

import com.huawei.oceanprotect.system.base.model.VlanPo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.logicport.LogicPortAddRequest;

import lombok.Data;

import java.util.List;

/**
 * 发送给外部所有端口列表集合
 *
 */
@Data
public class AllPortListResponseDto {
    /**
     * 绑定端口列表
     */
    List<BondPortDto> bondPortList;

    /**
     * 以太网端口
     */
    List<EthPortDto> ethPortDtoList;

    /**
     * PM逻辑端口列表
     */
    List<LogicPortDto> logicPortDtoList;

    /**
     * 底座逻辑端口列表
     */
    List<LogicPortAddRequest> dmLogicPortList;

    /**
     * vlan列表
     */
    List<VlanPo> vlanList;

    /**
     * 复用逻辑端口名称列表
     */
    List<String> reuseLogicPortNameList;

    /**
     * 是否有不生效的端口
     */
    boolean isAllLogicPortsValid;
}
