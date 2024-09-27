/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.repository.api;

import java.util.Optional;

/**
 * ProductStorageService SDK
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-03-16
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