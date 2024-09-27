package openbackup.system.base.util;

import static org.powermock.api.mockito.PowerMockito.mock;

import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.sdk.user.model.RolePo;
import openbackup.system.base.sdk.user.RoleServiceApi;
import openbackup.system.base.util.DefaultRoleHelper;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * 默认角色工具类单元测试
 *
 * @author z00842230
 * @version [OceanProtect DataBackup 1.7.0]
 * @since 2024-07-04
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {RoleServiceApi.class, DefaultRoleHelper.class})
public class DefaultRoleHelperTest {
    @Test
    public void test_is_admin_or_audit_true_when_sys_admin() {
        RoleServiceApi roleServiceApi = mock(RoleServiceApi.class);
        PowerMockito.when(roleServiceApi.getDefaultRolePoByUserId("123"))
            .thenReturn(prepareRolePo(Constants.Builtin.ROLE_SYS_ADMIN));
        DefaultRoleHelper helper = new DefaultRoleHelper(roleServiceApi);
        Assert.assertTrue(DefaultRoleHelper.isAdminOrAudit("123"));
    }

    @Test
    public void test_is_audit_true_when_audit_user() {
        RoleServiceApi roleServiceApi = mock(RoleServiceApi.class);
        PowerMockito.when(roleServiceApi.getDefaultRolePoByUserId("456"))
            .thenReturn(prepareRolePo(Constants.Builtin.ROLE_AUDITOR));
        DefaultRoleHelper helper = new DefaultRoleHelper(roleServiceApi);
        Assert.assertTrue(DefaultRoleHelper.isAudit("456"));
    }

    @Test
    public void test_is_admin_true_when_sys_admin_user() {
        RoleServiceApi roleServiceApi = mock(RoleServiceApi.class);
        PowerMockito.when(roleServiceApi.getDefaultRolePoByUserId("789"))
            .thenReturn(prepareRolePo(Constants.Builtin.ROLE_SYS_ADMIN));
        DefaultRoleHelper helper = new DefaultRoleHelper(roleServiceApi);
        Assert.assertTrue(DefaultRoleHelper.isAdmin("789"));
    }

    @Test
    public void test_is_dp_true_when_dp_user() {
        RoleServiceApi roleServiceApi = mock(RoleServiceApi.class);
        PowerMockito.when(roleServiceApi.getDefaultRolePoByUserId("abc"))
            .thenReturn(prepareRolePo(Constants.Builtin.ROLE_DP_ADMIN));
        DefaultRoleHelper helper = new DefaultRoleHelper(roleServiceApi);
        Assert.assertTrue(DefaultRoleHelper.isDp("abc"));
    }

    @Test
    public void test_is_device_manager_true_when_device_manager_user() {
        RoleServiceApi roleServiceApi = mock(RoleServiceApi.class);
        PowerMockito.when(roleServiceApi.getDefaultRolePoByUserId("def"))
            .thenReturn(prepareRolePo(Constants.Builtin.ROLE_DEVICE_MANAGER));
        DefaultRoleHelper helper = new DefaultRoleHelper(roleServiceApi);
        Assert.assertTrue(DefaultRoleHelper.isDeviceManager("def"));
    }

    private RolePo prepareRolePo(String roleName) {
        RolePo rolePo = new RolePo();
        rolePo.setRoleName(roleName);
        return rolePo;
    }
}
