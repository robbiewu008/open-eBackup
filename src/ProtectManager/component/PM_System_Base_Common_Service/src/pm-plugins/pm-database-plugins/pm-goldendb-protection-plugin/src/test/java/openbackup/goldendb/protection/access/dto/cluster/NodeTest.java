package openbackup.goldendb.protection.access.dto.cluster;

import nl.jqno.equalsverifier.EqualsVerifier;
import openbackup.goldendb.protection.access.dto.cluster.Node;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述
 *
 * @author s30036254
 * @since 2023-02-16
 */
public class NodeTest {
@Test
public void testNode() {
    EqualsVerifier.simple().forClass(Node.class).verify();
    EqualsVerifier.simple().forClass(Node.class).usingGetClass().verify();
    Assert.assertTrue(true);
}
}