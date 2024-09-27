/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.livemount.controller.policy.response;

import lombok.Data;

/**
 * 更新策略Vo对象
 *
 * @author h30003246
 * @since 2020-09-17
 */
@Data
public class LiveMountPolicyVo {
    private String policyId;

    private String name;

    private String copyDataSelectionPolicy;

    private String retentionPolicy;

    private Integer retentionValue;

    private String retentionUnit;

    private String schedulePolicy;

    private Integer scheduleInterval;

    private String scheduleIntervalUnit;

    private String scheduleStartTime;

    private Integer liveMountCount;
}
