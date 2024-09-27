/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.oracle.livemount;

import openbackup.data.access.framework.livemount.common.model.LiveMountFileSystemShareInfo;
import openbackup.data.access.framework.livemount.common.model.LiveMountObject;

import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.livemount.model.LiveMountTargetLocation;
import openbackup.system.base.sdk.resource.model.ResourceEntity;

import com.google.common.collect.Lists;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;
import java.util.List;

/**
 * 功能描述: OracleLiveMountServiceProviderTest
 *
 * @author c30016231
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023/02/24
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(TokenBo.class)
public class OracleLiveMountServiceProviderTest {
    private OracleLiveMountServiceProvider oracleLiveMountServiceProvider;
    private final CopyRestApi copyRestApi = Mockito.mock(CopyRestApi.class);

    @Before
    public void init() {
        this.oracleLiveMountServiceProvider = new OracleLiveMountServiceProvider(copyRestApi);
    }

    /**
     * 用例场景：构造即时挂载实体类 <br/>
     * 前置条件：无  <br/>
     * 检查点：即时挂载实体类构造成功
     */
    @Test
    public void test_build_oracle_live_mount_entity_success() {
        mockUserToken();
        LiveMountObject liveMountObject = mockLiveMountObject();
        ResourceEntity sourceResourceEntity = new ResourceEntity();
        ResourceEntity targetResourceEntity = new ResourceEntity();
        String uuid = UUIDGenerator.getUUID();
        targetResourceEntity.setUuid(uuid);
        Mockito.when(copyRestApi.queryCopyByID(Mockito.anyString())).thenReturn(new Copy());
        LiveMountEntity liveMountEntity = oracleLiveMountServiceProvider.buildLiveMountEntity(liveMountObject,
            sourceResourceEntity, targetResourceEntity);

        Assert.assertNotNull(liveMountEntity);
        List<LiveMountFileSystemShareInfo> list = JSONArray.fromObject(liveMountEntity.getFileSystemShareInfo())
            .toBean(LiveMountFileSystemShareInfo.class);
        Assert.assertEquals("Test-file-system" + uuid, list.get(0).getFileSystemName());
    }

    /**
     * 用例场景：构造即时挂载实体类 <br/>
     * 前置条件：副本是即时挂载副本  <br/>
     * 检查点：即时挂载实体类构造成功
     */
    @Test
    public void test_build_oracle_live_mount_entity_success_if_copy_generated_by_live_mount() {
        mockUserToken();
        String fileSystemName = "Test-file-system";
        ResourceEntity sourceResourceEntity = new ResourceEntity();
        ResourceEntity targetResourceEntity = new ResourceEntity();
        String uuid = UUIDGenerator.getUUID();
        targetResourceEntity.setUuid(uuid);
        Copy copy = new Copy();
        copy.setGeneratedBy("live_mount");
        Mockito.when(copyRestApi.queryCopyByID(Mockito.anyString())).thenReturn(copy);

        LiveMountEntity liveMountEntity = oracleLiveMountServiceProvider.buildLiveMountEntity(mockLiveMountObject(),
            sourceResourceEntity, targetResourceEntity);

        Assert.assertNotNull(liveMountEntity);
        List<LiveMountFileSystemShareInfo> list = JSONArray.fromObject(liveMountEntity.getFileSystemShareInfo())
            .toBean(LiveMountFileSystemShareInfo.class);
        Assert.assertEquals(fileSystemName, list.get(0).getFileSystemName());
    }

    /**
     * 测试场景：applicable匹配成功 <br/>
     * 前置条件：传入资源信息subtype类型为Oracle <br/>
     * 检查点：返回True
     */
    @Test
    public void test_applicable_success() {
        boolean applicable = oracleLiveMountServiceProvider.applicable(ResourceSubTypeEnum.ORACLE.getType());
        Assert.assertTrue(applicable);
    }


    /**
     * 测试场景：测试日志副本不可挂载 <br/>
     * 前置条件：副本类型为日志副本 <br/>
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_exception_when_is_source_copy_can_be_mounted_success_if_latest_copy_is_log_copy() {
        Copy copy = new Copy();
        copy.setSourceCopyType(4);
        Assert.assertThrows("latest log copy is not available.", LegoCheckedException.class,
            () -> oracleLiveMountServiceProvider.isSourceCopyCanBeMounted(copy, true));
    }

    private LiveMountObject mockLiveMountObject() {
        LiveMountObject liveMountObject = new LiveMountObject();
        liveMountObject.setTargetLocation(LiveMountTargetLocation.OTHERS);
        liveMountObject.setCopyId("copy_id");
        LiveMountFileSystemShareInfo shareInfo = new LiveMountFileSystemShareInfo();
        shareInfo.setFileSystemName("Test-file-system");
        liveMountObject.setFileSystemShareInfoList(Collections.singletonList(shareInfo));
        return liveMountObject;
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