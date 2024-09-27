package openbackup.system.base.sdk.job.model;

import openbackup.system.base.sdk.job.model.JobTypeEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * JobTypeEnum test
 *
 * @author jwx701567
 * @since 2021-03-12
 */
public class JobTypeEnumTest {
    /**
     * get job log type enum by str
     */
    @Test
    public void get_job_status_success() {
        String BACKUP = JobTypeEnum.get("BACKUP").name();
        Assert.assertEquals("BACKUP", BACKUP);
    }
}
