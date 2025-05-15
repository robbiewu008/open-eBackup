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

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortRole;

import lombok.Getter;
import lombok.Setter;

/**
 * 映射初始化表中存放的业务端口
 *
 */

@Getter
@Setter
public class ServicePortPo {
    /**
     * 业务端口名称
     */
    private String name;

    /**
     * 业务端口id
     */
    private String id;

    /**
     * 端口类型:1-以太网,7-绑定端口,8-Vlan
     */
    private HomePortType homePortType;

    /**
     * 业务端口所属控制器
     */
    private String currentControllerId;

    /**
     * 业务端口配置的Vlan信息
     */
    private VlanPo vlan;

    /**
     * 业务端口配置的绑定端口
     */
    private BondPortPo bondPort;

    /**
     * 业务端口界面展示的角色，备份：2、复制：4、归档：11
     */
    private PortRole role;

    /**
     * 底座真正创建的角色类型
     */
    private PortRole dmRole;
}
