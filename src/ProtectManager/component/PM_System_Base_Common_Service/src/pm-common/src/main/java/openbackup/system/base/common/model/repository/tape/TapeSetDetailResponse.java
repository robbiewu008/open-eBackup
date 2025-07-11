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
package openbackup.system.base.common.model.repository.tape;

import lombok.Data;

import java.util.List;

/**
 * 介质集详情
 *
 */
@Data
public class TapeSetDetailResponse {
    /**
     * 介质集UUID
     */
    private String mediaSetId;

    /**
     * 介质集名
     */
    private String mediaSetName;

    /**
     * 介质集类型
     */
    private TapeWorm type;

    /**
     * 介质集支持的应用类型
     */
    private List<MediaSetAppType> appTypes;

    /**
     * 控制器
     */
    private String node;

    /**
     * 是否开启告警
     */
    private boolean alarmEnable;

    /**
     * 阈值
     */
    private Integer alarmThreshold;

    /**
     * 保留策略
     */
    private TapeRetentionType retentionType;

    /**
     * 保留时间
     */
    private Integer retentionDuration;

    /**
     * 保留时间单位
     */
    private TapeTimeUnit retentionUnit;

    /**
     * 介质集选择的磁带
     */
    private List<TapeVo> tapes;

    /**
     * 可用磁带数
     */
    private Integer availableTapeCount;

    /**
     * 磁带总数
     */
    private Integer totalTapeCount;

    // 块大小
    private Integer blockSize;

    // 压缩状态
    private TapeDriveCompressionStatus compressionStatus;

    // 介质集所在集群的esn
    private String esn;
}
