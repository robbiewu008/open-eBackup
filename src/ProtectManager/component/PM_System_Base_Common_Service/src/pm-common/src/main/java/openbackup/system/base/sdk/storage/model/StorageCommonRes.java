/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Response
 *
 * @author p00511147
 * @since 2020-10-29
 */
@Data
public class StorageCommonRes<T> {
    @JsonProperty("data")
    private T data;

    @JsonProperty("error")
    private StorageCommonResError error;
}
