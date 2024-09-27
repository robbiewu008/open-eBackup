/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 */

package openbackup.data.access.client.sdk.api.framework.dme.replicate.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * 删除副本参数
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-10-07
 */
@Setter
@Getter
public class RemoveCopyRequest {
    @JsonProperty("resourceID")
    private String resourceId;
    private String copyId;
    private String targetEsn;
    private String jobType;

    // 0: backup,1: replication
    private int copyType;
}
