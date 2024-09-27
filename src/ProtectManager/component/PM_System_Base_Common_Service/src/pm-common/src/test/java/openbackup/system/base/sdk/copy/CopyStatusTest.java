package openbackup.system.base.sdk.copy;

import openbackup.system.base.sdk.copy.model.CopyStatus;
import org.junit.Assert;
import org.junit.Test;

/**
 * CopyStatus test
 *
 * @author jwx701567
 * @since 2021-03-16
 */
public class CopyStatusTest {

    @Test
    public void get_copy_status_success() {
        CopyStatus normal = CopyStatus.get("Normal");
        Assert.assertEquals(CopyStatus.NORMAL, normal);
    }

}
