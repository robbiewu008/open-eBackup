package openbackup.tdsql.resources.access.interceptor;

import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * 功能描述
 *
 * @author z00445440
 * @since 2024-06-18
 */
@RunWith(PowerMockRunner.class)
public class TdsqlInstanceCopyDeleteInterceptorTest {
    private TdsqlInstanceCopyDeleteInterceptor copyDeleteInterceptor;

    private CopyRestApi copyRestApi;

    private ResourceService resourceService;

    @Before
    public void init() {
        copyRestApi = Mockito.mock(CopyRestApi.class);
        resourceService = Mockito.mock(ResourceService.class);
        copyDeleteInterceptor = new TdsqlInstanceCopyDeleteInterceptor(copyRestApi, resourceService);
    }

    /**
     * 用例场景：Tdsql 下发备份任务 applicable 校验
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void check_applicable_success() {
        boolean applicable = copyDeleteInterceptor.applicable(ResourceSubTypeEnum.OCEAN_BASE_CLUSTER.getType());
        Assert.assertTrue(applicable);
    }
}

