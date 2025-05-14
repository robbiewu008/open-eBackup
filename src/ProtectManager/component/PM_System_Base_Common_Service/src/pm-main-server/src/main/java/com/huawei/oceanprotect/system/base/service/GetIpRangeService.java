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

import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;

import openbackup.system.base.util.Applicable;

import java.util.List;

/**
 * 处理ip段的接口
 *
 * @since 2021-11-29
 */
public interface GetIpRangeService extends Applicable<String> {
    /**
     * 获取处理后的IP段信息
     *
     * @param netMask 掩码
     * @param netPlaneName 平面网络名字
     * @param service DM 对象
     * @return 返回对应的IP段信息
     */
    List getResource(String netMask, String netPlaneName, DeviceManagerService service);

    /**
     * 获取处理后的路由
     *
     * @param netPlaneId 平面网络ID
     * @param service DM对象
     * @return 路由信息
     */
    List getRouteInfo(String netPlaneId, DeviceManagerService service);
}
