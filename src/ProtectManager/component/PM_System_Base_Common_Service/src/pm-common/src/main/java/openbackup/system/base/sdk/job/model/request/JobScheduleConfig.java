package openbackup.system.base.sdk.job.model.request;

import com.fasterxml.jackson.annotation.JsonAlias;

import lombok.Data;

import java.util.List;

/**
 * Job Schedule Config
 *
 * @author l00272247
 * @since 2021-03-12
 */
@Data
public class JobScheduleConfig {
    @JsonAlias("job_type")
    private String jobType;

    private List<JobScheduleRule> rules;

    /**
     * create job schedule config
     *
     * @param jobType job type
     * @param rules rules
     * @return job schedule config
     */
    public static JobScheduleConfig create(String jobType, List<JobScheduleRule> rules) {
        JobScheduleConfig config = new JobScheduleConfig();
        config.setJobType(jobType);
        config.setRules(rules);
        return config;
    }
}
