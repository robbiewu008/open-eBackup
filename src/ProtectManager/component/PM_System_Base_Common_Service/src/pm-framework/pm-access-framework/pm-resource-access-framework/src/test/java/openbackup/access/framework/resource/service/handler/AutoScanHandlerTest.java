package openbackup.access.framework.resource.service.handler;

import openbackup.access.framework.resource.service.handler.AutoScanHandler;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.util.MessageTemplate;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

/**
 * auto scan test
 *
 * @author h30027154
 * @since 2022-06-23
 */
public class AutoScanHandlerTest {
    /**
     * 用例名称：验证auto-scan触发环境扫描。<br/>
     * 前置条件：环境信息入库。<br/>
     * check点：能成功运行到环境扫描接口。
     */
    @Test
    public void test_handle_auto_scan() {
        ResourceService resourceService = Mockito.mock(ResourceService.class);
        ProtectedEnvironmentService protectedEnvironmentService = Mockito.mock(ProtectedEnvironmentService.class);
        SessionService sessionService = Mockito.mock(SessionService.class);
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setId("userId");
        Mockito.when(sessionService.getCurrentUser()).thenReturn(userBo);
        MessageTemplate messageTemplate = Mockito.mock(MessageTemplate.class);
        AutoScanHandler autoScanHandler = new AutoScanHandler(resourceService, protectedEnvironmentService,
            sessionService, messageTemplate);
        ProtectedResource protectedResource = new ProtectedEnvironment();
        protectedResource.setUuid("uuid");
        autoScanHandler.handle(true, protectedResource);
        Mockito.verify(messageTemplate, Mockito.times(1))
            .send(Mockito.anyString(),Mockito.any(JSONObject.class));
    }

    /**
     * 用例名称：验证auto-scan触发环境扫描。<br/>
     * 前置条件：环境信息入库。<br/>
     * check点：查询不到用户时抛出异常。
     */
    @Test
    public void exception_when_user_not_find() {
        ResourceService resourceService = Mockito.mock(ResourceService.class);
        ProtectedEnvironmentService protectedEnvironmentService = Mockito.mock(ProtectedEnvironmentService.class);
        SessionService sessionService = Mockito.mock(SessionService.class);
        Mockito.when(sessionService.getCurrentUser()).thenReturn(null);
        AutoScanHandler autoScanHandler = new AutoScanHandler(resourceService, protectedEnvironmentService,
            sessionService, Mockito.mock(MessageTemplate.class));
        ProtectedResource protectedResource = new ProtectedEnvironment();
        protectedResource.setUuid("uuid");
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> autoScanHandler.handle(true, protectedResource));
        Assert.assertEquals(legoCheckedException.getMessage(), "do not find userId");
    }
}
