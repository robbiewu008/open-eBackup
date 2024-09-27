/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.protection.common.enums;

import openbackup.data.access.framework.copy.mng.enums.CopyTypeEnum;
import openbackup.system.base.common.enums.TimeUnitEnum;

import lombok.Getter;

/**
 * 统一转换成天比较
 *
 * @author z30006621
 * @since 2021-09-07
 */
@Getter
public class SpecifiedScope {
    private final TimeUnitEnum timeUnit;

    private final int retentionDuration;

    private final int days;

    public SpecifiedScope(CopyTypeEnum copyType, int retentionDuration) {
        this.timeUnit = copyType.getTimeUnitEnum();
        this.retentionDuration = retentionDuration;
        this.days = copyType.getDays() * retentionDuration;
    }
}
