/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.agent.dto;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.util.BeanTools;

import lombok.Getter;
import lombok.Setter;

import java.util.Map;

/**
 * App Resource
 *
 * @author 30009433
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-19
 */
@Setter
@Getter
public class AppResource {
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
     * 父资源的uuid。如备份数据库，这个是数据库实例的uuid
     */
    private String parentUuid;

    /**
     * 父资源的名称
     */
    private String parentName;

    /**
     * 资源的扩展信息
     */
    private Map<String, String> extendInfo;

    /**
     * AppResource 转换为 ProtectedResource
     *
     * @return ProtectedResource
     */
    public ProtectedResource castToProtectedResource() {
        return BeanTools.copy(this, ProtectedResource::new);
    }
}