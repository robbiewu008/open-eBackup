/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.agent.dto;

import openbackup.data.protection.access.provider.sdk.base.Authentication;

import lombok.Data;

import java.util.List;
import java.util.Map;

/**
 * App Env
 *
 * @author 30009433
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-19
 */
@Data
public class AppEnv {
    /**
     * 资源ID
     */
    private String uuid;

    /**
     * 资源名称
     */
    private String name;

    /**
     * 资源类型
     */
    private String type;

    /**
     * 资源子类型
     */
    private String subType;

    /**
     * X8000
     * 访问地址，ip或者域名
     */
    private String endpoint;

    /**
     * 端口
     */
    private Integer port;

    /**
     * 环境的认证信息
     */
    private Authentication auth;

    /**
     * 节点
     */
    private List<AppEnv> nodes;

    /**
     * 资源的扩展信息
     */
    private Map<String, String> extendInfo;
}