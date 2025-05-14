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
package com.huawei.oceanprotect.system.base.initialize.backstorage;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;

/**
 * 初始化动作
 *
 * @since 2020-12-21
 */
public interface InitializeFileSystems {
    /**
     * 创建文件系统类型
     *
     * @param service 本地存储Service
     * @return 文件系统类型ID
     */
    long attainSetWorkloadTypeId(DeviceManagerService service);
}