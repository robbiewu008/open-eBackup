/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.infrastructure.model.beans;

import lombok.Data;

import java.util.List;

/**
 * 校验浮动IP和仲裁网关请求体
 *
 * @author w00607005
 * @since 2023-05-19
 */
@Data
public class NetworkCheckRequest {
    /**
     * 浮动IP地址
     */
    private String floatIpAddress;

    /**
     * 仲裁网关
     */
    private List<String> gatewayIpList;
}
