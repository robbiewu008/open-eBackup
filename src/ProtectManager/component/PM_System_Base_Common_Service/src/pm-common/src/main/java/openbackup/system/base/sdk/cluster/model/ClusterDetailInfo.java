/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model;

import lombok.Data;

import java.util.List;

/**
 * Local cluster detail info
 *
 * @author p30001902
 * @since 2020-08-26
 */
@Data
public class ClusterDetailInfo {
    private SourceClustersParams sourceClusters;

    private List<DataProtectionParams> dataProtectionEngines;

    private StorageSystemInfo storageSystem;

    private List<ClusterDetailInfo> allMemberClustersDetail;

    private TargetClusterVo targetClusterVo;

    private Integer backupStorageUnitId;
}
