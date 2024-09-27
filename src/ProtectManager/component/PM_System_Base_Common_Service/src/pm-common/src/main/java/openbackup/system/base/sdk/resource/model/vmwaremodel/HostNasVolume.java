/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model.vmwaremodel;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * nas卷信息
 *
 * @author z30047175
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-22
 */
@Setter
@Getter
public class HostNasVolume {
    @JsonProperty("name")
    private String name;

    @JsonProperty("type")
    private String type;

    @JsonProperty("remote_host")
    private String remoteHost;

    @JsonProperty("remote_host_names")
    private List<String> remoteHostNames;

    @JsonProperty("remote_path")
    private String remotePath;
}
