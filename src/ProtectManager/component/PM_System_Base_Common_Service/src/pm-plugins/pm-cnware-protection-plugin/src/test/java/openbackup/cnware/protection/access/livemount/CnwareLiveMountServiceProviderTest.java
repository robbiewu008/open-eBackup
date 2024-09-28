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
package openbackup.cnware.protection.access.livemount;

import openbackup.data.access.framework.livemount.common.model.LiveMountFileSystemShareInfo;
import openbackup.data.access.framework.livemount.common.model.LiveMountObject;

import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.livemount.model.LiveMountTargetLocation;
import openbackup.system.base.sdk.resource.model.ResourceEntity;

import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(TokenBo.class)
public class CnwareLiveMountServiceProviderTest {
    private CnwareLiveMountServiceProvider cnwareLiveMountServiceProviderTest;
    @Before
    public void setUp() {
        cnwareLiveMountServiceProviderTest =
            new CnwareLiveMountServiceProvider();
    }

    /**
     * 用例场景：Cnware备份插件类型判断正确 <br/>
     * 前置条件：流程正常 <br/>
     * 检查点：返回结果为True
     */
    @Test
    public void test_applicable_success() {
        Assert.assertTrue(cnwareLiveMountServiceProviderTest.applicable("CNwareVm"));
    }

    /**
     * 用例场景：构造即时挂载实体类
     * 前置条件：无
     * 检查点：即时挂载实体类构造成功
     */
    @Test
    public void test_build_cnware_livemount_entity_success_when_all_parameters_are_normal() {
        mockUserToken();
        CnwareLiveMountServiceProvider provider = new CnwareLiveMountServiceProvider();
        LiveMountObject liveMountObject = new LiveMountObject();
        liveMountObject.setTargetLocation(LiveMountTargetLocation.OTHERS);
        Map<String, Object> map = Optional.ofNullable(liveMountObject.getParameters()).orElse(new HashMap<>());
        map.put("isDeleteOriginalVM", Boolean.FALSE);
        liveMountObject.setParameters(map);
        LiveMountFileSystemShareInfo shareInfo = new LiveMountFileSystemShareInfo();
        shareInfo.setFileSystemName("Test-file-system");
        liveMountObject.setFileSystemShareInfoList(Collections.singletonList(shareInfo));
        ResourceEntity sourceResourceEntity = new ResourceEntity();
        ResourceEntity targetResourceEntity = new ResourceEntity();
        String uuid = UUIDGenerator.getUUID();
        targetResourceEntity.setUuid(uuid);
        LiveMountEntity liveMountEntity =
            cnwareLiveMountServiceProviderTest.buildLiveMountEntity(liveMountObject,
                sourceResourceEntity, targetResourceEntity);
        Assert.assertNotNull(liveMountEntity);
        List<LiveMountFileSystemShareInfo> list = JSONArray.fromObject(liveMountEntity.getFileSystemShareInfo())
            .toBean(LiveMountFileSystemShareInfo.class);
        Assert.assertEquals("Test-file-system" + uuid, list.get(0).getFileSystemName());
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
