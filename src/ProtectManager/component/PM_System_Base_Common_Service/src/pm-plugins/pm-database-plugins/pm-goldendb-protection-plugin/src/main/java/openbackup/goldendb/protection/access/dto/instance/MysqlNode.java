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
package openbackup.goldendb.protection.access.dto.instance;

import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 功能描述 mysql节点
 *
 */
@NoArgsConstructor
@Data
public class MysqlNode {
    /**
     * uuid唯一id
     */
    private String uuid;

    /**
     * id
     */
    private String id;

    /**
     * name名称
     */
    private String name;

    /**
     * role角色
     */
    private String role;

    /**
     * ip
     */
    private String ip;

    /**
     * port端口
     */
    private String port;

    /**
     * osUser
     */
    private String osUser;

    /**
     * nodeType节点类型
     */
    private String nodeType;

    /**
     * parentUuid agentId
     */
    private String parentUuid;

    /**
     * 连接状态
     */
    private String linkStatus;

    /**
     * 分组
     */
    private String group;

    /**
     * agent
     */
    private String parent;

    /**
     * agent name
     */
    private String parentName;
}
