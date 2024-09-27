package openbackup.system.base.sdk.accesspoint.model;

import openbackup.system.base.sdk.accesspoint.model.InitializeResult;
import openbackup.system.base.sdk.accesspoint.model.InitializeResultDesc;
import openbackup.system.base.sdk.accesspoint.model.enums.InitializeResultCode;
import org.junit.Assert;
import org.junit.Test;

/**
 * InitializeResult test
 *
 * @author jwx701567
 * @since 2021-03-15
 */
public class InitializeResultTest {

    @Test
    public void init_initialize_result_success() {
        InitializeResult result = new InitializeResult();
        result.addActionResultDesc(
                new InitializeResultDesc(InitializeResultCode.SUCCESS, "Expand volume pool OK"));
        boolean ok = result.isOk();
        Assert.assertTrue(ok);

    }


    @Test
    public void should_return_false_if_code_is_not_zero_when_is_ok() {
        InitializeResult result = new InitializeResult(
                new InitializeResultDesc(
                        InitializeResultCode.ERROR_NO_NODE, "Query nodes failed or no any node"));
        boolean isOk = result.isOk();
        Assert.assertFalse(isOk);
    }

    @Test
    public void add_action_error() {
        InitializeResult result = new InitializeResult();
        InitializeResult initializeResultDesc = new InitializeResult(
                new InitializeResultDesc(InitializeResultCode.ERROR_MOUNT_FAILED, "Mount path(NFS) failed"));
        result.addActionError(initializeResultDesc);
        boolean isOk = result.isOk();
        Assert.assertFalse(isOk);
    }
}
