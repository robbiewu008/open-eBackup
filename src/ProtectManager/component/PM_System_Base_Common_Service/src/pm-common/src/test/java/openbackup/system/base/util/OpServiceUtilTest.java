package openbackup.system.base.util;

import openbackup.system.base.util.OpServiceUtil;

import org.junit.Assert;
import org.junit.Test;

/**
 * OpServiceUtilTest
 *
 * @author l30044826
 * @since 2023-08-29
 */
public class OpServiceUtilTest {
    @Test
    public void test_is_hcs_service_false(){
        Assert.assertFalse(OpServiceUtil.isHcsService());
    }
}
