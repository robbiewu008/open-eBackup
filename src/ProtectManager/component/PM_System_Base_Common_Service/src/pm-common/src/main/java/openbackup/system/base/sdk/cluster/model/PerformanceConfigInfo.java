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
package openbackup.system.base.sdk.cluster.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * 性能监控查询开启状态
 *
 */
@Setter
@Getter
public class PerformanceConfigInfo {
    /**
     * 性能监控开关状态
     */
    @JsonProperty("isPerformanceConfigOpen")
    private boolean isPerformanceConfigOpen;

    /**
     * 性能监控历史记录是否删除：0未启用  1已删除  2未删除
     */
    private Integer hasRemoveHistoryData;
}
