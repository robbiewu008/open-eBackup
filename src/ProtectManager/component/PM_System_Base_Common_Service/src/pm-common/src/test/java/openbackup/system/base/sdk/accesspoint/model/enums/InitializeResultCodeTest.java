package openbackup.system.base.sdk.accesspoint.model.enums;

import openbackup.system.base.sdk.accesspoint.model.enums.InitializeResultCode;

import org.junit.Assert;
import org.junit.Test;

/**
 * InitializeResultCode test
 *
 * @author jwx701567
 * @since 2021-03-15
 */
public class InitializeResultCodeTest {

    @Test
    public void get_initialize_result_code_success() {
        String zero = InitializeResultCode.forValues("0").toString();
        Assert.assertEquals("0", zero);

        String SUCCESS = InitializeResultCode.forValues("0").name();
        Assert.assertEquals("SUCCESS", SUCCESS);

    }

    @Test
    public void should_return_true_if_code_is_zero_when_is_ok() {
        InitializeResultCode SUCCESS = InitializeResultCode.forValues("0");
        boolean successOk = SUCCESS.isOk();
        Assert.assertTrue(successOk);
    }

    @Test
    public void should_return_false_if_code_is_not_zero_when_is_ok() {
        InitializeResultCode ERROR_NO_NODE = InitializeResultCode.forValues("10000");
        boolean falseCode = ERROR_NO_NODE.isOk();
        Assert.assertFalse(falseCode);
    }
}
