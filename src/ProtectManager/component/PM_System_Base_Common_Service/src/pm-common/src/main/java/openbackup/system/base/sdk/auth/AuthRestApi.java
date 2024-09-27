package openbackup.system.base.sdk.auth;

import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RequestParam;

import java.lang.reflect.InvocationTargetException;

/**
 * Auth Rest Api
 *
 * @author l00272247
 * @since 2020-12-06
 */
@FeignClient(name = "auth-client-rest-api", url = "${service.url.pm-system-base}/v1",
    configuration = CommonFeignConfiguration.class)
public interface AuthRestApi {
    /**
     * query user info by name
     *
     * @param username username
     * @return user info
     */
    @ExterAttack
    @GetMapping("/internal/auth/user")
    TokenBo.UserInfo queryUserInfoByName(@RequestParam("username") String username);

    /**
     * 获取用户详细信息
     *
     * @param userId 用户id
     * @param token token
     * @return 用户详情
     * @throws InvocationTargetException InvocationTargetException
     * @throws IllegalAccessException IllegalAccessException
     */
    @ExterAttack
    @GetMapping("/users/{userId}")
    UserDetail getUserDetail(@PathVariable("userId") String userId, @RequestHeader(name = "x-auth-token") String token)
        throws InvocationTargetException, IllegalAccessException;
}
