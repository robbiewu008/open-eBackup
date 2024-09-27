package openbackup.goldendb.protection.access.provider;

import openbackup.goldendb.protection.access.provider.GoldenDBJobQueueProvider;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.junit.MockitoJUnitRunner;

/**
 * GoldenDB自定义排队规则测试类
 *
 * @author c30035089
 * @since 2024-07-24
 */
@RunWith(MockitoJUnitRunner.class)
public class GoldenDBJobQueueProviderTest {
    GoldenDBJobQueueProvider goldenDBJobQueueProvider;

    @Before
    public void init() throws IllegalAccessException {
        goldenDBJobQueueProvider = new GoldenDBJobQueueProvider();
    }

    @Test
    public void test_applicable_success() {
        Assert.assertTrue(goldenDBJobQueueProvider.applicable(buildJob()));
    }

    private Job buildJob() {
        Job job = new Job();
        job.setType(JobTypeEnum.BACKUP.getValue());
        job.setSourceSubType(ResourceSubTypeEnum.GOLDENDB_CLUSETER_INSTANCE.getType());
        return job;
    }

    @Test
    public void test_get_customized_schedulePolicy_success() {
        Job job = buildJob();
        Assert.assertEquals(goldenDBJobQueueProvider.getCustomizedSchedulePolicy(job).size(), 1);
    }
}
