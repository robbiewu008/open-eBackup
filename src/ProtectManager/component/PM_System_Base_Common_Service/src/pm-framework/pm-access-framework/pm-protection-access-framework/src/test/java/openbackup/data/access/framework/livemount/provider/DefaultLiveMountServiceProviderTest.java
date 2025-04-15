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
package openbackup.data.access.framework.livemount.provider;

import openbackup.data.access.client.sdk.api.framework.dme.DmeBackupClone;
import openbackup.data.access.framework.livemount.common.model.LiveMountObject;
import openbackup.data.access.framework.livemount.provider.CloneCopyParam;
import openbackup.data.access.framework.livemount.provider.DefaultLiveMountServiceProvider;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.livemount.model.LiveMountTargetLocation;
import openbackup.system.base.sdk.resource.model.ResourceEntity;

import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;
import java.util.List;

/**
 * 功能描述: DefaultLiveMountServiceProviderTest
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {TokenBo.class})
public class DefaultLiveMountServiceProviderTest {
    /**
     * 用例场景：生成克隆副本请求参数
     * 前置条件：无
     * 检查点：请求参数生成成功
     */
    @Test
    public void test_build_dme_clone_copy_request() {
        CloneCopyParam cloneCopyParam = new CloneCopyParam(UUIDGenerator.getUUID(), UUIDGenerator.getUUID(),
                UUIDGenerator.getUUID(), Collections.emptyList(), null);
        DefaultLiveMountServiceProvider defaultLiveMountServiceProvider = new DefaultLiveMountServiceProvider();
        DmeBackupClone dmeBackupClone = defaultLiveMountServiceProvider.buildDmeCloneCopyRequest(cloneCopyParam);
        Assert.assertNotNull(dmeBackupClone);
    }

    /**
     * 用例场景：校验副本是否可以被挂载
     * 前置条件：无
     * 检查点：检查成功
     */
    @Test
    public void test_check_copy_can_be_mounted_success() {
        DefaultLiveMountServiceProvider provider = new DefaultLiveMountServiceProvider();
        Assert.assertThrows(LegoCheckedException.class, () -> provider.isSourceCopyCanBeMounted(null, true));
        Assert.assertFalse(provider.isSourceCopyCanBeMounted(null, false));
        Assert.assertTrue(provider.isSourceCopyCanBeMounted(new Copy(), false));
    }

    /**
     * 用例场景：构造即时挂载实体类
     * 前置条件：无
     * 检查点：即时挂载实体类构造成功
     */
    @Test
    public void test_build_livemount_entity_success() {
        mockUserToken();
        DefaultLiveMountServiceProvider provider = new DefaultLiveMountServiceProvider();
        LiveMountObject liveMountObject = new LiveMountObject();
        liveMountObject.setTargetLocation(LiveMountTargetLocation.OTHERS);
        ResourceEntity sourceResourceEntity = new ResourceEntity();
        ResourceEntity targetResourceEntity = new ResourceEntity();
        LiveMountEntity liveMountEntity = provider.buildLiveMountEntity(liveMountObject, sourceResourceEntity, targetResourceEntity);
        Assert.assertNotNull(liveMountEntity);
    }

    private void mockUserToken() {
        TokenBo.RoleBo roleBo = new TokenBo.RoleBo();
        roleBo.setName(Constants.Builtin.ROLE_SYS_ADMIN);
        roleBo.setId("1");
        List<TokenBo.RoleBo> roles = Lists.newArrayList(roleBo);
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setId("test-user-id");
        userBo.setRoles(roles);
        PowerMockito.mockStatic(TokenBo.class);
        TokenBo tokenBo = PowerMockito.mock(TokenBo.class);
        PowerMockito.when(TokenBo.get()).thenReturn(tokenBo);
        PowerMockito.when(tokenBo.getUser()).thenReturn(userBo);
    }
}