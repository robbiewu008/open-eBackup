/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.livemount.bo;

import openbackup.data.access.framework.livemount.common.enums.CopyDataSelection;
import openbackup.data.access.framework.livemount.common.enums.RetentionType;
import openbackup.data.access.framework.livemount.common.enums.RetentionUnit;
import openbackup.data.access.framework.livemount.common.enums.ScheduledType;
import openbackup.data.access.framework.livemount.common.enums.ScheduledUnit;

import lombok.Data;

/**
 * 更新策略业务对象
 *
 * @author h30003246
 * @since 2020-09-17
 */
@Data
public class PolicyBo {
    private String name;

    private CopyDataSelection copyDataSelectionPolicy;

    private RetentionType retentionPolicy;

    private Integer retentionValue;

    private RetentionUnit retentionUnit;

    private ScheduledType schedulePolicy;

    private Integer scheduleInterval;

    private ScheduledUnit scheduleIntervalUnit;

    private String scheduleStartTime;
}
