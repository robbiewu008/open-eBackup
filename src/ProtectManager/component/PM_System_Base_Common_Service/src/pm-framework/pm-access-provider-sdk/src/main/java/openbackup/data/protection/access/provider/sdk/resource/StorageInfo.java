/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 功能描述 存储设备信息DTO
 *
 * @author s30031954
 * @since 2022-12-21
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class StorageInfo {
    // 租户ID
    private String tenantId;

    // 设备ID
    private String deviceId;

    // 地址
    private String endpoint;

    // 端口
    private Integer port;

    // 用户名
    private String userName;

    // 密码
    private String password;

    // 类型
    private String type;

    // 用户ID
    private String userId;
}