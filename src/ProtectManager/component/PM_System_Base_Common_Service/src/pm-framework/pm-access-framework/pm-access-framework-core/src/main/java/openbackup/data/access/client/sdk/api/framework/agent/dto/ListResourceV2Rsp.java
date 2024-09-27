/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.agent.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * PM向Agent查询应用详细信息(V2接口)后的返回对象
 *
 * @author 30009433
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-19
 */
@Setter
@Getter
public class ListResourceV2Rsp extends AgentBaseDto {
    @JsonProperty("resourceList")
    private ResourceListDto resourceListDto;
}
