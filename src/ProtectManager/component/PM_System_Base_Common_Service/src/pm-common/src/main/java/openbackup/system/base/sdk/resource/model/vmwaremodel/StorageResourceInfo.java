/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model.vmwaremodel;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 注册VMware时所填的存储信息
 *
 * @author z30047175
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-02-18
 */
@Setter
@Getter
public class StorageResourceInfo {
    // ip列表
    private List<String> ip;

    // 端口
    private int port;

    // 存储用户名
    private String username;

    // 存储密码
    private String password;

    // 存储类型：0 DoradoV6，1 NetApp
    private String type;
}