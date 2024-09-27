package openbackup.system.base.sdk.job.model.request;

import com.fasterxml.jackson.annotation.JsonAlias;
import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.Data;

import java.util.Objects;

/**
 * Job Schedule Policy
 *
 * @author l00272247
 * @since 2021-03-10
 */
@Data
@JsonInclude(JsonInclude.Include.NON_DEFAULT)
public class JobSchedulePolicy extends JobScheduleRule {
    @JsonAlias("job_type")
    private String jobType;

    /**
     * equals method
     *
     * @param that another object
     * @return result
     */
    @Override
    public boolean equals(Object that) {
        if (this == that) {
            return true;
        }
        if (!(that instanceof JobSchedulePolicy) || getClass() != that.getClass()) {
            return false;
        }
        JobSchedulePolicy policy = (JobSchedulePolicy) that;
        return Objects.equals(jobType, policy.jobType) && Objects.equals(getScope(), policy.getScope())
            && Objects.equals(getMinorPriorities(), policy.getMinorPriorities())
            && Objects.equals(getExamine(), policy.getExamine());
    }

    /**
     * hash code
     *
     * @return hash code
     */
    @Override
    public int hashCode() {
        return Objects.hash(jobType, getScope(), getMinorPriorities(), getExamine());
    }
}
