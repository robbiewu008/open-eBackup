package openbackup.system.base.sdk.copy.model;

import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * CopyGeneratedByEnum test
 *
 * @author jwx701567
 * @since 2021-03-12
 */
public class CopyGeneratedByEnumTest {
    @Test
    public void should_return_status_if_enum_is_exists_when_query_statu() {
        String Backup = CopyGeneratedByEnum.valueOf("BY_BACKUP").value();
        Assert.assertEquals("Backup", Backup);
    }

}
