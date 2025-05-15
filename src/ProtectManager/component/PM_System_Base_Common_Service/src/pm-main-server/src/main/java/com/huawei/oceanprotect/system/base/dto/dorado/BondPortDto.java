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
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.RunningStatus;

import lombok.Data;

import java.util.List;

/**
 * 对外体现绑定端口
 *
 */
@Data
public class BondPortDto {
    /**
     * id
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
     * 健康状态
     */
    private HealthStatus healthStatus;

    /**
     * 运行状态
     */
    private RunningStatus runningStatus;

    /**
     * 端口id列表
     */
    private List<String> portIdList;

    /**
     * 端口名称str
     */
    private String bondInfo;

    /**
     * 端口mtu
     */
    private String mtu;
}
