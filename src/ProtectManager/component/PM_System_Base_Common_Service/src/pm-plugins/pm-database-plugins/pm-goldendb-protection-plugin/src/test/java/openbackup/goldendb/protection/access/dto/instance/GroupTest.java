package openbackup.goldendb.protection.access.dto.instance;

import nl.jqno.equalsverifier.EqualsVerifier;
import openbackup.goldendb.protection.access.dto.instance.Group;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述
 *
 * @author s30036254
 * @since 2023-02-16
 */
public class GroupTest {
@Test
public void testGroup() {
    EqualsVerifier.simple().forClass(Group.class).verify();
    EqualsVerifier.simple().forClass(Group.class).usingGetClass().verify();
    Assert.assertTrue(true);
}
}