package openbackup.opengauss.resources.access.interceptor;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

/**
 * OpenGaussDatabaseResourceProvide 数据库资源测试类
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-18
 */
public class OpenGaussDatabaseResourceProviderTest {
    private OpenGaussDatabaseResourceProvider openGaussDatabaseResourceProvider;

    @Before
    public void init() {
        openGaussDatabaseResourceProvider = new OpenGaussDatabaseResourceProvider();
    }

    /**
     * 用例场景：调用入库资源接口
     * 前置条件：无
     * 检查点：资源入库接口，不会对参数做任何操作
     */
    @Test
    public void before_create_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.OPENGAUSS_INSTANCE.getType());
        openGaussDatabaseResourceProvider.beforeCreate(protectedResource);
        Assert.assertEquals(ResourceSubTypeEnum.OPENGAUSS_INSTANCE.getType(), protectedResource.getSubType());
    }

    /**
     * 用例场景：调用更新资源接口
     * 前置条件：无
     * 检查点：资源更新接口，不会对参数做任何操作
     */
    @Test
    public void before_update_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.OPENGAUSS.getType());
        openGaussDatabaseResourceProvider.beforeUpdate(protectedResource);
        Assert.assertEquals(ResourceSubTypeEnum.OPENGAUSS.getType(), protectedResource.getSubType());
    }

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入资源类型
     * 检查点：是否返回true
     */
    @Test
    public void applicable_open_gauss_restore_interceptor_provider_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.OPENGAUSS.getType());
        Assert.assertTrue(openGaussDatabaseResourceProvider.applicable(protectedResource));

        ProtectedResource protectedResource1 = new ProtectedResource();
        protectedResource1.setSubType(ResourceSubTypeEnum.GAUSSDBT.getType());
        Assert.assertFalse(openGaussDatabaseResourceProvider.applicable(protectedResource1));
    }
}