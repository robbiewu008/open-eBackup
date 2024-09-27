package openbackup.system.base.sdk.job.model;

import openbackup.system.base.sdk.job.model.JobLogLevelEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * JobLogLevelEnum test
 *
 * @author jwx701567
 * @since 2021-03-12
 */
public class JobLogLevelEnumTest {
    /**
     * get job log type enum by str
     */
    @Test
    public void get_job_status_success() {
        String INFO = JobLogLevelEnum.get("INFO").name();
        Assert.assertEquals("INFO", INFO);
    }
}
