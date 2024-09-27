/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.alarm.model;

import lombok.Data;
import lombok.EqualsAndHashCode;

import org.hibernate.validator.constraints.Range;

/**
 * Lego告警
 *
 * @author swx1010572
 * @version [Lego V100R002C10, 2014-12-20]
 * @since 2021-04-22
 */
@Data
public class InternalAlarm {
    private Long sequence;

    private String alarmSource;

    private String sourceType;

    private String alarmId;

    private String name;

    private String desc;

    // 告警参数，多个用“,”分隔
    private String params;

    @Range(min = 1, max = 6, message = "out of range")
    private Integer alarmType;

    private String advice;

    private String effect;

    private String location;

    @Range(min = 1, max = 4, message = "out of range")
    private Integer severity;

    @EqualsAndHashCode.Exclude
    private Long createTime;

    @Range(min = 0, max = 6, message = "out of range")
    private Integer type = 1;

    private String userId;

    private String resourceId;
}
