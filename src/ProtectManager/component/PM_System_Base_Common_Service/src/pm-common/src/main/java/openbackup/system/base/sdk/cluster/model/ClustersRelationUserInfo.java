/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model;

import com.fasterxml.jackson.annotation.JsonIgnore;

import lombok.Data;

/**
 * 授权关系用户信息
 *
 * @author nwx1077006
 * @since 2022-02-23
 */
@Data
public class ClustersRelationUserInfo {
    /**
     * 本地用户id
     */
    private String userId;

    /**
     * 本地用户名
     */
    private String userName;

    /**
     * 被管理集群用户名
     */
    private String managedUserName;

    @JsonIgnore
    private Integer clusterId;
}
