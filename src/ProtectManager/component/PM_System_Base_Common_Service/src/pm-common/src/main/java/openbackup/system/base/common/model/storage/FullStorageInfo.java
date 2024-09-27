/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.common.model.storage;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import lombok.EqualsAndHashCode;

/**
 * 存储库全部信息
 *
 * @author w00504341
 * @since 2020-12-18
 */
@Data
@EqualsAndHashCode(callSuper = true)
public class FullStorageInfo extends CompleteStorageInfoRes {
    private String id;

    private int port;

    private String sk;

    @JsonProperty("ProxyUrl")
    private String proxyUrl;

    private String proxyPort;

    private String proxyUserPwd;
}
