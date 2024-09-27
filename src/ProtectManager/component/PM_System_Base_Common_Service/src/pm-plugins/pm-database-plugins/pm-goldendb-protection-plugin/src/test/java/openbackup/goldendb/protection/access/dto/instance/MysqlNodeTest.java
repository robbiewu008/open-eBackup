package openbackup.goldendb.protection.access.dto.instance;

import nl.jqno.equalsverifier.EqualsVerifier;
import openbackup.goldendb.protection.access.dto.instance.MysqlNode;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述
 *
 * @author s30036254
 * @since 2023-02-16
 */
public class MysqlNodeTest {
    @Test
    public void testMysqlNode() {
        EqualsVerifier.simple().forClass(MysqlNode.class).verify();
        EqualsVerifier.simple().forClass(MysqlNode.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
}