/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.anti.model;

import lombok.Data;

import java.util.ArrayList;
import java.util.List;

/**
 * 防勒索策略返回对象
 *
 * @author nwx1077006
 * @since 2021-11-16
 */
@Data
public class AntiRansomwarePolicyRes {
    // 策略id
    private Integer id;

    // 策略名称
    private String policyName;

    // 策略描述
    private String description;

    // 集群id
    private Integer clusterId;

    // 数据源类型
    private String dataSourceType;

    // 资源子类型
    private String resourceSubType;

    // 资源列表
    private List<AntiRansomwarePolicyResourceRes> selectedResources = new ArrayList<>();

    // 调度计划
    private AntiRansomwareScheduleRes schedule;

    // 角色id
    private String roleId;

    // 租户数量
    private Long vStoreCount;
}