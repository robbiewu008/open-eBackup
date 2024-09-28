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
 * 创建快照请求参数
 * 支持Dorado/OceanProtect/Pacific设备
 *
 */
@Data
public class CreateFsSnapshotRequest {
    /**
     * 请求id
     */
    private String requestId;

    /**
     * 任务id
     */
    private String taskId;

    /**
     * 文件系统名
     */
    private String filesystemName;

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
     * 资源ID
     */
    private String resourceId;

    /**
     * 租户名
     */
    private String vstoreName;

    /**
     * 安全快照保留时间；指定创建的安全快照的保留时间，最大保留时间为7300天，240月和20年；仅在创建安全快照时字段有效，且不能为0。
     */
    private int retentionDuration;

    /**
     * 创建安全快照时，指定保留时间的单位；仅支持日，月，年；46代表保留时间的单位为日，47代表月，48代表年
     */
    private int retentionType;

    /**
     * 保留时间单位字符串表示
     */
    private String durationUnit;

    /**
     * sla json字符串
     */
    private String slaString;

    /**
     * sla名
     */
    private String slaName;

    /**
     * 副本ID
     */
    private String copyId;

    /**
     * 设备ESN号
     */
    private String deviceEsn;

    /**
     * 副本链ID
     */
    private String chainId;

    /**
     * 租户ID
     */
    private String vstoreId;
}
