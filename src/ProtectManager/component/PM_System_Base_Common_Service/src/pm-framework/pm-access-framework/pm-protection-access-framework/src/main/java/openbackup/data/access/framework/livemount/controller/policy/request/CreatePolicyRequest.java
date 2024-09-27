/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.livemount.controller.policy.request;

import openbackup.data.access.framework.livemount.common.enums.CopyDataSelection;
import openbackup.data.access.framework.livemount.common.enums.RetentionType;
import openbackup.data.access.framework.livemount.common.enums.RetentionUnit;
import openbackup.data.access.framework.livemount.common.enums.ScheduledType;
import openbackup.data.access.framework.livemount.common.enums.ScheduledUnit;

import lombok.Data;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * 创建更新策略
 *
 * @author h30003246
 * @since 2020-09-17
 */
@Data
public class CreatePolicyRequest {
    @NotNull
    @Pattern(regexp = "[a-zA-Z_\\u4e00-\\u9fa5]{1}[\\u4e00-\\u9fa5\\w-]*$")
    @Size(min = 1, max = 64)
    private String name;

    @NotNull
    private CopyDataSelection copyDataSelectionPolicy;

    @NotNull
    private RetentionType retentionPolicy;

    private Integer retentionValue;

    private RetentionUnit retentionUnit;

    @NotNull
    private ScheduledType schedulePolicy;

    private Integer scheduleInterval;

    private ScheduledUnit scheduleIntervalUnit;

    private String scheduleStartTime;
}
