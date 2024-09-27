/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.common.model.storage;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 功能描述
 *
 * @author y00413474
 * @since 2020-07-01
 */
@Data
public class StorageSession {
    @JsonProperty("iBaseToken")
    private String baseToken;

    private String deviceid;

    private String cookie;

    private String roleId;

    private String accountstate;
}
