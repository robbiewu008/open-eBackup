/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.access.framework.resource.persistence.dao;

import openbackup.access.framework.resource.persistence.dao.ProtectedEnvironmentExtendInfoMapper;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceExtendInfoMapper;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceMapper;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceRepositoryImpl;
import openbackup.access.framework.resource.persistence.model.ProtectedEnvironmentExtendInfoPo;
import openbackup.access.framework.resource.persistence.model.ProtectedEnvironmentPo;
import openbackup.data.access.framework.core.dao.ProtectedObjectMapper;
import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.access.framework.resource.persistence.model.ProtectedResourceExtendInfoPo;
import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;
import openbackup.access.framework.resource.provider.DefaultResourceProvider;
import openbackup.access.framework.resource.service.ProtectedResourceRepository;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteParams;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.enums.ConsistentStatusEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.config.configmap.ConfigMapService;
import openbackup.system.base.query.PageQueryService;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.service.DeployTypeService;

import com.baomidou.mybatisplus.autoconfigure.MybatisPlusAutoConfiguration;
import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;

import org.apache.commons.lang3.StringUtils;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mock;
import org.mybatis.spring.annotation.MapperScan;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration;
import org.springframework.boot.autoconfigure.sql.init.SqlInitializationAutoConfiguration;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.util.CollectionUtils;

import java.lang.reflect.Field;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;

import javax.annotation.Resource;

@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@MapperScan(basePackages = {"openbackup.access.framework.resource.persistence.dao","openbackup.data.access.framework.core.dao"})
@SpringBootTest(classes = {
    DataSourceAutoConfiguration.class, MybatisPlusAutoConfiguration.class, SqlInitializationAutoConfiguration.class,
        ProtectedResourceRepositoryImpl.class,
    PageQueryService.class, DeployTypeService.class,
})
@MockBean( {SessionService.class, ConfigMapService.class})
public class ProtectedResourceRepositoryTest {
    @Resource
    private ProtectedResourceMapper protectedResourceMapper;

    @Resource
    private ProtectedResourceExtendInfoMapper resourceExtendInfoMapper;

    @Resource
    private ProtectedObjectMapper protectedObjectMapper;

    @Mock
    private ProtectedObjectMapper protectedObjectMockMapper;

    @Mock
    private DeployTypeService deployTypeService;

    @Resource
    private ProtectedEnvironmentExtendInfoMapper protectedEnvironmentExtendInfoMapper;

    @Autowired
    private ProtectedResourceRepository protectedResourceRepository;

    @MockBean
    private ProviderManager providerManager;

    @MockBean(name = "defaultResourceProvider")
    private DefaultResourceProvider defaultResourceProvider;

    @Before
    public void prepare() {
        protectedResourceMapper.delete(new QueryWrapper<>());
        protectedObjectMapper.delete(new QueryWrapper<>());
    }

    /**
     * 用例场景：删除设备失败
     * 前置条件：设备下的文件系统绑定了智能侦测策略
     * 检查点: 校验异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_resource_bind_intellect_detection_policy()
        throws NoSuchFieldException, IllegalAccessException {
        Class<?> classType = protectedResourceRepository.getClass();
        Field storageDataVerifyServiceField = classType.getDeclaredField("protectedObjectMapper");
        storageDataVerifyServiceField.setAccessible(true);
        storageDataVerifyServiceField.set(protectedResourceRepository, protectedObjectMockMapper);
        Field deployTypeServiceField = classType.getDeclaredField("deployTypeService");
        deployTypeServiceField.setAccessible(true);
        deployTypeServiceField.set(protectedResourceRepository, deployTypeService);
        List<ProtectedObjectPo> protectedObjectPos = Arrays.asList(new ProtectedObjectPo());
        PowerMockito.when(protectedObjectMockMapper.selectList(ArgumentMatchers.any(LambdaQueryWrapper.class)))
            .thenReturn(protectedObjectPos);
        PowerMockito.when(deployTypeService.isCyberEngine()).thenReturn(true);
        ResourceDeleteParams params = new ResourceDeleteParams();
        params.setForce(false);
        params.setShouldDeleteRegister(true);
        params.setResources(new String[] {"22", "33", "11"});
        LegoCheckedException dataProtectionAccessException = Assert.assertThrows(LegoCheckedException.class,
            () -> protectedResourceRepository.delete(params));
        Assert.assertEquals(ProtectedResourceRepositoryImpl.RESOURCE_ALREADY_BIND_INTELLECT_DETECTION_POLICY,
            dataProtectionAccessException.getErrorCode());

    }

    /**
     * 用例场景：删除设备失败
     * 前置条件：设备下的资源绑定了sla
     * 检查点: 校验异常
     */
    @Test
    public void test_querdy_exist_uuid_by_sou2rce_type() throws NoSuchFieldException, IllegalAccessException {
        Class<?> classType = protectedResourceRepository.getClass();
        Field storageDataVerifyServiceField = classType.getDeclaredField("protectedObjectMapper");
        storageDataVerifyServiceField.setAccessible(true);
        storageDataVerifyServiceField.set(protectedResourceRepository, protectedObjectMockMapper);
        Field deployTypeServiceField = classType.getDeclaredField("deployTypeService");
        deployTypeServiceField.setAccessible(true);
        deployTypeServiceField.set(protectedResourceRepository, deployTypeService);
        List<ProtectedObjectPo> protectedObjectPos = Arrays.asList(new ProtectedObjectPo());
        PowerMockito.when(protectedObjectMockMapper.selectList(ArgumentMatchers.any(LambdaQueryWrapper.class)))
            .thenReturn(protectedObjectPos);
        PowerMockito.when(deployTypeService.isCyberEngine()).thenReturn(false);
        ResourceDeleteParams params = new ResourceDeleteParams();
        params.setForce(false);
        params.setShouldDeleteRegister(true);
        params.setResources(new String[] {"22", "33", "11"});
        LegoCheckedException dataProtectionAccessException = Assert.assertThrows(LegoCheckedException.class,
            () -> protectedResourceRepository.delete(params));
        Assert.assertEquals(ProtectedResourceRepositoryImpl.RESOURCE_ALREADY_BIND_SLA,
            dataProtectionAccessException.getErrorCode());

    }

    @Test
    public void test_query_exist_uuid_by_source_type() {
        List<String> allUuids = new ArrayList<>();
        allUuids.add(createResource("1", ResourceConstants.SOURCE_TYPE_REGISTER));
        allUuids.add(createResource("2", ResourceConstants.SOURCE_TYPE_REGISTER));
        allUuids.add(createResource("3", "other"));
        List<String> registerUuids = protectedResourceRepository.queryExistResourceUuidBySourceType(allUuids,
            ResourceConstants.SOURCE_TYPE_REGISTER);
        Assert.assertArrayEquals(registerUuids.toArray(new String[0]), new String[] {"1", "2"});
    }

    @Test
    public void test_update_link_status() {
        createResource("env1", ResourceConstants.SOURCE_TYPE_REGISTER);
        ProtectedEnvironmentExtendInfoPo protectedEnvironmentExtendInfoPo = new ProtectedEnvironmentExtendInfoPo();
        protectedEnvironmentExtendInfoPo.setUuid("env1");
        protectedEnvironmentExtendInfoPo.setLinkStatus("4");
        protectedEnvironmentExtendInfoMapper.insert(protectedEnvironmentExtendInfoPo);
        protectedResourceRepository.updateLinkStatus("env1", "1");
        ProtectedEnvironmentExtendInfoPo env1 = protectedEnvironmentExtendInfoMapper.selectById("env1");
        Assert.assertEquals(env1.getLinkStatus(), "1");
    }

    @Test
    public void test_query_root_uuid() {
        Assert.assertNull(protectedResourceMapper.queryRootUuid("0"));
        insert("1", null);
        Assert.assertEquals("1", protectedResourceMapper.queryRootUuid("1"));
        insert("2", "1");
        Assert.assertEquals("1", protectedResourceMapper.queryRootUuid("1"));
        Assert.assertEquals("1", protectedResourceMapper.queryRootUuid("2"));
        insert("3", "2");
        Assert.assertEquals("1", protectedResourceMapper.queryRootUuid("1"));
        Assert.assertEquals("1", protectedResourceMapper.queryRootUuid("2"));
        Assert.assertEquals("1", protectedResourceMapper.queryRootUuid("3"));
        insert("4", null);
        Assert.assertEquals("4", protectedResourceMapper.queryRootUuid("4"));
        insert("5", "4");
        Assert.assertEquals("4", protectedResourceMapper.queryRootUuid("5"));
    }

    @Test
    public void test_query_resources_by_user_id() {
        String userId = "test";
        Assert.assertTrue(CollectionUtils.isEmpty(protectedResourceMapper.queryResourcesByUserId(userId)));
        ProtectedResourcePo resourcePo = insertResource(userId);
        ProtectedObjectPo platform = mockProtectObject(ResourceTypeEnum.PLATFORM, ConsistentStatusEnum.UNDETECTED);
        platform.setResourceId(resourcePo.getUuid());
        platform.setUuid(resourcePo.getUuid());
        insertProtectObject(platform);
        List<ProtectedEnvironmentPo> protectedEnvironmentPos = protectedResourceMapper.queryResourcesByUserId(userId);
        Assert.assertTrue(protectedEnvironmentPos.size() == 1);
    }

    private ProtectedResourcePo insertResource(String userId) {
        ProtectedResourcePo resourcePo = new ProtectedResourcePo();
        resourcePo.setUuid(UUID.randomUUID().toString());
        resourcePo.setUserId(userId);
        resourcePo.setCreatedTime(new Timestamp(System.currentTimeMillis()));
        protectedResourceMapper.insert(resourcePo);
        return resourcePo;
    }

    private String createResource(String uuid, String sourceType) {
        ProtectedResourcePo resourcePo = new ProtectedResourcePo();
        resourcePo.setUuid(uuid);
        resourcePo.setSourceType(sourceType);
        resourcePo.setCreatedTime(new Timestamp(System.currentTimeMillis()));
        protectedResourceMapper.insert(resourcePo);
        return uuid;
    }

    private void insert(String uuid, String parentUuid) {
        ProtectedResourcePo resourcePo = new ProtectedResourcePo();
        resourcePo.setUuid(uuid);
        resourcePo.setParentUuid(parentUuid);
        resourcePo.setCreatedTime(new Timestamp(System.currentTimeMillis()));
        protectedResourceMapper.insert(resourcePo);
    }

    /**
     * 用例名称：验证保护对象查询逻辑是否正确
     * 前置条件：数据库中存在相应的保护对象数据
     * 检查点：保护对象查询成功
     */
    @Test
    public void test_query_protect_object_success() {
        ProtectedObjectPo platform = mockProtectObject(ResourceTypeEnum.PLATFORM, ConsistentStatusEnum.UNDETECTED);
        ProtectedObjectPo cluster = mockProtectObject(ResourceTypeEnum.CLUSTER, ConsistentStatusEnum.UNDETECTED);
        ProtectedObjectPo host = mockProtectObject(ResourceTypeEnum.HOST, ConsistentStatusEnum.UNDETECTED);
        insertProtectObject(platform);
        insertProtectObject(cluster);
        insertProtectObject(host);
        int count = protectedResourceRepository.queryProtectObjectCountByConsistentStatus(
            ConsistentStatusEnum.UNDETECTED);
        Assert.assertEquals(3, count);
        List<String> idList = protectedResourceRepository.queryProtectObjectIdListByConsistentStatus(
            ConsistentStatusEnum.UNDETECTED);
        Assert.assertEquals(3, idList.size());
        Assert.assertEquals(platform.getUuid(), idList.get(0));
        Assert.assertEquals(cluster.getUuid(), idList.get(1));
        Assert.assertEquals(host.getUuid(), idList.get(2));
        ProtectedObjectPo protectedObjectPo = protectedResourceRepository.queryProtectObjectById(platform.getUuid());
        Assert.assertEquals(ConsistentStatusEnum.UNDETECTED.getStatus(), protectedObjectPo.getConsistentStatus());
        Assert.assertEquals(platform.getUuid(), protectedObjectPo.getUuid());
    }

    /**
     * 用例名称：验证保护对象更新逻辑是否正确
     * 前置条件：数据库中存在相应的保护对象数据
     * 检查点：保护对象更新成功
     */
    @Test
    public void test_update_project_object_success() {
        ProtectedObjectPo platform = mockProtectObject(ResourceTypeEnum.PLATFORM, ConsistentStatusEnum.UNDETECTED);
        ProtectedObjectPo cluster = mockProtectObject(ResourceTypeEnum.CLUSTER, ConsistentStatusEnum.CONSISTENT);
        ProtectedObjectPo host = mockProtectObject(ResourceTypeEnum.HOST, ConsistentStatusEnum.INCONSISTENT);
        insertProtectObject(platform);
        insertProtectObject(cluster);
        insertProtectObject(host);
        int count = protectedResourceRepository.queryProtectObjectCountByConsistentStatus(
            ConsistentStatusEnum.UNDETECTED);
        Assert.assertEquals(1, count);
        protectedResourceRepository.updateProtectObjectConsistentById(cluster.getUuid(),
            ConsistentStatusEnum.UNDETECTED, StringUtils.EMPTY);
        count = protectedResourceRepository.queryProtectObjectCountByConsistentStatus(ConsistentStatusEnum.UNDETECTED);
        Assert.assertEquals(2, count);
        protectedResourceRepository.updateAllProtectObjectConsistentStatus(ConsistentStatusEnum.UNDETECTED);
        count = protectedResourceRepository.queryProtectObjectCountByConsistentStatus(ConsistentStatusEnum.UNDETECTED);
        Assert.assertEquals(3, count);
    }

    /**
     * 用例名称：根据key和资源id删除资源拓展信息
     * 前置条件：数据库中存在相应的保护对象数据
     * 检查点：成功删除
     */
    @Test
    public void test_delete_protect_resource_extend_info_by_resource_id() {
        String uuid = UUIDGenerator.getUUID();
        ProtectedResourcePo resourcePo = new ProtectedResourcePo();
        resourcePo.setUuid("0d72bf9e-05e9-48f4-833b-411c74694047");
        resourcePo.setUserId("userId");
        resourcePo.setCreatedTime(new Timestamp(System.currentTimeMillis()));
        protectedResourceMapper.insert(resourcePo);
        ProtectedResourceExtendInfoPo resourceExtendInfoPo = new ProtectedResourceExtendInfoPo();
        String key = Constants.MANUAL_UNINSTALLATION;
        String value = "1";
        resourceExtendInfoPo.setResourceId("0d72bf9e-05e9-48f4-833b-411c74694047");
        resourceExtendInfoPo.setUuid(uuid);
        resourceExtendInfoPo.setKey(key);
        resourceExtendInfoPo.setValue(value);
        resourceExtendInfoMapper.insert(resourceExtendInfoPo);
        ProtectedResourceExtendInfoPo protectedResourceExtendInfoPo = resourceExtendInfoMapper.selectByResourceId(
            "0d72bf9e-05e9-48f4-833b-411c74694047");
        Assert.assertNotNull(protectedResourceExtendInfoPo);
        protectedResourceRepository.deleteProtectResourceExtendInfoByResourceId("0d72bf9e-05e9-48f4-833b-411c74694047",
            Constants.MANUAL_UNINSTALLATION);
        ProtectedResourceExtendInfoPo protectedResourceExtendInfoPo2 = resourceExtendInfoMapper.selectByResourceId(
            "0d72bf9e-05e9-48f4-833b-411c74694047");
        Assert.assertNull(protectedResourceExtendInfoPo2);
    }


    @Test
    public void test_query_extendInfo_list_by_resource_id_and_key() {
        ProtectedResourceExtendInfoPo resourceExtendInfoPo = new ProtectedResourceExtendInfoPo();
        String uuid = UUIDGenerator.getUUID();
        ProtectedResourcePo resourcePo = new ProtectedResourcePo();
        resourcePo.setUuid(uuid);
        resourcePo.setUserId("userId");
        resourcePo.setCreatedTime(new Timestamp(System.currentTimeMillis()));
        protectedResourceMapper.insert(resourcePo);
        String key = "key1";
        String value = "value1";
        resourceExtendInfoPo.setResourceId(uuid);
        resourceExtendInfoPo.setUuid(UUIDGenerator.getUUID());
        resourceExtendInfoPo.setKey(key);
        resourceExtendInfoPo.setValue(value);
        resourceExtendInfoMapper.insert(resourceExtendInfoPo);
        List<ProtectedResourceExtendInfoPo> resourceExtendInfoPos =
            protectedResourceRepository.queryExtendInfoListByResourceIdAndKey(uuid, key);
        Assert.assertEquals(resourceExtendInfoPos.get(0).getValue(), value);
    }

    /**
     * 用例名称：安全一体机删除设备
     * 前置条件：数据库中存在相应的受保护环境
     * 检查点：删除成功
     */
    @Test
    public void test_delete_cyber_engine_environment_success(){
       ProtectedResourcePo resourcePo = new ProtectedResourcePo();
       resourcePo.setSubType("FileSystem");
       resourcePo.setRootUuid("0");
       resourcePo.setUuid("1");
        resourcePo.setCreatedTime(new Timestamp(System.currentTimeMillis()));
       insertProtectResource(resourcePo);

        Assert.assertEquals(2,protectedResourceRepository.deleteCyberEngineEnvironment("0").size());
    }

    private void insertProtectObject(ProtectedObjectPo protectedObjectPo) {
        protectedObjectMapper.insert(protectedObjectPo);
    }

    private void insertProtectResource(ProtectedResourcePo protectedResourcePo){
        protectedResourceMapper.insert(protectedResourcePo);
    }

    private ProtectedObjectPo mockProtectObject(ResourceTypeEnum type, ConsistentStatusEnum consistentStatus) {
        ProtectedObjectPo protectedObjectPo = new ProtectedObjectPo();
        protectedObjectPo.setUuid(UUIDGenerator.getUUID());
        protectedObjectPo.setResourceId(UUIDGenerator.getUUID());
        protectedObjectPo.setType(type.getType());
        protectedObjectPo.setChainId(UUIDGenerator.getUUID());
        protectedObjectPo.setConsistentStatus(consistentStatus.getStatus());
        return protectedObjectPo;
    }
}
