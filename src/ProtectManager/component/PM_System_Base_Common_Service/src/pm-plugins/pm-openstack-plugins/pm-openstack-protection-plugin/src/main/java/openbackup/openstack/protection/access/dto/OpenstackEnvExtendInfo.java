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
package openbackup.openstack.protection.access.dto;

import lombok.Getter;
import lombok.Setter;

/**
 * OpenStack环境所需扩展字段
 *
 * @author c30016231
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-01-16
 */
@Getter
@Setter
public class OpenstackEnvExtendInfo {
    /**
     * service id
     */
    private String serviceId;

    /**
     * 是否注册service到OpenStack
     */
    private String registerService;

    /**
     * 注册service的代理ip
     */
    private String cpsIp;
}
