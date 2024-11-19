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
package openbackup.ndmp.protection.access.common;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.ndmp.protection.access.constant.NdmpConstant;

import java.util.Map;

/**
 * Ndmp公共函数
 *
 */
@Slf4j
public class NdmpCommon {
    /**
     * 设置Ndmp设备名称，租户名称
     *
     * @param protectedResource protectedResource
     */
    public static void setNdmpNames(ProtectedResource protectedResource) {
        String fullName = protectedResource.getName();
        String[] fullNames = fullName.split("/");
        if (fullNames.length == 3) {
            String name = fullNames[2];
            String tenant = fullNames[1];
            protectedResource.setName(name);
            Map<String, String> extendInfo = protectedResource.getExtendInfo();
            extendInfo.put(NdmpConstant.TENANT_NAME, tenant);
            extendInfo.put(NdmpConstant.FULL_NAME, fullName);
        } else if (fullNames.length == 2) {
            // unity没有租户，返回的fullname格式为：/ifs
            Map<String, String> extendInfo = protectedResource.getExtendInfo();
            String name = fullNames[1];
            protectedResource.setName(name);
            extendInfo.put(NdmpConstant.FULL_NAME, fullName);
        } else {
            log.info("Failed to get tenant name.");
        }
    }
}
