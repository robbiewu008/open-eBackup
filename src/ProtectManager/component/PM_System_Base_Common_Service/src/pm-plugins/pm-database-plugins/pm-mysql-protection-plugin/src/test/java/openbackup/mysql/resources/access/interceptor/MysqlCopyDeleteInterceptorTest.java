package openbackup.mysql.resources.access.interceptor;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.alibaba.fastjson.JSONObject;
import com.google.common.collect.ImmutableMap;

import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.Arrays;
import java.util.List;
import java.util.Optional;

/**
 * MySQL副本删除
 *
 * @author fwx1022842
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022/10/27
 */
public class MysqlCopyDeleteInterceptorTest {
    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);
    private final MysqlCopyDeleteInterceptor mysqlCopyDeleteInterceptor = new MysqlCopyDeleteInterceptor(copyRestApi, resourceService);

    /**
     * 用例场景：测试适用范围
     * 前置条件：1. 不是MySQL的副本
     * 检 查 点：1. 返回false
     */
    @Test
    public void should_return_false_if_applicable_when_is_not_mysql() {
        Assert.assertFalse(mysqlCopyDeleteInterceptor.applicable("111"));
    }

    /**
     * 用例场景：测试适用范围
     * 前置条件：1. 是MySQL的副本
     * 检 查 点：1. 返回true
     */
    @Test
    public void should_return_true_if_applicable_when_is_not_mysql() {
        Assert.assertTrue(mysqlCopyDeleteInterceptor.applicable(ResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.getType()));
        Assert.assertTrue(mysqlCopyDeleteInterceptor.applicable(ResourceSubTypeEnum.MYSQL_DATABASE.getType()));
        Assert.assertTrue(mysqlCopyDeleteInterceptor.applicable(ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType()));
    }

    /**
     * 用例场景：测试获取增量副本之后应该删除的副本
     * 前置条件：1. 无
     * 检 查 点：1. 返回预期删除的副本
     */
    @Test
    @Ignore
    public void get_copies_when_this_copy_is_difference_increment_or_cumulative_increment() {
        Copy thisCopy = new Copy();
        thisCopy.setGn(1);
        thisCopy.setUuid("1");
        thisCopy.setResourceId("resourceId");
        Copy copy2 = new Copy();
        copy2.setGn(2);
        copy2.setUuid("2");
        Copy nextFullCopy = new Copy();
        nextFullCopy.setGn(3);
        nextFullCopy.setUuid("3");
        Copy copy4 = new Copy();
        copy4.setGn(4);
        copy4.setUuid("4");
        List<Copy> copies = Arrays.asList(copy2, nextFullCopy, copy4);
        ProtectedResource resource = new ProtectedResource();
        resource.setExtendInfo(ImmutableMap.of(DatabaseConstants.CLUSTER_TYPE, MysqlConstants.AP));
        PowerMockito.when(resourceService.getResourceById("resourceId")).thenReturn(Optional.of(resource));
        final List<String> copyUuids = mysqlCopyDeleteInterceptor.getCopiesCopyTypeIsCumulativeIncrement(copies, thisCopy, nextFullCopy);
        Assert.assertTrue(copyUuids.contains(copy2.getUuid()));
        Assert.assertFalse(copyUuids.contains(copy4.getUuid()));
    }

    @Test
    public void test_get_copies_when_this_copy_is_full() {
        Copy thisCopy = new Copy();
        thisCopy.setGn(1);
        thisCopy.setUuid("1");
        thisCopy.setResourceId("resourceId");
        Copy copy2 = new Copy();
        copy2.setGn(2);
        copy2.setUuid("2");
        Copy nextFullCopy = new Copy();
        nextFullCopy.setGn(3);
        nextFullCopy.setUuid("3");
        List<Copy> copies = Arrays.asList(copy2, nextFullCopy);
        ProtectedResource resource = new ProtectedResource();
        resource.setExtendInfo(ImmutableMap.of(DatabaseConstants.CLUSTER_TYPE, MysqlConstants.EAPP));
        thisCopy.setResourceProperties(JSONObject.toJSONString(resource));
        final List<String> copyUuids = mysqlCopyDeleteInterceptor.getCopiesCopyTypeIsCumulativeIncrement(copies, thisCopy, nextFullCopy);
        Assert.assertEquals(copyUuids.size(), 1);
        Assert.assertEquals(copyUuids.get(0), "1");
    }

    /**
     * 用例场景：测试是否下发Agent
     * 前置条件：1. 无
     * 检 查 点：1. 返回false
     */
    @Test
    public void should_return_false_if_test_should_supply_agent() {
        Assert.assertFalse(mysqlCopyDeleteInterceptor.shouldSupplyAgent(null, null));
    }
}