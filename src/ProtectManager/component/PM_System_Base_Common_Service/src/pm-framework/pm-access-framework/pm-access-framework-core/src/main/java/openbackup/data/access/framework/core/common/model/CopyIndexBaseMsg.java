/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.core.common.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Global Search copy index base message body
 *
 * @author l00347293
 * @since 2021-01-08
 **/
@Data
public class CopyIndexBaseMsg extends SearchBaseMsg {
    @JsonProperty("copy_id")
    private String copyId;

    @JsonProperty("path")
    private String path;
}
