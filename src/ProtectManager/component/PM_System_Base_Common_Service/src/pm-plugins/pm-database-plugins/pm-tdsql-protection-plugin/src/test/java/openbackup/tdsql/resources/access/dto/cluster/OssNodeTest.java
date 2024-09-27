package openbackup.tdsql.resources.access.dto.cluster;

import nl.jqno.equalsverifier.EqualsVerifier;
import openbackup.tdsql.resources.access.dto.cluster.OssNode;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-06-25
 */
public class OssNodeTest {
    /**
     * 用例场景：测试OssNode类
     * 前置条件：equals()和hashCode()方法正确
     * 检查点：校验通过
     */
    @Test
    public void test_oss_node() {
        EqualsVerifier.simple().forClass(OssNode.class).verify();
        EqualsVerifier.simple().forClass(OssNode.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
}
