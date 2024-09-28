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
 * 安全一体机基于快照创建克隆请求参数
 *
 */
@Data
public class OcLiveMountCloneReq {
    /**
     * 请求id
     */
    private String requestId;

    /**
     * 任务id
     */
    private String taskId;

    /**
     * 存储设备id
     */
    private String deviceId;

    /**
     * 克隆文件系统名
     */
    private String name;

    /**
     * 基于文件系统的ID
     */
    private String parentFileSystemId;

    /**
     * 基于文件系统的名称
     */
    private String parentFileSystemName;

    /**
     * 父文件系统快照ID
     */
    private String parentSnapshotId;

    /**
     * 父文件系统快照名称
     */
    private String parentSnapshotName;

    /**
     * 租户Id
     */
    private String vstoreId;
}
