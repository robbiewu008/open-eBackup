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

import lombok.Getter;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

import java.util.List;

/**
 * 业务端口存放的绑定端口信息
 *
 */
@Getter
@Setter
public class BondPortPo {
    /**
     * 绑定端口id
     */
    private String id;

    /**
     * 绑定端口名称
     */
    @Length(max = 31, message = "The length of port is 1 ~ 31.")
    private String name;

    /**
     * 以太网端口的名称列表
     */
    private List<String> portNameList;

    /**
     * 最大传输单元
     */
    private String mtu;
}
