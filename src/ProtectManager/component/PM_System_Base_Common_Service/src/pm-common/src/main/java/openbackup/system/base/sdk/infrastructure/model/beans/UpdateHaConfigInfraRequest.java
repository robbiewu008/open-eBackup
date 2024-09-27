/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.infrastructure.model.beans;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * 修改HA配置请求
 *
 * @author c30047317
 * @since 2023-05-17
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class UpdateHaConfigInfraRequest {
    /**
     * 浮动IP
     */
    @JsonProperty("floatIpAddress")
    private String floatIpAddress;

    /**
     * 仲裁网关
     */
    @JsonProperty("gatewayIpList")
    private List<String> gatewayIpList;

    /**
     * 节点角色:
     * primary: 主节点, standby: 从节点
     */
    @JsonProperty("role")
    private String role;
}
