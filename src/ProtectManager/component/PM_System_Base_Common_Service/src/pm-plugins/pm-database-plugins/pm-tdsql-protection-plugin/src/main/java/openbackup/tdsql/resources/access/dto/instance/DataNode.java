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
package openbackup.tdsql.resources.access.dto.instance;

import lombok.Data;

/**
 * mysqlNodes
 *
 * @author z30047175
 * @since 2023-05-24
 */
@Data
public class DataNode {
    /**
     * 节点ip
     */
    private String ip;

    /**
     * port端口号
     */
    private String port;

    /**
     * mysql conf文件路径
     */
    private String defaultsFile;

    /**
     * mysql socket文件路径
     */
    private String socket;

    /**
     * 1-主节点 0-备节点
     */
    private String isMaster;

    /**
     * 节点优先级 1-high 2-medium 3-low
     */
    private String priority;

    /**
     * 节点类型 数据节点
     */
    private String nodeType;

    /**
     * 代理主机uuid 前端
     */
    private String parentUuid;

    /**
     * 节点状态
     */
    private String linkStatus;
}
