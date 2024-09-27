/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.openstack.adapter.enums;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

import java.util.stream.Stream;

/**
 * 云核OpenStack备份任务调度类型
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-05
 */
public enum JobScheduleType {
    /**
     * 每隔N天在指定时间执行
     */
    DAYS("days"),
    /**
     * 每周指定某天在指定时间执行
     */
    WEEKS("weeks");

    private final String type;

    JobScheduleType(String type) {
        this.type = type;
    }

    @JsonValue
    public String getType() {
        return type;
    }

    /**
     * 根据type获取JobScheduleType枚举实例
     *
     * @param type type
     * @return JobScheduleType
     */
    @JsonCreator
    public static JobScheduleType create(String type) {
        return Stream.of(JobScheduleType.values())
                .filter(scheduleType -> scheduleType.type.equals(type))
                .findFirst()
                .orElseThrow(() -> new IllegalArgumentException("Schedule type is illegal."));
    }
}
