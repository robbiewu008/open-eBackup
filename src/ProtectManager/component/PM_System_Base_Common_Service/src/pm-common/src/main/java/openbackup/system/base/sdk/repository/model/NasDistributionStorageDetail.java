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
package openbackup.system.base.sdk.repository.model;

import lombok.Data;

import java.util.List;

/**
 * 分布式NAS存储库详情
 *
 * @author nwx1077006
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-14
 */
@Data
public class NasDistributionStorageDetail {
    /**
     * 存储库ID
     */
    private String uuid;

    /**
     * 存储库名称
     */
    private String name;

    /**
     * 描述
     */
    private String description;

    /**
     * 类型
     */
    private String type;

    /**
     * 存储策略
     */
    private Integer storageStrategyType;

    /**
     * 故障切换 超时时间
     */
    private Integer timeoutPeriod;

    /**
     * 是否开启并行存储
     */
    private boolean hasEnableParallelStorage;

    /**
     * 备份存储库集群信息
     */
    private List<BackupUnitVo> unitList;

    /**
     * 设备类型
     */
    private String deviceType;
}
