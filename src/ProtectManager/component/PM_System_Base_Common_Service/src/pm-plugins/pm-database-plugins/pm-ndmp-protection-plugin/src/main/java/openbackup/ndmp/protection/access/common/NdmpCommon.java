/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.ndmp.protection.access.common;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import lombok.extern.slf4j.Slf4j;

import java.util.Map;

/**
 * Ndmp公共函数
 *
 * @author s30043630
 * @version [OceanProtect X8000 1.6.0]
 * @since 2024-06-12
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
            extendInfo.put("tenantName", tenant);
            extendInfo.put("fullName", fullName);
        } else {
            log.info("Failed to get tenant name.");
        }
    }
}
