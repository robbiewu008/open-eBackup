package openbackup.system.base.sdk.resource.enums;

import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * LinkStatusEnum test
 *
 * @author jwx701567
 * @since 2021-03-17
 */
public class LinkStatusEnumTest {


    @Test
    public void getByStatus() {
        String ONLINE = LinkStatusEnum.getByStatus(1).name();
        Assert.assertEquals("ONLINE", ONLINE);
    }

}
