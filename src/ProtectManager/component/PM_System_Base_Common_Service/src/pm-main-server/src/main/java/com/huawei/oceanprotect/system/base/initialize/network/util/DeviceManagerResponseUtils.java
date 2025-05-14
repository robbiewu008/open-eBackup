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
package com.huawei.oceanprotect.system.base.initialize.network.util;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.netplane.NetPlane;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import java.util.List;

/**
 * DM 返回值处理公共类
 *
 */
public class DeviceManagerResponseUtils {
    /**
     * 平面网络获取公共方法
     *
     * @param name 平面网络名称
     * @param netPlaneList 全部平面网络列表
     * @return 获取的及时平面网络信息
     */
    public static NetPlane pourNetPlaneList(String name, List<NetPlane> netPlaneList) {
        return netPlaneList.stream()
            .filter(netPlane -> name.equals(netPlane.getName()))
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OPERATION_FAILED));
    }
}
