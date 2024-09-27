package openbackup.system.base.sdk.job.model;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

/**
 * 功能描述
 *
 * @author h30003246
 * @since 2021-01-12
 */
public enum JobLogMessageLevelEnum {
    /**
     * INFO
     */
    INFO(1),

    /**
     * WARNING
     */
    WARNING(2),

    /**
     * ERROR
     */
    ERROR(3),

    /**
     * fatal
     */
    FATAL(4);

    private final int type;

    JobLogMessageLevelEnum(int value) {
        this.type = value;
    }

    /**
     * get job type enum by str
     *
     * @param str str
     * @return job type enum
     */
    @JsonCreator
    public static JobLogMessageLevelEnum get(int str) {
        return EnumUtil.get(JobLogMessageLevelEnum.class, JobLogMessageLevelEnum::getValue, str, false);
    }

    /**
     * get json value
     *
     * @return json value
     */
    @JsonValue
    public int getValue() {
        return type;
    }

    /**
     * convert anyBackup status to self job status
     *
     * @param instStatus anyBackup status
     * @return job status
     */
    public static JobLogLevelEnum getJogLogStatus(int instStatus) {
        JobLogMessageLevelEnum state = JobLogMessageLevelEnum.get(instStatus);
        switch (state) {
            case WARNING:
                return JobLogLevelEnum.WARNING;
            case ERROR:
                return JobLogLevelEnum.ERROR;
            case FATAL:
                return JobLogLevelEnum.FATAL;
            default:
                return JobLogLevelEnum.INFO;
        }
    }
}
