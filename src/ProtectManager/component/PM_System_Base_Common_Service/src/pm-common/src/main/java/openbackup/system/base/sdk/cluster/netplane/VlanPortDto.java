/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.netplane;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * VlanPortDto
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-04-11
 */
@Data
public class VlanPortDto {
    @JsonProperty("port_type")
    String portType;

    @JsonProperty("port_list")
    List<String> portList;

    String tag;
}
