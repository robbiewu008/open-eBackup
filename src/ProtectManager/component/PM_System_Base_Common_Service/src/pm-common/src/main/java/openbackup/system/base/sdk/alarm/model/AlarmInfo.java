package openbackup.system.base.sdk.alarm.model;

import openbackup.system.base.common.constants.FaultEnum;

import lombok.Data;

import java.util.List;

/**
 * 告警信息
 *
 * @author w00607005
 * @since 2023-07-22
 */
@Data
public class AlarmInfo {
    private String alarmId;

    private FaultEnum.AlarmSeverity severity;

    private String name;

    private String description;

    private String detail;

    private String suggestion;

    private String objType;

    private long alarmTime;

    private Long recoverTime;

    private String location;

    private List<String> params;

    private Integer status;

    private Integer type;

    private Long sequence;

    private String entity;

    private String esn;

    private String nodeName;
}
