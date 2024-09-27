package openbackup.system.base.common.aspect;

import openbackup.system.base.common.aspect.DomainBasedOwnershipVerifier;
import openbackup.system.base.common.aspect.RightsControlInterceptor;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.RightsControl;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;

import java.lang.reflect.Method;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Optional;

/**
 * 功能描述
 *
 * @author y30021475
 * @since 2023-09-27
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({TokenBo.class, RequestContextHolder.class})
public class RightsControlInterceptorTest {
    @InjectMocks
    private RightsControlInterceptor rightsControlInterceptor;

    @Mock
    private List<DomainBasedOwnershipVerifier> verifiers;

    @Mock
    private AuthNativeApi authNativeApi;

    @Test
    public void should_throw_ACCESS_DENIED_if_post_request_and_hcs_read_only_when_access_denied() {
        // Given
        TokenBo.UserBo userBo = TokenBo.UserBo.builder()
            .id("0")
            .name("username")
            .roles(
                Collections.singletonList(TokenBo.RoleBo.builder().name(Constants.Builtin.ROLE_DP_ADMIN).build()))
            .passwordVersion(0L).userType("HCS").isHcsUserManagePermission(false)
            .build();
        TokenBo tokenBo = TokenBo.builder().user(userBo).build();
        TokenBo.UserInfo userInfo = new TokenBo.UserInfo();
        userInfo.setRoles(userBo.getRoles());
        userInfo.setUserType(tokenBo.getUser().getUserType());
        userInfo.setName(userBo.getName());
        PowerMockito.when(authNativeApi.queryUserInfoById(Mockito.any())).thenReturn(Optional.of(userInfo));
        // when
        MockHttpServletRequest request = new MockHttpServletRequest();
        request.setMethod("POST");
        PowerMockito.mockStatic(RequestContextHolder.class);
        PowerMockito.when(RequestContextHolder.getRequestAttributes()).thenReturn(new ServletRequestAttributes(request));
        RightsControl rightsControl = PowerMockito.mock(RightsControl.class);
        String[] args = new String[1];
        args[0] = userBo.getRoles().get(0).getName();
        PowerMockito.when(rightsControl.roles()).thenReturn(args);
        // Then
        Assert.assertThrows(LegoCheckedException.class,
            () -> rightsControlInterceptor.intercept(PowerMockito.mock(Method.class), rightsControl, new HashMap<>(), tokenBo));
    }
}
