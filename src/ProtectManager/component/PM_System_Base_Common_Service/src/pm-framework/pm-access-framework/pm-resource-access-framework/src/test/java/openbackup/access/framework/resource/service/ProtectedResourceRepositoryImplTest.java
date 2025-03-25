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
package openbackup.access.framework.resource.service;

import openbackup.access.framework.resource.persistence.dao.ProtectedAgentExtendMapper;
import openbackup.access.framework.resource.persistence.dao.ProtectedEnvironmentExtendInfoMapper;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceAgentMapper;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceExtendInfoMapper;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceMapper;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceRepositoryImpl;
import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;
import openbackup.access.framework.resource.provider.DefaultResourceProvider;
import openbackup.access.framework.resource.testutils.MyBatisPlusConfig;
import openbackup.data.access.framework.core.dao.ProtectedObjectMapper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedAgentExtend;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.query.PageQueryService;
import openbackup.system.base.service.DeployTypeService;

import com.baomidou.mybatisplus.autoconfigure.MybatisPlusAutoConfiguration;
import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mybatis.spring.annotation.MapperScan;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.redisson.api.RedissonClient;
import org.springframework.boot.SpringBootConfiguration;
import org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration;
import org.springframework.boot.autoconfigure.sql.init.SqlInitializationAutoConfiguration;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.junit4.SpringRunner;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

import javax.annotation.Resource;

/**
 * ProtectedResourceRepositoryImplTest
 *
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@MapperScan(basePackages = {"openbackup.access.framework.resource.persistence.dao","openbackup.data.access.framework.core.dao"})
@SpringBootConfiguration
@SpringBootTest(classes = {DataSourceAutoConfiguration.class, MybatisPlusAutoConfiguration.class,
    MyBatisPlusConfig.class, RedissonClient.class, SqlInitializationAutoConfiguration.class})
@Slf4j
public class ProtectedResourceRepositoryImplTest {
    @InjectMocks
    ProtectedResourceRepositoryImpl protectedResourceRepository;

    @Resource
    private ProtectedResourceMapper protectedResourceMapper;

    @Resource
    private ProtectedResourceExtendInfoMapper protectedResourceExtendInfoMapper;

    @Mock
    private PageQueryService pageQueryService;

    @Resource
    private ProtectedEnvironmentExtendInfoMapper protectedEnvironmentExtendInfoMapper;

    @Resource
    private ProtectedObjectMapper protectedObjectMapper;

    @Mock
    private ProviderManager providerManager;

    @Mock
    private DefaultResourceProvider defaultResourceProvider;

    @Mock
    private DeployTypeService deployTypeService;

    @Resource
    private ProtectedResourceAgentMapper protectedResourceAgentMapper;

    @Resource
    private ProtectedAgentExtendMapper protectedAgentExtendMapper;

    @Before
    public void prepare() throws Exception {
        protectedResourceRepository =
            new ProtectedResourceRepositoryImpl(protectedResourceMapper, protectedResourceExtendInfoMapper,
                pageQueryService, protectedEnvironmentExtendInfoMapper, protectedObjectMapper);
        Field field = protectedResourceRepository.getClass().getDeclaredField("providerManager");
        field.setAccessible(true);
        field.set(protectedResourceRepository, providerManager);
        Field field2 = protectedResourceRepository.getClass().getDeclaredField("defaultResourceProvider");
        field2.setAccessible(true);
        field2.set(protectedResourceRepository, defaultResourceProvider);
        Field field3 = protectedResourceRepository.getClass().getDeclaredField("deployTypeService");
        field3.setAccessible(true);
        field3.set(protectedResourceRepository, deployTypeService);
        Field field4 = protectedResourceRepository.getClass().getDeclaredField("protectedResourceAgentMapper");
        field4.setAccessible(true);
        field4.set(protectedResourceRepository, protectedResourceAgentMapper);
        Field field5 = protectedResourceRepository.getClass().getDeclaredField("protectedAgentExtendMapper");
        field5.setAccessible(true);
        field5.set(protectedResourceRepository, protectedAgentExtendMapper);
    }

    @Test
    public void select_agent_resource_list() {
        Map<String, Object> map = new HashMap<>();
        ArrayList<String> list = new ArrayList<>();
        list.add("DBBackupAgent");
        list.add("UBackupAgent");
        list.add("SBackupAgent");
        list.add("VMBackupAgent");
        map.put("type", "Host");
        map.put("subType", list);
        map.put("isCluster", false);
        map.put("version", "%1.5%");
        map.put("authorizedUser", "%user%");
        map.put("cpuRate", "asc");
        map.put("memRate", "desc");
        map.put("pageNo", 0);
        map.put("pageSize", 1);
        map.put("tag", "%test%");

        List<ProtectedResourcePo> protectedResourcePos = protectedResourceRepository.queryAgentResourceList(map);
        List<ProtectedResource> protectedResources =
            protectedResourcePos.stream().map(ProtectedResourcePo::toProtectedResource).collect(Collectors.toList());
        ProtectedAgentExtend protectedExtend = protectedResources.get(0).getProtectedAgentExtend();
        log.info("protectedExtend.getCpuRate():{}", protectedExtend.getCpuRate());
        Assert.assertEquals(protectedExtend.getCpuRate(), 0.12, 0.01);
        Assert.assertEquals(protectedResources.size(), 1);
    }

    @Test
    public void select_shared_agent_resource_list() {
        Map<String, Object> map = new HashMap<>();
        ArrayList<String> list = new ArrayList<>();
        list.add("DBBackupAgent");
        list.add("UBackupAgent");
        list.add("SBackupAgent");
        list.add("VMBackupAgent");
        map.put("type", "Host");
        map.put("subType", list);
        map.put("isCluster", false);
        map.put("version", "%1.5%");
        map.put("authorizedUser", "%user%");
        map.put("cpuRate", "asc");
        map.put("memRate", "desc");
        map.put("pageNo", 0);
        map.put("pageSize", 1);
        map.put("tag", "%test%");
        map.put("isShared", Lists.newArrayList(true));

        List<ProtectedResourcePo> protectedResourcePos = protectedResourceRepository.queryAgentResourceList(map);
        List<ProtectedResource> protectedResources =
            protectedResourcePos.stream().map(ProtectedResourcePo::toProtectedResource).collect(Collectors.toList());
        ProtectedAgentExtend protectedExtend = protectedResources.get(0).getProtectedAgentExtend();
        log.info("protectedExtend.getCpuRate():{}", protectedExtend.getCpuRate());
        Assert.assertEquals(protectedExtend.getCpuRate(), 0.12, 0.01);
        Assert.assertEquals(protectedResources.size(), 1);
        Assert.assertEquals(protectedResources.get(0).getUuid(), "a3f5d6b2-7c4e-11eb-9439-0242ac130002");
    }

    @Test
    public void select_agent_resource_list_count() {
        Map<String, Object> map = new HashMap<>();
        ArrayList<String> list = new ArrayList<>();
        list.add("DBBackupAgent");
        list.add("UBackupAgent");
        list.add("SBackupAgent");
        list.add("VMBackupAgent");
        map.put("type", "Host");
        map.put("subType", list);
        map.put("isCluster", false);
        map.put("version", "%1.5%");
        map.put("authorizedUser", "%user%");
        map.put("cpuRate", "asc");
        map.put("memRate", "desc");
        map.put("pageNo", 0);
        map.put("pageSize", 1);
        map.put("tag", "%test%");

        int count = protectedResourceRepository.queryAgentResourceCount(map);
        Assert.assertEquals(count, 1);
    }

    @Test
    public void select_agent_resource_list_default_order_by() {
        Map<String, Object> map = new HashMap<>();
        ArrayList<String> list = new ArrayList<>();
        list.add("VMBackupAgent");
        list.add("UBackupAgent");
        list.add("SBackupAgent");
        map.put("type", "Host");
        map.put("subType", list);
        map.put("isCluster", false);
        map.put("cpuRate", "asc");
        map.put("pageNo", 0);
        map.put("pageSize", 10);

        List<ProtectedResourcePo> protectedResourcePos = protectedResourceRepository.queryAgentResourceList(map);
        List<ProtectedResource> protectedResources =
                protectedResourcePos.stream().map(ProtectedResourcePo::toProtectedResource).collect(Collectors.toList());
        ProtectedAgentExtend protectedExtend = protectedResources.get(0).getProtectedAgentExtend();
        log.info("protectedExtend.getCpuRate():{}", protectedExtend.getCpuRate());
        Assert.assertEquals(protectedExtend.getCpuRate(), 0.12, 0.01);
    }

    @Test
    public void test_legoHostSighWithOldPrivateKey(){
        Map<String, Object> map = new HashMap<>();
        ArrayList<String> list = new ArrayList<>();
        list.add("DBBackupAgent");
        list.add("UBackupAgent");
        list.add("SBackupAgent");
        list.add("VMBackupAgent");
        map.put("type", "Host");
        map.put("subType", list);
        map.put("isCluster", false);
        map.put("version", "%1.5%");
        map.put("authorizedUser", "%user%");
        map.put("cpuRate", "asc");
        map.put("memRate", "desc");
        map.put("pageNo", 0);
        map.put("pageSize", 1);
        map.put("tag", "%test%");

        protectedResourceRepository.queryAgentResourceList(map);
        protectedResourceRepository.legoHostSighWithOldPrivateKey(false);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：更新agentShare字段
     * 前置条件：无
     * 检查点： 符合预期逻辑
     */
    @Test
    public void test_insert_agent_with_shared(){
        protectedResourceRepository.updateAgentShared("agentId1", null);
        Assert.assertEquals(false, protectedAgentExtendMapper.selectById("agentId1").getIsShared());
        protectedResourceRepository.updateAgentShared("agentId2", true);
        Assert.assertEquals(true, protectedAgentExtendMapper.selectById("agentId2").getIsShared());
        protectedResourceRepository.updateAgentShared("agentId2", false);
        Assert.assertEquals(false, protectedAgentExtendMapper.selectById("agentId2").getIsShared());
    }

    /**
     * 用例场景：查询指定插件类型agent
     * 前置条件：无
     * 检查点： 符合预期逻辑
     */
    @Test
    public void select_agent_resource_list_by_plugin_type() {
        Map<String, Object> map = new HashMap<>();
        map.put("pluginType", "HDFSFilesetPlugin");
        map.put("pageNo", 0);
        map.put("pageSize", 100);
        map.put("filteredByPluginTypeIds", Lists.newArrayList("d620cba5-e890-4af6-9ab6-835bb3e106d1"));
        List<ProtectedResourcePo> protectedResourcePos = protectedResourceRepository.queryAgentResourceList(map);
        List<ProtectedResource> protectedResources =
            protectedResourcePos.stream().map(ProtectedResourcePo::toProtectedResource).collect(Collectors.toList());
        ProtectedAgentExtend protectedExtend = protectedResources.get(0).getProtectedAgentExtend();
        log.info("protectedExtend.getCpuRate():{}", protectedExtend.getCpuRate());
        Assert.assertEquals(protectedExtend.getCpuRate(), 0.22, 0.35);
        Assert.assertEquals(protectedResources.size(), 1);
    }
}