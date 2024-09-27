/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.core.entity;

import lombok.Data;

/**
 * Protection Plan Entity
 *
 * @author l00272247
 * @since 2020-07-14
 */
@Data
public class ProtectionPlanEntity {
    // 保护计划对应的资源ID
    private String resourceId;

    // 保护计划对应的资源类型
    private String protectType;

    // 保护计划对应的SLA信息
    private String sla;

    // 保护计划对应的policy
    private String policy;

    // 保护计划名称
    private String protectionPlanName;

    // 保护计划对应的计划详情
    private String protectionPlanId;

    // 备份保护类型
    private String backupJobType;

    // 保护计划的过滤条件
    private String filter;

    // 保护计划的文件数据源
    private String dataSource;
}
