package openbackup.system.base.common.constants;

import lombok.Data;

/**
 * dorado告警对象
 *
 * @author y30000858
 * @since 2021-01-15
 */
@Data
public class DoradoInternalAlarm extends LegoInternalAlarm {
    /**
     * 清除时间
     */
    private long clearTime;

    /**
     * 恢复时间
     */
    private long recoverTime;

    private String name;

    private String suggestion;

    private String description;

    private String detail;

    private String alarmObjType;

    private String location;
}
