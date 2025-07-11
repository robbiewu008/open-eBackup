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
package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Data;

import java.util.List;
import java.util.Map;

/**
 * 集群信息查询接口返回对象
 *
 */
@Data
public class AppEnvResponse {
    /**
     * 资源ID
     */
    private String uuid;

    /**
     * 资源名称
     */
    private String name;

    /**
     * 资源类型
     */
    private String type;

    /**
     * 资源子类型
     */
    private String subType;

    /**
     * 访问地址，ip或者域名
     */
    private String endpoint;

    /**
     * 节点信息
     */
    private List<NodeInfo> nodes;

    /**
     * 节点角色类型 0-none 1-active 2-standby 3-shard
     */
    private Integer role;

    /**
     * 资源的扩展信息
     */
    private Map<String, String> extendInfo;
}