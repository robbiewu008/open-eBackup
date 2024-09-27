/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource;

import lombok.Data;

/**
 * 子资源查询过滤条件对象组合
 *
 * @author swx1010572
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-30
 */
@Data
public class BrowseEnvironmentResourceConditions {
    // 受保护环境ID
    private String envId;

    // 代理id
    private String agentId;

    // 父资源ID
    private String parentId;

    // 资源类型
    private String resourceType;

    // 资源子类型
    private String resourceSubType;

    // 起始页
    private int pageNo;

    // 每页大小
    private int pageSize;

    // 模糊匹配参数
    private String conditions;
}
