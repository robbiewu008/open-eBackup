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
package openbackup.data.access.framework.protection.listener.v1.model;

import lombok.Data;

/**
 * 任务监控消息
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.1.0]
 * @since 2020-10-16
 */
@Data
public class JobMonitorBo {
    /**
     * 请求id
     */
    private String requestId;

    /**
     * 资源类型
     */
    private String resourceType;

    /**
     * 任务id
     */
    private String jobId;

    /**
     * 任务类型
     */
    private String jobType;

    /**
     * 标准备份计划id
     */
    private String planId;

    /**
     * 标准备份任务实例id
     */
    private String instanceId;
}
