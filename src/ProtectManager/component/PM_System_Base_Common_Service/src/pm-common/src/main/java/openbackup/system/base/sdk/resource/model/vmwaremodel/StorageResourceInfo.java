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