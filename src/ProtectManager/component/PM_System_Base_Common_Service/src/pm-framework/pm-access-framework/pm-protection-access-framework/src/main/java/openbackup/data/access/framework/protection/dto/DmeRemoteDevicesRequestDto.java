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
package openbackup.data.access.framework.protection.dto;

import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;

import lombok.Builder;
import lombok.Data;

import java.util.List;
import java.util.Map;

/**
 * 获取DME远端设备信息请求体
 *
 * @author l00853347
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-06-25
 */
@Data
@Builder
public class DmeRemoteDevicesRequestDto {
    private List<ClusterDetailInfo> allMemberClustersDetail;
    private String storageId;
    private Map<String, String> netPlaneMap;
    private Map<String, List<String>> mgrIpMap;
    private String token;
    private String esn;
    private String resourceId;
}
