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
package openbackup.tidb.resources.access.model;

import lombok.Getter;
import lombok.Setter;

/**
 * cluster Info model
 *
 */
@Getter
@Setter
public class ClusterInfo {
    /**
     * 集群id
     */
    private String id;

    /**
     * 集群角色
     */
    private String role;

    /**
     * 集群host
     */
    private String host;

    /**
     * 集群节点状态
     */
    private String status;

    /**
     * 集群节点uuid
     */
    private String hostManagerResourceUuid;
}
