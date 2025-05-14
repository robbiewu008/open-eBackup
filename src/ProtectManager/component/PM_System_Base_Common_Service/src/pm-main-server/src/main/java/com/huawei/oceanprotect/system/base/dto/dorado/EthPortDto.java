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

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HealthStatus;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.LogicType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.RunningStatus;

import lombok.Data;

/**
 * 对外体现以太网端口
 *
 * @since 2022-12-13
 */
@Data
public class EthPortDto {
    /**
     * 编号
     */
    private String id;

    /**
     * 名称
     */
    private String name;

    /**
     * 端口归属控制器
     */
    private String ownIngController;

    /**
     * 端口逻辑类型
     */
    private LogicType logicType;

    /**
     * 端口物理类型
     */
    private String physicalType;

    /**
     * 健康状态
     */
    private HealthStatus healthStatus;

    /**
     * 运行状态
     */
    private RunningStatus runningStatus;

    /**
     * MAC地址
     */
    private String macAddress;

    /**
     * 位置
     */
    private String location;

    /**
     * 最大传输单元
     */
    private long mtu;

    /**
     * 最大工作效率
     */
    private String maxSpeed;

    /**
     * 绑定端口名称
     */
    private String bondName;

    /**
     * 太网口所在的任一漂移组
     */
    private String failOverGroup;
}
