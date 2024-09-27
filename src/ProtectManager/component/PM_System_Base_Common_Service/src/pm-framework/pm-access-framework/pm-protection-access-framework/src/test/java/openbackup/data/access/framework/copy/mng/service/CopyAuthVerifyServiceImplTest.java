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
package openbackup.data.access.framework.copy.mng.service;

import openbackup.data.access.framework.copy.mng.service.impl.CopyAuthVerifyServiceImpl;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.user.AuthServiceApi;
import openbackup.system.base.sdk.user.DomainResourceSetServiceApi;
import openbackup.system.base.sdk.user.ResourceSetResourceServiceApi;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;
import openbackup.system.base.service.DeployTypeService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.boot.test.context.SpringBootTest;

import java.util.ArrayList;

/**
 * 功能描述
 *
 * @author z00842230
 * @since 2024-07-17
 */
@RunWith(PowerMockRunner.class)
@SpringBootTest()
@PrepareForTest({TokenBo.class, CopyAuthVerifyServiceImpl.class})
public class CopyAuthVerifyServiceImplTest {
    @Mock
    private DeployTypeService deployTypeService;

    @Mock
    private AuthServiceApi authServiceApi;

    @Mock
    private ResourceSetResourceServiceApi resourceSetResourceServiceApi;

    @Mock
    private DomainResourceSetServiceApi domainResourceSetServiceApi;

    @InjectMocks
    private CopyAuthVerifyServiceImpl copyAuthVerifyService;

    @Test
    public void test_has_no_copy_query_auth_when_copy_not_in_domain() {
        PowerMockito.when(deployTypeService.isNotSupportMemberCluster()).thenReturn(false);
        PowerMockito.mockStatic(TokenBo.class);
        TokenBo tokenBo = new TokenBo();
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setDomainId("123");
        tokenBo.setUser(userBo);
        PowerMockito.when(TokenBo.get()).thenReturn(tokenBo);
        PowerMockito.when(domainResourceSetServiceApi.getResourceSetType(Mockito.anyString(), Mockito.anyString(),
            Mockito.anyString())).thenReturn(ResourceSetTypeEnum.COPY.getType());
        Assert.assertThrows(LegoCheckedException.class,
            () -> copyAuthVerifyService.checkCopyQueryAuth(prepareReplicationCopy(
                CopyGeneratedByEnum.BY_BACKUP.value())));
    }

    @Test
    public void test_has_no_copy_operation_auth_when_replication_copy() {
        PowerMockito.when(deployTypeService.isNotSupportMemberCluster()).thenReturn(false);
        PowerMockito.mockStatic(TokenBo.class);
        TokenBo tokenBo = new TokenBo();
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setDomainId("123");
        tokenBo.setUser(userBo);
        PowerMockito.when(TokenBo.get()).thenReturn(tokenBo);
        PowerMockito.when(authServiceApi.isDefaultRoleHasAuthOperation(Mockito.anyString(),
            Mockito.anyList())).thenReturn(false);
        PowerMockito.when(resourceSetResourceServiceApi.checkHasResourceOperation(Mockito.anyString(),
            Mockito.anyList(), Mockito.anyString(), Mockito.anyString())).thenReturn(false);
        Assert.assertThrows(LegoCheckedException.class,
            () -> copyAuthVerifyService.checkCopyOperationAuth(prepareReplicationCopy(
                CopyGeneratedByEnum.BY_REPLICATED.value()), new ArrayList<>()));
    }

    @Test
    public void test_has_copy_operation_auth_when_backup_copy() {
        PowerMockito.when(deployTypeService.isNotSupportMemberCluster()).thenReturn(false);
        PowerMockito.mockStatic(TokenBo.class);
        TokenBo tokenBo = new TokenBo();
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setDomainId("123");
        tokenBo.setUser(userBo);
        PowerMockito.when(TokenBo.get()).thenReturn(tokenBo);
        PowerMockito.when(resourceSetResourceServiceApi.checkHasResourceOperation(Mockito.anyString(),
            Mockito.anyList(), Mockito.anyString(), Mockito.anyString())).thenReturn(false);
        Assert.assertThrows(LegoCheckedException.class,
            () -> copyAuthVerifyService.checkCopyOperationAuth(prepareReplicationCopy(
                CopyGeneratedByEnum.BY_BACKUP.value()), new ArrayList<>()));
    }

    private Copy prepareReplicationCopy(String generateBy) {
        Copy copy = new Copy();
        copy.setGeneratedBy(generateBy);
        copy.setUuid("123456");
        return copy;
    }
}
