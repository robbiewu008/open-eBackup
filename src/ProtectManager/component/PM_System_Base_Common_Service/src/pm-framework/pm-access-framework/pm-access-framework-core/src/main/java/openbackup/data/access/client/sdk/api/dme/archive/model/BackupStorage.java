/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 备份存储
 *
 * @author z30009433
 * @since 2020-12-31
 */
@Data
public class BackupStorage {
    @JsonProperty("Type")
    int type;

    @JsonProperty("IP")
    String ip;

    @JsonProperty("Port")
    String port;

    @JsonProperty("Username")
    String username;

    @JsonProperty("Password")
    String password;

    @JsonProperty("Certificate")
    String certificate;
}
