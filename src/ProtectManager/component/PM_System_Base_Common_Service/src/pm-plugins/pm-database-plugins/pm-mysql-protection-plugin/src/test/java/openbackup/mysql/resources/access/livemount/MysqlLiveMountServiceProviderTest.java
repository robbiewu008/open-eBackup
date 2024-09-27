package openbackup.mysql.resources.access.livemount;

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
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;
import java.util.List;

/**
 * 功能描述: MysqlLiveMountServiceProviderTest
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-10-09
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {TokenBo.class})
public class MysqlLiveMountServiceProviderTest {
    /**
     * 用例场景：构造即时挂载实体类
     * 前置条件：无
     * 检查点：即时挂载实体类构造成功
     */
    @Test
    public void test_build_mysql_livemount_entity_success() {
        mockUserToken();
        MysqlLiveMountServiceProvider provider = new MysqlLiveMountServiceProvider();
        LiveMountObject liveMountObject = new LiveMountObject();
        liveMountObject.setTargetLocation(LiveMountTargetLocation.OTHERS);
        LiveMountFileSystemShareInfo shareInfo = new LiveMountFileSystemShareInfo();
        shareInfo.setFileSystemName("Test-file-system");
        liveMountObject.setFileSystemShareInfoList(Collections.singletonList(shareInfo));
        ResourceEntity sourceResourceEntity = new ResourceEntity();
        ResourceEntity targetResourceEntity = new ResourceEntity();
        String uuid = UUIDGenerator.getUUID();
        targetResourceEntity.setUuid(uuid);
        LiveMountEntity liveMountEntity = provider.buildLiveMountEntity(liveMountObject,
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