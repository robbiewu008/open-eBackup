/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.controller.resp;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 功能描述: HBase、HDFS集群登录响应
 *
 * @author l00570077
 * @since 2021/08/25
 */
@Data
public class ResourceConnectResp {
    @JsonProperty("env_id")
    private String envId;
}