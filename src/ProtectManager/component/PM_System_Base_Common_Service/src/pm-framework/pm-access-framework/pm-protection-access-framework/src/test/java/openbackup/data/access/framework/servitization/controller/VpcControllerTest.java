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
package openbackup.data.access.framework.servitization.controller;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.servitization.controller.VpcController;
import openbackup.data.access.framework.servitization.controller.req.VpcRequest;
import openbackup.data.access.framework.servitization.entity.VpcInfoEntity;
import openbackup.data.access.framework.servitization.service.IVpcService;
import openbackup.system.base.common.aspect.OperationLogService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.RequestUtil;
import openbackup.system.base.sdk.auth.api.HcsTokenAPi;
import openbackup.system.base.sdk.auth.model.HcsToken;
import openbackup.system.base.sdk.auth.model.Role;
import openbackup.system.base.sdk.auth.model.TokenUser;
import com.huawei.oceanprotect.system.base.user.entity.hcs.HcsRoleEnum;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Arrays;

/**
 * VpcControllerTest
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(RequestUtil.class)
public class VpcControllerTest {
    @Mock
    private HcsTokenAPi hcsTokenAPi;

    @Mock
    private IVpcService vpcService;

    @Mock
    private OperationLogService operationLogService;

    @InjectMocks
    private VpcController vpcController;

    private HcsToken token;

    @Before
    public void init() {
        token = new HcsToken();
        Role role = new Role();
        role.setId("i1");
        role.setName(HcsRoleEnum.TE_ADMIN.getValue());
        token.setRoles(Arrays.asList(role));
        token.setUser(new TokenUser());
        PowerMockito.when(hcsTokenAPi.getTokenByProjectId(any())).thenReturn(token);
        PowerMockito.when(hcsTokenAPi.verifyAuthToken(any(), any())).thenReturn(token);
        PowerMockito.when(vpcService.saveVpcInfo(any(), any(), any())).thenReturn("id1");
        PowerMockito.when(vpcService.deleteVpcInfo(any())).thenReturn(true);
        VpcInfoEntity vpcInfoEntity = new VpcInfoEntity();
        vpcInfoEntity.setProjectId("p1");
        PowerMockito.when(vpcService.getProjectIdByMarkId(any())).thenReturn(vpcInfoEntity);
        PowerMockito.mockStatic(RequestUtil.class);
        PowerMockito.when(RequestUtil.getClientIpAddress(any())).thenReturn("");
        PowerMockito.doNothing().when(operationLogService).sendEvent(any());
    }

    @Test
    public void test_save_policy() {
        String token = "token12";
        VpcRequest vpcRequest = new VpcRequest();
        vpcRequest.setVpcId("id11");
        vpcRequest.setProjectId("pj13");
        String markId = "markId12";
        vpcController.savePolicy(token, null, vpcRequest, markId);
    }

    @Test
    public void test_delete_policy() {
        String markId = "markId12";
        String token = "token11";
        vpcController.deletePolicy(token, null, markId);
    }

    @Test
    public void test_check_permission() {
        String token = "token11";
        vpcController.checkPermission(token);
    }

    @Test(expected = LegoCheckedException.class)
    public void test_check_permission_fail() {
        token.getRoles().get(0).setName(HcsRoleEnum.OWNER.getValue());
        String token = "token11";
        vpcController.checkPermission(token);
    }
}
