package openbackup.tdsql.resources.access.dto.instance;

/**
 * 功能描述
 *
 * @author z00445440
 * @since 2023-11-16
 */

import nl.jqno.equalsverifier.EqualsVerifier;
import openbackup.tdsql.resources.access.dto.instance.TdsqlGroup;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述 测试TdsqlGroup类
 *
 * @author z00445440
 * @since 2023-11-16
 */
public class TdsqlGroupTest {
    @Test
    public void test_tdsql_group() {
        EqualsVerifier.simple().forClass(TdsqlGroup.class).verify();
        EqualsVerifier.simple().forClass(TdsqlGroup.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
}
