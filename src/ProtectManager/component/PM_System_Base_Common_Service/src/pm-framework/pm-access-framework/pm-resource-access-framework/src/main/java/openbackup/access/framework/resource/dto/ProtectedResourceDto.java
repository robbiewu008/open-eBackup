/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.access.framework.resource.dto;

import lombok.Data;

import java.util.Map;

/**
 * 受保护资源DTO对象
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-10-14
 */
@Data
public class ProtectedResourceDto {
    /**
     * 资源UUID
     */
    private String uuid;

    /**
     * 资源名称
     */
    private String name;

    /**
     * 资源类型（主类）
     */
    private String type;

    /**
     * 资源子类
     */
    private String subType;

    /**
     * 资源路径
     */
    private String path;

    /**
     * 创建时间
     */
    private String createdTime;

    /**
     * 父资源名称
     */
    private String parentName;

    /**
     * 父资源uuid
     */
    private String parentUuid;

    /**
     * 受保护环境uuid
     */
    private String rootUuid;

    /**
     * 受保护状态
     */
    private Integer protectionStatus;

    /**
     * 资源的来源
     */
    private String sourceType;

    /**
     * 资源所属的用户
     */
    private String userId;

    /**
     * 资源授权的用户名称
     */
    private String authorizedUser;

    /**
     * 资源的扩展属性
     */
    private Map<String, String> extendInfo;
}
