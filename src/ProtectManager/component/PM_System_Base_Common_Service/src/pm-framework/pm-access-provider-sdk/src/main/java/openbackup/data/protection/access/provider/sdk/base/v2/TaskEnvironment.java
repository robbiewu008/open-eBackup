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
package openbackup.data.protection.access.provider.sdk.base.v2;

import lombok.Data;

import java.util.List;

/**
 * RestoreAppEnvironment
 *
 * @description: 恢复任务中的资源对应的环境信息
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/11/30
 **/
@Data
public class TaskEnvironment extends TaskCommonResource {
    /**
     * 访问地址，ip或者域名
     */
    private String endpoint;

    /**
     * 端口
     */
    private Integer port;

    /**
     * 集群环境对应的节点信息，环境为非集群环境时，该值为空
     */
    private List<TaskEnvironment> nodes;

    /**
     * 环境的连接状态，框架校验使用（序列化时忽略）
     */
    private String linkStatus;
}
