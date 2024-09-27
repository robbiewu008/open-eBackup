/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2024. All rights reserved.
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
