package openbackup.openstack.adapter.service;

import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.system.base.common.aspect.OperationLogAspect;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import com.huawei.oceanprotect.system.base.user.entity.UserInfoEntity;
import com.huawei.oceanprotect.system.base.user.service.UserService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.stream.Collectors;

import javax.servlet.http.HttpServletRequest;

/**
 * OpenStack用户管理类
 * 云核OpenStack场景会创建一个用户名固定的数据保护管理员用户
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-23
 */
@Slf4j
@Component
public class OpenStackUserManager {
    private final UserService userService;
    private final HttpServletRequest request;

    public OpenStackUserManager(UserService userService, HttpServletRequest request) {
        this.userService = userService;
        this.request = request;
    }

    /**
     * 获取内置用户id
     *
     * @return 用户id
     */
    public String obtainUserId() {
        UserInfoEntity user = userService.getUserInfoByName(OpenstackConstant.QUOTA_USER_NAME);
        if (user == null) {
            throw new LegoCheckedException(CommonErrorCode.UNKNOWN_USER, "user not exists.");
        }
        return user.getUuid();
    }

    /**
     * 请求中添加token
     */
    public void setTokenToRequest() {
        TokenBo token = obtainToken();
        request.setAttribute(OperationLogAspect.TOKEN_BO, token);
        log.debug("Openstack set header attribute of user: {} to request success.", token.getUser().getId());
    }

    private TokenBo obtainToken() {
        String userId = obtainUserId();
        UserInnerResponse user = userService.getUserInfoByUserId(userId);
        List<TokenBo.RoleBo> roles = user.getRolesSet()
            .stream()
            .map(role -> TokenBo.RoleBo.builder().id(role.getRoleId().toString()).name(role.getRoleName()).build())
            .collect(Collectors.toList());

        TokenBo.UserBo userBo = TokenBo.UserBo.builder()
            .name(user.getUserName())
            .userType(user.getUserType())
            .id(userId)
            .roles(roles)
            .build();

        return TokenBo.builder().user(userBo).build();
    }
}
