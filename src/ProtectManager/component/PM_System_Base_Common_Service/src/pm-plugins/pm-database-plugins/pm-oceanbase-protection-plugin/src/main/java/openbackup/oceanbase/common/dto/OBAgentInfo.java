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
package openbackup.oceanbase.common.dto;

import lombok.Data;

/**
 * 功能描述
 *
 */
@Data
public class OBAgentInfo {
    /**
     * 主机agent节点的uuid
     */
    private String parentUuid;

    /**
     * 主机agent上OceanBase提供服务的IP
     */
    private String ip;

    /**
     * 主机agent上OceanBase提供服务的端口
     */
    private String port;

    /**
     * 节点类型，枚举: OBServer, OBClient
     */
    private String nodeType;

    /**
     * 节点状态
     */
    private String linkStatus;
}
