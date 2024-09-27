/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.auth;

import lombok.Data;

/**
 * RoleInfo from rest request
 *
 * @author dwx1009286
 * @version [OceanProtect 1.1.0]
 * @since 2022-02-24
 */
@Data
public class RoleInfo {
    // 角色名称（系统管理员、数据保护管理员等）
    private String roleName = "";
}