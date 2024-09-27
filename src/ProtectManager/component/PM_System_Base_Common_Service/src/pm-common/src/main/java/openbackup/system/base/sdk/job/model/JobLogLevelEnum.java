package openbackup.system.base.sdk.job.model;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;
import com.google.common.collect.ImmutableList;

import java.util.List;

/**
 * 功能描述
 *
 * @author h30003246
 * @since 2021-01-12
 */
public enum JobLogLevelEnum {
    /**
     * INFO
     */
    INFO("info"),

    /**
     * WARNING
     */
    WARNING("warning"),

    /**
     * ERROR
     */
    ERROR("error"),

    /**
     * fatal
     */
    FATAL("fatal");

    private final String type;

    /**
     * 需要展示任务事件的事件级别
     */
    public static final List<String> needShowLevels = ImmutableList.of(JobLogLevelEnum.WARNING.getValue(),
            JobLogLevelEnum.ERROR.getValue(), JobLogLevelEnum.FATAL.getValue());

    JobLogLevelEnum(String value) {
        this.type = value;
    }

    /**
     * get job type enum by str
     *
     * @param str str
     * @return job type enum
     */
    @JsonCreator
    public static JobLogLevelEnum get(String str) {
        return EnumUtil.get(JobLogLevelEnum.class, JobLogLevelEnum::getValue, str, false);
    }

    /**
     * get json value
     *
     * @return json value
     */
    @JsonValue
    public String getValue() {
        return type;
    }
}
