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
package openbackup.access.framework.resource.client.model;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * 更新设备请求体
 *
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class UpdateDeviceRequest {
    /**
     * 设备ID
     */
    private String id;

    /**
     * 设备名称
     */
    private String name;

    /**
     * 设备状态
     */
    private String status;

    /**
     * 设备类型
     */
    private String type;

    /**
     * 设备地址
     */
    private String ip;

    /**
     *  设备端口
     */
    private String port;

    /**
     * 用户名称
     */
    private String userName;

    /**
     * 用户密码
     */
    private String userPwd;

    /**
     * 是否校验证书
     */
    private String enableCert;

    /**
     * 证书
     */
    private String certification;

    /**
     * 吊销列表
     */
    private String revocationList;

    /**
     * 所属用户
     */
    private String deviceBelongUserId;

    /**
     * 租户信息
     */
    private List<Vstore> vstores;
}
