package openbackup.system.base.common.system;

import openbackup.system.base.common.system.JobScheduleControlRegistrar;
import openbackup.system.base.sdk.job.api.JobCenterNativeApi;
import openbackup.system.base.sdk.job.model.request.JobScheduleRule;
import org.junit.Test;
import org.mockito.Mockito;
import org.springframework.core.env.ConfigurableEnvironment;
import org.springframework.test.util.ReflectionTestUtils;

import static org.assertj.core.api.Assertions.assertThat;

/**
 * {@link JobScheduleControlRegistrar} 测试类
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-02
 */
public class JobScheduleControlRegistrarTest {
    private final ConfigurableEnvironment environment = Mockito.mock(ConfigurableEnvironment.class);
    private final JobCenterNativeApi jobCenterNativeApi = Mockito.mock(JobCenterNativeApi.class);
    private final JobScheduleControlRegistrar registrar = new JobScheduleControlRegistrar(environment, jobCenterNativeApi);

    /**
     * 用例名称：如果任务排队限流配置项没有globalJobLimit，则返回为空。<br/>
     * 前置条件：无。<br/>
     * check点：不存在globalJobLimit，则rule中也不应有对应值。
     */
    @Test
    public void test_should_return_empty_when_build_global_job_limit_if_job_limit_empty() {
        JobScheduleRule rule = new JobScheduleRule();
        ReflectionTestUtils.invokeMethod(registrar, "buildGlobalJobLimit", rule, null);
        assertThat(rule.getGlobalJobLimit()).isNull();
    }

    /**
     * 用例名称：如果任务排队限流配置中存在globalJobLimit，则返回对应值。<br/>
     * 前置条件：无。<br/>
     * check点：存在globalJobLimit，则rule中也有对应值。
     */
    @Test
    public void test_should_return_10_when_build_global_job_limit_if_job_limit_has_int_value() {
        JobScheduleRule rule = new JobScheduleRule();
        String jobLimit = "{\"common\": 10}";
        ReflectionTestUtils.invokeMethod(registrar, "buildGlobalJobLimit", rule, jobLimit);
        assertThat(rule.getGlobalJobLimit()).containsEntry("common", 10);

        jobLimit = "{\"common\": \"20\"}";
        ReflectionTestUtils.invokeMethod(registrar, "buildGlobalJobLimit", rule, jobLimit);
        assertThat(rule.getGlobalJobLimit()).containsEntry("common", 20);
    }

    /**
     * 用例名称：如果环境变量中配置值为整数，则返回对应值。<br/>
     * 前置条件：无。<br/>
     * check点：能够获取环境变量中的值。
     */
    @Test
    public void test_should_return_10_when_build_global_job_limit_if_environment_config_has_value() {
        JobScheduleRule rule = new JobScheduleRule();
        String jobLimit = "{\"protect_agent_update\": 'PROTECT_AGENT_UPDATE_GLOBAL_JOB_LIMIT'}";
        Mockito.when(environment.getProperty("PROTECT_AGENT_UPDATE_GLOBAL_JOB_LIMIT")).thenReturn("10");
        ReflectionTestUtils.invokeMethod(registrar, "buildGlobalJobLimit", rule, jobLimit);
        assertThat(rule.getGlobalJobLimit()).containsEntry("protect_agent_update", 10);

    }

    /**
     * 用例名称：如果环境变量中配置值不为整数，则返回为空。<br/>
     * 前置条件：无。<br/>
     * check点：globalJobLimit必须为整数。
     */
    @Test
    public void test_should_return_empty_when_build_global_job_limit_if_environment_config_not_number_value() {
        JobScheduleRule rule = new JobScheduleRule();
        String jobLimit = "{\"protect_agent_update\": 'PROTECT_AGENT_UPDATE_GLOBAL_JOB_LIMIT'}";

        Mockito.when(environment.getProperty("PROTECT_AGENT_UPDATE_GLOBAL_JOB_LIMIT")).thenReturn("test");
        ReflectionTestUtils.invokeMethod(registrar, "buildGlobalJobLimit", rule, jobLimit);
        assertThat(rule.getGlobalJobLimit()).isNull();
    }
}
