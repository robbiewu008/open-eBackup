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
package com.huawei.oceanprotect.system.base.util;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.netplane.NetPlane;

import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

/**
 * DM 返回值处理测试类
 *
 * @author swx1010572
 * @version: [OceanProtect DataBackup 1.3.0]
 * @since 2023-01-09
 */
public class DeviceManagerResponseUtilsTest {
    /**
     * 用例场景：测试返回的平面网络满足要求
     * 前置条件：满足条件
     * 检查点：成功
     */
    @Test
    public void pour_net_plane_list_success() {
        List<NetPlane> netPlaneList = new ArrayList<>();
        NetPlane netPlane1 = new NetPlane();
        netPlane1.setName("backupNetPlane");
        netPlaneList.add(netPlane1);
    }
}
