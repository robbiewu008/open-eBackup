/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.Map;

/**
 * 功能描述 安全一体机资源接入请求参数
 *
 * @author s30031954
 * @since 2022-12-21
 */
@Data
public class CyberEngineResourceRequestParam {
    private boolean shouldDecrypt = false;

    private boolean shouldQueryDependency = false;

    private boolean shouldLoadEnvironment = true;

    // 查询条件是否忽略资源的拥有者
    private boolean shouldIgnoreOwner = false;

    @JsonProperty("pageNo")
    private int page = 0;

    @JsonProperty("pageSize")
    private int size = 10;

    private Map<String, Object> conditions;

    private String[] orders = new String[0];
}