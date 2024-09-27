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

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * 查询历史容量信息
 *
 * @author z00613137
 * @since 2023-05-11
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class ClusterStorageTendencyInfo {
    private Integer clusterId;

    private String esn;

    private int peakPoint;

    private List<ClusterStorageTendencyDayInfo> existingDatas;

    private List<ClusterStorageTendencyDayInfo> forecastDatas;
}