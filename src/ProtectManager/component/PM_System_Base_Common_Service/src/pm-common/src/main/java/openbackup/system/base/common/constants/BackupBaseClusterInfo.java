/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.common.constants;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 集群信息
 *
 * @author z00666391
 * @since 2023-06-06
 */
@Setter
@Getter
public class BackupBaseClusterInfo {
    private List<String> clusterIp;

    private String esn;

    private String roleType;

    private boolean isClusterEstablished;
}
