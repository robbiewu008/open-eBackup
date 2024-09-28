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
package openbackup.data.access.client.sdk.api.framework.dee.model;

import lombok.Data;

/**
 * 删除快照请求参数
 * 支持Dorado/OceanProtect/Pacific设备
 *
 */
@Data
public class DeleteFsSnapshotRequest {
    /**
     * 请求id
     */
    private String requestId;

    /**
     * 任务id
     */
    private String taskId;

    /**
     * 快照名
     */
    private String snapshotName;

    /**
     * Dorado/OceanProtect设备：文件系统ID
     * Pacific设备：命名空间ID
     */
    private String filesystemId;

    /**
     * 存储设备ID
     */
    private String deviceId;

    /**
     * 租户ID
     */
    private String vstoreId;

    /**
     * 生成方式
     */
    private String generateBy;
}
