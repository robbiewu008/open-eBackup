/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.core.common.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Global Search index delete message body
 *
 * @author l00347293
 * @since 2021-01-08
 **/
@Data
public class CopyIndexDeleteMsg extends CopyIndexBaseMsg {
    @JsonProperty("chain_id")
    private String chainId;

    @JsonProperty("prev_copy_gn")
    private int prevCopyGn;

    @JsonProperty("next_copy_gn")
    private int nextCopyGn;
}
