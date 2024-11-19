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
package openbackup.system.base.sdk.repository.api;

import java.util.Optional;

/**
 * ProductStorageService SDK
 *
 */
public interface ProductStorageApi {
    /**
     * 查询指定设备的业务认证用户名
     *
     * @param esn esn
     * @return 用户名
     */
    Optional<String> getServiceUsername(String esn);

    /**
     * 查询auth_type为0的存储信息的名称
     *
     * @param deviceId 本地存储设备的device_id
     * @return 业务认证用户名，如果没有就传空字符串
     */
    String getBusinessAuthNameByDeviceId(String deviceId);
}