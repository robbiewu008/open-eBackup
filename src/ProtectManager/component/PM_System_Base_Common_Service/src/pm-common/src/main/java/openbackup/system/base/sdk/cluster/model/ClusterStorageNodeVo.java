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
package openbackup.system.base.sdk.cluster.model;

import lombok.Data;

/**
 * Storage nodes vo
 *
 */
@Data
public class ClusterStorageNodeVo {
    private String nodeId;

    private String nodeName;

    private Integer nodeRole;

    private String managementIPv4;

    private String managementIPv6;

    private Integer status;

    // 数据备份引擎IP
    private String backupEngineIp;

    // 数据利用引擎IP
    private String deeEngineIp;

    // 数据归档引擎IP
    private String archiveEngineIp;

    // 复制网络ip
    private String copyEngineIp;

    // 健康状态
    private String healthStatus;
}
