package openbackup.informix.protection.access.provider.resource;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.informix.protection.access.service.InformixService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

/**
 * {@link InformixClusterInstanceProvider Test}
 *
 * @author dwx1009286
 * @version [DataBackup 1.5.0]
 * @since 2023-05-17
 */
public class InformixClusterInstanceProviderTest {
    private static final InformixService informixService = PowerMockito.mock(InformixService.class);

    private static final InformixClusterInstanceProvider provider = new InformixClusterInstanceProvider(informixService);

    /**
     * 用例场景：applicable集群实例类型
     * 前置条件：类型为Informix-clusterInstance
     * 检查点：是否返回true
     */
    @Test
    public void applicable_informix_clusterInstance_success() {
        final ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.INFORMIX_CLUSTER_INSTANCE.getType());
        Assert.assertTrue(provider.applicable(resource));
    }

    /**
     * 用例场景：创建前置检查成功
     * 前置条件：无
     * 检查点：是否调用informixService进行参数校验与封装
     */
    @Test
    public void check_success() {
        ProtectedResource resource = new ProtectedResource();
        provider.check(resource);
        Mockito.verify(informixService, Mockito.times(1)).doClusterInstanceAction(resource, true);
    }

    /**
     * 用例场景：更新检查成功
     * 前置条件：无
     * 检查点：是否调用informixService进行参数校验与封装
     */
    @Test
    public void update_check_success() {
        ProtectedResource resource = new ProtectedResource();
        provider.updateCheck(resource);
        Mockito.verify(informixService, Mockito.times(1)).doClusterInstanceAction(resource, false);
    }
}
