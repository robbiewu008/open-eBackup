/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Dme Tape Request
 *
 * @author l00272247
 * @since 2022-02-24
 */
@Data
public class DmeTapeRequest {
    @JsonProperty("TapeSetId")
    String tapeSetId;
}
