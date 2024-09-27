/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.infrastructure.model.beans;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * ????
 *
 * @author x00464136
 * @since 2023-06-09
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class ContainerInfo {
    /**
     * ????
     */
    @JsonProperty("containerReady")
    private Boolean hasContainerReady;

    /**
     * ????
     */
    @JsonProperty("containerName")
    private String containerName;
}
