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
package openbackup.data.protection.access.provider.sdk.restore.v2;

import lombok.Data;

/**
 * 实时侦测安全快照相关任务实体类
 *
 **/
@Data
public class DeeCopiesRelatedTask {
    /**
     * 任务id
     */
    private String taskId;

    /**
     * 请求id
     */
    private String requestId;

    /**
     * 副本id
     */
    private String snapshotId;

    /**
     * 副本名称
     */
    private String snapshotName;

    /**
     * 文件系统id(DM)
     */
    private String filesystemId;

    /**
     * 文件系统名称
     */
    private String filesystemName;

    /**
     * 租户id
     */
    private String vstoreId;

    /**
     * 租户名称
     */
    private String vstoreName;

    /**
     * 设备id
     */
    private String deviceId;

    /**
     * 设备名称
     */
    private String deviceName;

    /**
     * 过期时间 单位天 永久保留不传
     */
    private Long retentionDay;

    /**
     * 资源ID(PM)
     */
    private String resourceId;

    /**
     * 类型
     */
    private String subType;
}
