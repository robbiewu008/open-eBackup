package openbackup.goldendb.protection.access.dto.instance;

import nl.jqno.equalsverifier.EqualsVerifier;
import openbackup.goldendb.protection.access.dto.instance.GoldenInstance;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述
 *
 * @author s30036254
 * @since 2023-02-16
 */
public class GoldenInstanceTest {
    @Test
    public void testGoldenInstance() {
        EqualsVerifier.simple().forClass(GoldenInstance.class).verify();
        EqualsVerifier.simple().forClass(GoldenInstance.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
}