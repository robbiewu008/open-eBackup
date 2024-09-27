/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * 本地远程复制Pair
 *
 * @author g30003063
 * @since 2021/12/14
 */
@Getter
@Setter
public class StorageRemoteReplicationPairBo {
    /**
     * 远程复制PairID
     */
    @JsonProperty("ID")
    private String id;

    /**
     * 本端是否是主端
     */
    @JsonProperty("ISPRIMARY")
    private boolean isPrimary;

    /**
     * 复制模式
     * 1：同步
     * 2：异步
     */
    @JsonProperty("REPLICATIONMODEL")
    private int replicationModel;
}
