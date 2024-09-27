/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.infrastructure.model.beans;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * 写入到configmap中的初始化网络信息
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023/12/21
 */

@Getter
@Setter
public class InitNetWorkParam {
    @JsonProperty("backup_net_plane")
    private String backupNetPlane;
    @JsonProperty("replication_net_plane")
    private String replicationNetPlane;
    @JsonProperty("archive_net_plane")
    private String archiveNetPlane;
}
