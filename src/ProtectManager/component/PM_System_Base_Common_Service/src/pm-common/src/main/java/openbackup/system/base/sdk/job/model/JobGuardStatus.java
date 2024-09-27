package openbackup.system.base.sdk.job.model;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonCreator;

import java.util.Locale;

/**
 * Job Guard Status
 *
 * @author l00272247
 * @since 2021-03-12
 */
public enum JobGuardStatus {
    /**
     * DISCARD
     */
    DISCARD,
    /**
     * cancelled
     */
    CANCELLED,
    /**
     * pending
     */
    PENDING,
    /**
     * running
     */
    RUNNING;

    /**
     * get job guard status by name
     *
     * @param name name
     * @return status
     */
    @JsonCreator
    public static JobGuardStatus get(String name) {
        return EnumUtil.get(JobGuardStatus.class, JobGuardStatus::name, name.toUpperCase(Locale.ENGLISH));
    }
}
