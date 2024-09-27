package openbackup.mongodb.protection.access.provider.service;

import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.mongodb.protection.access.service.impl.MongoDBBaseServiceImpl;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

/**
 * MongoDBBaseService 实现类测试类
 *
 * @author twx1009756
 * @version [DataBackup 1.5.0]
 * @since 2023-06-29
 */
public class MongoDBBaseServiceImplTest {
    private final ResourceService resourceService = Mockito.mock(ResourceService.class);

    private final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);

    private final ProtectedEnvironmentService protectedEnvironmentService = Mockito.mock(ProtectedEnvironmentService.class);

    private final MongoDBBaseServiceImpl mongoDBBaseServiceImpl = new MongoDBBaseServiceImpl(resourceService,
        agentUnifiedService, protectedEnvironmentService);

    /**
     * 用例场景：校验auth认证信息正确
     * 前置条件：无
     * 检查点：校验auth认证信息正确
     */
    @Test
    public void check_auth_key_and_auth_pwd_success() {
        String username = "aaa";
        String password = "sss";
        mongoDBBaseServiceImpl.checkKeyLength(username, password);
        Assert.assertNotNull(mongoDBBaseServiceImpl);
    }

    /**
     * 用例场景：校验auth key认证信息错误
     * 前置条件：无
     * 检查点：校验auth key认证长度超过32字符
     */
    @Test
    public void throw_lego_exception_when_auth_key_length_over_32() {
        String username = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
        String password = "sss";
        Assert.assertThrows(LegoCheckedException.class,
            () -> mongoDBBaseServiceImpl.checkKeyLength(username, password));
    }

    /**
     * 用例场景：校验auth pwd认证信息错误
     * 前置条件：无
     * 检查点：校验auth pwd认证长度超过32字符
     */
    @Test
    public void throw_lego_exception_when_auth_pwd_length_over_32() {
        String username = "sss";
        String password = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
        Assert.assertThrows(LegoCheckedException.class,
            () -> mongoDBBaseServiceImpl.checkKeyLength(username, password));
    }
}
