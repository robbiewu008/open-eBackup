package openbackup.system.base.common.utils;

import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.enums.UserTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UserUtils;

import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Test;

import java.util.Optional;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-09-25
 */

public class UserUtilsTest {
    /**
     * 设备管理员不校验数据库信息
     */
    @Test
    public void test_check_token() {
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setId("testUUID");
        TokenBo.RoleBo roleBo = new TokenBo.RoleBo();
        roleBo.setId("4");
        roleBo.setName("1");
        userBo.setRoles(Lists.newArrayList(roleBo));
        UserUtils.checkToken(userBo, null);
    }

    /**
     * LDAPGROUP不校验用户名必须相等
     */
    @Test
    public void test_check_ldap_group_token() {
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setId("testUUID");
        userBo.setUserType(UserTypeEnum.LDAPGROUP.getValue());
        TokenBo.RoleBo roleBo = new TokenBo.RoleBo();
        roleBo.setId("1");
        roleBo.setName("1");
        userBo.setRoles(Lists.newArrayList(roleBo));
        TokenBo.UserInfo userInfo = new TokenBo.UserInfo();
        userBo.setId("testUUID");
        userInfo.setUserType(UserTypeEnum.LDAPGROUP.getValue());
        UserUtils.checkToken(userBo, Optional.of(userInfo));
    }

    /**
     * COMMON用户校验密码版本，username和usertype 必须匹配
     */
    @Test
    public void test_check_common_token() {
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setId("testUUID");
        userBo.setUserType(UserTypeEnum.COMMON.getValue());
        userBo.setPasswordVersion(1L);
        userBo.setName("TestUserName");
        TokenBo.RoleBo roleBo = new TokenBo.RoleBo();
        roleBo.setId("1");
        roleBo.setName("1");
        userBo.setRoles(Lists.newArrayList(roleBo));
        TokenBo.UserInfo userInfo = new TokenBo.UserInfo();
        userInfo.setName("TestUserName");
        userInfo.setUserType(UserTypeEnum.COMMON.getValue());
        userInfo.setPasswordVersion(1L);
        UserUtils.checkToken(userBo, Optional.of(userInfo));
    }

    /**
     * COMMON用户校验密码版本，username和usertype 必须匹配
     */
    @Test
    public void test_check_common_token_fail_when_username_not_equals() {
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setId("testUUID");
        userBo.setUserType(UserTypeEnum.COMMON.getValue());
        userBo.setPasswordVersion(1L);
        userBo.setName("TestUserName1");
        TokenBo.RoleBo roleBo = new TokenBo.RoleBo();
        roleBo.setId("1");
        roleBo.setName("1");
        userBo.setRoles(Lists.newArrayList(roleBo));
        TokenBo.UserInfo userInfo = new TokenBo.UserInfo();
        userInfo.setName("TestUserName2");
        userInfo.setUserType(UserTypeEnum.COMMON.getValue());
        userInfo.setPasswordVersion(1L);
        Assert.assertThrows(LegoCheckedException.class, () -> {
            UserUtils.checkToken(userBo, Optional.of(userInfo));
        });
    }
}
