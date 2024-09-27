package openbackup.goldendb.protection.access.dto.instance;

import nl.jqno.equalsverifier.EqualsVerifier;
import openbackup.goldendb.protection.access.dto.instance.Gtm;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述
 *
 * @author s30036254
 * @since 2023-02-16
 */
public class GtmTest {
@Test
public void testGtm() {
    EqualsVerifier.simple().forClass(Gtm.class).verify();
    EqualsVerifier.simple().forClass(Gtm.class).usingGetClass().verify();
    Assert.assertTrue(true);
}
}