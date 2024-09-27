package openbackup.system.base.sdk.job.model;

import openbackup.system.base.sdk.job.model.JobLogMessageLevelEnum;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

/**
 * JobLogMessageLevelEnum test
 *
 * @author jwx701567
 * @since 2021-03-12
 */
public class JobLogMessageLevelEnumTest {
    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    /**
     * get job type enum by str
     */
    @Test
    public void should_return_jog_log_status_if_num_is_defined_when_get_value() {
        String INFO = JobLogMessageLevelEnum.getJogLogStatus(1).name();
        String WARNING = JobLogMessageLevelEnum.getJogLogStatus(2).name();
        String ERROR = JobLogMessageLevelEnum.getJogLogStatus(3).name();
        String FATAL = JobLogMessageLevelEnum.getJogLogStatus(4).name();
        Assert.assertEquals("INFO", INFO);
        Assert.assertEquals("WARNING", WARNING);
        Assert.assertEquals("ERROR", ERROR);
        Assert.assertEquals("FATAL", FATAL);
    }

    /**
     * 查询的枚举属性,不在自定义的范围，抛出异常
     */
    @Test
    public void should_throw_IllegalArgumentException_if_enum_is_not_defined_when_get_value() {
        expectedException.expect(IllegalArgumentException.class);
        JobLogMessageLevelEnum.get(100).name();
    }
}
