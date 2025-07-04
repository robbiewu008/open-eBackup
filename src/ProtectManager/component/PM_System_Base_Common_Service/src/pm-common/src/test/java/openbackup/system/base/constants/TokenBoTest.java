/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.system.base.constants;

import openbackup.system.base.common.constants.TokenBo;

import openbackup.system.base.util.DefaultRoleHelper;
import org.junit.Assert;
import org.junit.Test;

import java.util.Arrays;
import java.util.Collections;

/**
 * TokenBo test
 *
 */
public class TokenBoTest {
    public static final String ROLE_SYS_ADMIN = "88a94c476f12a21e016f12a246e50001";
    public static final String ROLE_DP_ADMIN = "88a94c476f12a21e016f12a246e50002";
    public static final String ROLE_AUDITOR = "88a94c476f12a21e016f12a246e50003";
    public static final String ROLE_COMMON_ADMIN = "88a94c476f12a21e016f12a246e50004";
    public static final String SYSADMIN_USER_ID = "88a94c476f12a21e016f12a246e50005";
    public static final String MMDP_ADMIN_USER_ID = "88a94c476f12a21e016f12a246e50006";
    public static final String MM_AUDIT_USER_ID = "88a94c476f12a21e016f12a246e50007";
    public static final String SYSADMIN_AND_AUDITOR = "88a94c476f12a21e016f12a246e50008";
    @Test
    public void testIsAdminOrAudit()
    {
        // roleBo
        TokenBo.RoleBo roleSysAdmin = new TokenBo.RoleBo();
        roleSysAdmin.setName("Role_SYS_Admin");
        roleSysAdmin.setId(TokenBoTest.ROLE_SYS_ADMIN);

        TokenBo.RoleBo dpAdmin = new TokenBo.RoleBo();
        dpAdmin.setName("Role_DP_Admin");
        dpAdmin.setId(TokenBoTest.ROLE_DP_ADMIN);

        TokenBo.RoleBo auditor = new TokenBo.RoleBo();
        auditor.setName("Role_Auditor");
        auditor.setId(TokenBoTest.ROLE_AUDITOR);

        TokenBo.RoleBo commonUser = new TokenBo.RoleBo();
        commonUser.setName("common_user");
        commonUser.setId(TokenBoTest.ROLE_COMMON_ADMIN);

        // user
        TokenBo.UserBo sysadmin = new TokenBo.UserBo();
        sysadmin.setId(TokenBoTest.SYSADMIN_USER_ID);
        sysadmin.setName("sysadmin");
        sysadmin.setRoles(Collections.singletonList(roleSysAdmin));
        Assert.assertTrue(DefaultRoleHelper.isAdminOrAudit(sysadmin.getId()));

        TokenBo.UserBo mmdpAdmin = new TokenBo.UserBo();
        mmdpAdmin.setId(TokenBoTest.MMDP_ADMIN_USER_ID);
        mmdpAdmin.setName("mmdp_admin");
        mmdpAdmin.setRoles(Collections.singletonList(dpAdmin));
        Assert.assertFalse(DefaultRoleHelper.isAdminOrAudit(mmdpAdmin.getId()));

        TokenBo.UserBo mmAudit = new TokenBo.UserBo();
        mmAudit.setId(TokenBoTest.MM_AUDIT_USER_ID);
        mmAudit.setName("mm_audit");
        mmAudit.setRoles(Collections.singletonList(auditor));
        Assert.assertTrue(DefaultRoleHelper.isAdminOrAudit(mmAudit.getId()));

        TokenBo.UserBo sysadminAndAuditor = new TokenBo.UserBo();
        sysadminAndAuditor.setId(TokenBoTest.SYSADMIN_AND_AUDITOR);
        sysadminAndAuditor.setName("sysadmin_and_auditor");
        sysadminAndAuditor.setRoles(Arrays.asList(roleSysAdmin, dpAdmin));
        Assert.assertTrue(DefaultRoleHelper.isAdminOrAudit(sysadminAndAuditor.getId()));
    }

    @Test
    public void test_is_admin_or_audit_by_role_name_success()
    {
        Assert.assertTrue(DefaultRoleHelper.isAdminOrAuditByRoleName("Role_SYS_Admin"));
    }
}
