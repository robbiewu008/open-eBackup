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

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.RunningStatus;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.VlanPortType;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 业务端口存放的vlan信息
 *
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/1/22
 */
@Getter
@Setter
public class VlanPo {
    /**
     * vlan id
     */
    private String id;

    /**
     * 创建逻辑端口时增加的多个vlan id,范围1-4094
     */
    private List<String> tags;

    /**
     * 端口类型
     */
    private VlanPortType portType;

    /**
     * 绑定端口的id
     */
    private String bondPortId;

    /**
     * 以太网端口的名称列表
     */
    private List<String> portNameList;

    /**
     * 运行状态
     */
    private RunningStatus runningStatus;

    /**
     * 端口名称
     */
    private String name;

    /**
     * 最大传输单元
     */
    private String mtu;

    /**
     * 添加的端口id
     */
    private String portId;
}
