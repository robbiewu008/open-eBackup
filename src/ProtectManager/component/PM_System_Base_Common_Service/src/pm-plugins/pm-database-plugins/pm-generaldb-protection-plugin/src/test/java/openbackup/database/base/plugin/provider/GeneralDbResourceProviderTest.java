package openbackup.database.base.plugin.provider;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.AppConf;
import openbackup.database.base.plugin.common.GeneralDbConstant;
import openbackup.database.base.plugin.common.GeneralDbErrorCode;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.util.GeneralDbUtil;
import openbackup.database.base.plugin.util.TestConfHelper;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * GeneralDbResourceProvider测试用例
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-22
 */
@SpringBootTest(classes = {
    GeneralDbResourceProvider.class
})
@RunWith(SpringRunner.class)
public class GeneralDbResourceProviderTest {
    @Autowired
    private GeneralDbResourceProvider generalDbResourceProvider;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private AgentUnifiedService agentUnifiedService;

    @Before
    public void init() {
        String hanaConf = TestConfHelper.getHanaConf();
        AppConf appConf = GeneralDbUtil.getAppConf(hanaConf).orElse(null);
        Map<String, Object> mockMap1 = new HashMap<>();
        mockMap1.put("a", "test");
        mockMap1.put("hana", appConf);
        Map<String, Object> mockMap2 = new HashMap<>();
        mockMap2.put("hana", appConf);
        mockMap2.put("c", "test");
        Mockito.when(agentUnifiedService.queryAppConf(Mockito.any(), Mockito.any(), Mockito.any(), Mockito.any()))
            .thenReturn(mockMap1)
            .thenReturn(mockMap2);
    }

    /**
     * 用例场景：通用数据库查询应用配置
     * 前置条件：无
     * 检查点：查询配置成功, 应该取所有主机的交集。
     */
    @Test
    public void query_app_conf_success() {
        Map<String, Object> map1 = generalDbResourceProvider.queryAppConf(null, null);
        Assert.assertEquals(0, map1.size());

        String[] hostUuids = new String[] {"a", "b", "c"};
        ProtectedEnvironment env1 = new ProtectedEnvironment();
        List<ProtectedResource> protectedResourceList = new ArrayList<>();
        protectedResourceList.add(env1);
        ProtectedEnvironment env2 = new ProtectedEnvironment();
        protectedResourceList.add(env2);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(protectedResourceList);
        Mockito.when(resourceService.query(Mockito.any())).thenReturn(pageListResponse);

        Map<String, Object> confMap = generalDbResourceProvider.queryAppConf(null, hostUuids);

        Assert.assertTrue(confMap.containsKey("hana"));
        Assert.assertFalse(confMap.containsKey("a"));
    }

    /**
     * 用例场景：注册时检查基本参数
     * 前置条件：无
     * 检查点：检查基本参数是否正确：name、script等是否为空
     */
    @Test
    public void check_essential_params_when_save() {
        ProtectedEnvironment singleInstance = TestConfHelper.mockInstance(true);
        singleInstance.setName(null);
        throw_params_error_when_check(singleInstance, "name is empty.");
        singleInstance.setName("name");
        singleInstance.getExtendInfo().remove(GeneralDbConstant.EXTEND_SCRIPT_KEY);
        throw_params_error_when_check(singleInstance, "script is empty.");
        singleInstance.setExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY, TestConfHelper.SAP_HANA);
        singleInstance.getExtendInfo().remove(GeneralDbConstant.EXTEND_FIRST_CLASSIFICATION_KEY);
        throw_params_error_when_check(singleInstance, "first classification is empty");
        singleInstance.setExtendInfoByKey(GeneralDbConstant.EXTEND_FIRST_CLASSIFICATION_KEY,
            GeneralDbConstant.GENERAL_DB_INSTANCE);
        singleInstance.setExtendInfoByKey(GeneralDbConstant.EXTEND_CUSTOM_PARAM, mockLongCustomParams());
        throw_params_error_when_check(singleInstance,
            "custom params length can not exceed " + GeneralDbConstant.EXTEND_CUSTOM_PARAM_LENGTH);
        singleInstance.setExtendInfoByKey(GeneralDbConstant.EXTEND_CUSTOM_PARAM, "");
        throw_params_error_when_check(singleInstance, "only support database now.");

        // 数据库情形
        ProtectedEnvironment singleDb = TestConfHelper.mockDatabase(true);
        singleDb.getExtendInfo().remove(GeneralDbConstant.EXTEND_DEPLOY_TYPE);
        throw_params_error_when_check(singleDb, "deploy type is empty.");
        singleDb.setExtendInfoByKey(GeneralDbConstant.EXTEND_DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        singleDb.getDependencies().get(GeneralDbConstant.DEPENDENCY_HOST_KEY).clear();
        throw_params_error_when_check(singleDb, "host number is wrong.");

        ProtectedEnvironment clusterDb = TestConfHelper.mockDatabase(false);
        clusterDb.getDependencies().get(GeneralDbConstant.DEPENDENCY_HOST_KEY).clear();
        throw_params_error_when_check(clusterDb, "host number is wrong.");
    }

    private void throw_params_error_when_check(ProtectedEnvironment environment, String message) {
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> generalDbResourceProvider.check(environment));
        Assert.assertEquals(exception.getErrorCode(), CommonErrorCode.ERR_PARAM);
        Assert.assertEquals(exception.getMessage(), message);
    }

    private String mockLongCustomParams() {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 501; i++) {
            sb.append("a");
        }
        return sb.toString();
    }

    /**
     * 用例场景：注册时检查资源
     * 前置条件：无
     * 检查点：如果找不到script配置，则抛出异常
     */
    @Test
    public void fail_find_script_conf_when_save() {
        ProtectedEnvironment environment = TestConfHelper.mockDatabase(false);
        environment.setEndpoint(null);
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(new ArrayList<>());
        PageListResponse<ProtectedResource> response2 = new PageListResponse<>();
        response2.setRecords(
            TestConfHelper.mockHost().stream().map(e -> (ProtectedResource) e).collect(Collectors.toList()));
        Mockito.when(resourceService.query(Mockito.any())).thenReturn(response2).thenReturn(response);

        AgentDetailDto agentDetailDto = new AgentDetailDto();
        agentDetailDto.setResourceList(new ArrayList<>());
        AppResource appResource = new AppResource();
        appResource.setExtendInfo(environment.getExtendInfo());
        appResource.getExtendInfo().put(GeneralDbConstant.EXTEND_VERSION_KEY, "1.1");
        agentDetailDto.getResourceList().add(appResource);
        Mockito.when(agentUnifiedService.getDetail(Mockito.any(), Mockito.any(), Mockito.any(), Mockito.any()))
            .thenReturn(agentDetailDto);

        String hanaConf = TestConfHelper.getHanaConf();
        AppConf appConf = GeneralDbUtil.getAppConf(hanaConf).orElse(null);
        Map<String, Object> mockMap1 = new HashMap<>();
        mockMap1.put("a", "test");
        mockMap1.put("hana", appConf);
        Map<String, Object> mockMap2 = new HashMap<>();
        mockMap2.put("d", appConf);
        mockMap2.put("c", "test");
        Mockito.when(agentUnifiedService.queryAppConf(Mockito.any(), Mockito.any(), Mockito.any(), Mockito.any()))
            .thenReturn(mockMap1)
            .thenReturn(mockMap2);

        throw_params_error_when_check(environment, "Can not find conf.");
    }

    /**
     * 用例场景：注册时检查名称是否重复
     * 前置条件：无
     * 检查点：如果存在重复名称，则抛出异常
     */
    @Test
    public void check_name_duplicate_when_save() {
        ProtectedEnvironment environment = TestConfHelper.mockDatabase(true);
        ProtectedEnvironment resource1 = new ProtectedEnvironment();
        resource1.setUuid("host1");
        resource1.setName("host1");
        resource1.setExtendInfoByKey(GeneralDbConstant.EXTEND_RELATED_HOST_IDS, "host1");
        PageListResponse<ProtectedResource> response1 = new PageListResponse<>();
        response1.setRecords(Collections.singletonList(resource1));

        ProtectedEnvironment resource2 = new ProtectedEnvironment();
        resource2.setUuid("11");
        resource2.setName("TEst");
        resource2.setExtendInfoByKey(GeneralDbConstant.EXTEND_RELATED_HOST_IDS, "host1");
        PageListResponse<ProtectedResource> response2 = new PageListResponse<>();
        response2.setRecords(Collections.singletonList(resource2));
        Mockito.when(resourceService.query(Mockito.any())).thenReturn(response1).thenReturn(response2);

        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> generalDbResourceProvider.check(environment));
        Assert.assertEquals(exception.getErrorCode(), GeneralDbErrorCode.GENERAL_DB_DUPLICATE_NAME);
    }

    /**
     * 用例场景：更新时检查名称是否重复
     * 前置条件：无
     * 检查点：如果存在重复名称，则抛出异常
     */
    @Test
    public void check_name_duplicate_when_update() {
        ProtectedEnvironment environment = TestConfHelper.mockInstance(true);
        environment.setUuid("env1");
        ProtectedEnvironment resource1 = new ProtectedEnvironment();
        resource1.setUuid("host1");
        resource1.setName("host1");
        resource1.setExtendInfoByKey(GeneralDbConstant.EXTEND_RELATED_HOST_IDS, "host1");
        PageListResponse<ProtectedResource> response1 = new PageListResponse<>();
        response1.setRecords(Collections.singletonList(resource1));

        ProtectedEnvironment resource2 = new ProtectedEnvironment();
        resource2.setUuid("11");
        resource2.setName("TEst");
        resource2.setExtendInfoByKey(GeneralDbConstant.EXTEND_RELATED_HOST_IDS, "host1");
        PageListResponse<ProtectedResource> response2 = new PageListResponse<>();
        response2.setRecords(Collections.singletonList(resource2));
        Mockito.when(resourceService.query(Mockito.any())).thenReturn(response1).thenReturn(response2);

        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> generalDbResourceProvider.updateCheck(environment));
        Assert.assertEquals(exception.getErrorCode(), GeneralDbErrorCode.GENERAL_DB_DUPLICATE_NAME);
    }

    /**
     * 用例场景：注册时检查资源信息
     * 前置条件：无
     * 检查点：如果找不到agent资源，则报错
     */
    @Test
    public void resource_detail_return_fail() {
        ProtectedEnvironment environment = TestConfHelper.mockDatabase(false);
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(new ArrayList<>());
        PageListResponse<ProtectedResource> response2 = new PageListResponse<>();
        response2.setRecords(
            TestConfHelper.mockHost().stream().map(e -> (ProtectedResource) e).collect(Collectors.toList()));
        Mockito.when(resourceService.query(Mockito.any())).thenReturn(response2).thenReturn(response);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> generalDbResourceProvider.check(environment));
        Assert.assertEquals(exception.getErrorCode(), CommonErrorCode.OPERATION_FAILED);
        Assert.assertEquals(exception.getMessage(), "agent return's resource is empty");
    }

    /**
     * 用例场景：注册时检查资源信息
     * 前置条件：无
     * 检查点：判断发现资源接口是否一直返回shouldNextSupport
     */
    @Test
    public void resource_can_find_next_support() {
        ProtectedEnvironment environment = TestConfHelper.mockDatabase(true);
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(new ArrayList<>());
        PageListResponse<ProtectedResource> response2 = new PageListResponse<>();
        response2.setRecords(
            TestConfHelper.mockHost().stream().map(e -> (ProtectedResource) e).collect(Collectors.toList()));
        Mockito.when(resourceService.query(Mockito.any())).thenReturn(response2).thenReturn(response);

        AgentDetailDto agentDetailDto = new AgentDetailDto();
        agentDetailDto.setResourceList(new ArrayList<>());
        AppResource appResource = new AppResource();
        appResource.setExtendInfo(environment.getExtendInfo());
        appResource.getExtendInfo().put(GeneralDbConstant.EXTEND_SHOULD_NEXT_SUPPORT_KEY, "true");
        agentDetailDto.getResourceList().add(appResource);
        Mockito.when(agentUnifiedService.getDetail(Mockito.any(), Mockito.any(), Mockito.any(), Mockito.any()))
            .thenReturn(agentDetailDto);

        throw_params_error_when_check(environment, "shouldNextSupport is true");
    }

    /**
     * 用例场景：注册时检查资源信息
     * 前置条件：无
     * 检查点：判断传入的script与agent上的是否一致
     */
    @Test
    public void check_script_in_agent_when_save() {
        ProtectedEnvironment environment = TestConfHelper.mockDatabase(true);
        environment.setExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY, "xxxx");
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(new ArrayList<>());
        PageListResponse<ProtectedResource> response2 = new PageListResponse<>();
        response2.setRecords(
            TestConfHelper.mockHost().stream().map(e -> (ProtectedResource) e).collect(Collectors.toList()));
        Mockito.when(resourceService.query(Mockito.any())).thenReturn(response2).thenReturn(response);

        AgentDetailDto agentDetailDto = new AgentDetailDto();
        agentDetailDto.setResourceList(new ArrayList<>());
        AppResource appResource = new AppResource();
        appResource.setExtendInfo(environment.getExtendInfo());
        appResource.getExtendInfo().put(GeneralDbConstant.EXTEND_SHOULD_NEXT_SUPPORT_KEY, "true");
        agentDetailDto.getResourceList().add(appResource);
        Mockito.when(agentUnifiedService.getDetail(Mockito.any(), Mockito.any(), Mockito.any(), Mockito.any()))
            .thenReturn(agentDetailDto);

        throw_params_error_when_check(environment, "the param of script errors.");
    }

    /**
     * 用例场景：注册时检查资源信息
     * 前置条件：无
     * 检查点：判断检查的信息是否保存在extend info中
     */
    @Test
    public void resource_save_check() {
        ProtectedEnvironment environment = TestConfHelper.mockDatabase(false);
        environment.setEndpoint(null);
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(new ArrayList<>());
        PageListResponse<ProtectedResource> response2 = new PageListResponse<>();
        response2.setRecords(
            TestConfHelper.mockHost().stream().map(e -> (ProtectedResource) e).collect(Collectors.toList()));
        Mockito.when(resourceService.query(Mockito.any())).thenReturn(response2).thenReturn(response);

        AgentDetailDto agentDetailDto = new AgentDetailDto();
        agentDetailDto.setResourceList(new ArrayList<>());
        AppResource appResource = new AppResource();
        appResource.setExtendInfo(environment.getExtendInfo());
        appResource.getExtendInfo().put(GeneralDbConstant.EXTEND_VERSION_KEY, "1.1");
        agentDetailDto.getResourceList().add(appResource);
        Mockito.when(agentUnifiedService.getDetail(Mockito.any(), Mockito.any(), Mockito.any(), Mockito.any()))
            .thenReturn(agentDetailDto);

        generalDbResourceProvider.check(environment);

        Assert.assertNotNull(environment.getExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_CONF));
        Assert.assertNotNull(environment.getExtendInfoByKey(GeneralDbConstant.EXTEND_RELATED_HOST_IPS));
        Assert.assertNotNull(environment.getExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY));
        Assert.assertEquals(environment.getLinkStatus(), LinkStatusEnum.ONLINE.getStatus().toString());
        Assert.assertEquals(environment.getVersion(), "1.1");
        Assert.assertFalse(VerifyUtil.isEmpty(environment.getPath()));
    }

    /**
     * 用例场景：更新时检查资源信息
     * 前置条件：无
     * 检查点：判断传入的script是否修改
     */
    @Test
    public void check_script_is_change_when_update() {
        ProtectedEnvironment environment = TestConfHelper.mockDatabase(false);
        environment.setExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY, "aa");
        environment.setName(null);

        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(new ArrayList<>());
        PageListResponse<ProtectedResource> response2 = new PageListResponse<>();
        response2.setRecords(
            TestConfHelper.mockHost().stream().map(e -> (ProtectedResource) e).collect(Collectors.toList()));
        Mockito.when(resourceService.query(Mockito.any())).thenReturn(response2).thenReturn(response);

        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY, TestConfHelper.SAP_HANA);
        Mockito.when(resourceService.getResourceById(Mockito.any())).thenReturn(Optional.of(protectedResource));

        AgentDetailDto agentDetailDto = new AgentDetailDto();
        agentDetailDto.setResourceList(new ArrayList<>());
        AppResource appResource = new AppResource();
        appResource.setExtendInfo(environment.getExtendInfo());
        appResource.getExtendInfo().put(GeneralDbConstant.EXTEND_VERSION_KEY, "1.1");
        agentDetailDto.getResourceList().add(appResource);
        Mockito.when(agentUnifiedService.getDetail(Mockito.any(), Mockito.any(), Mockito.any(), Mockito.any()))
            .thenReturn(agentDetailDto);

        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> generalDbResourceProvider.updateCheck(environment));
        Assert.assertEquals(exception.getErrorCode(), CommonErrorCode.ERR_PARAM);
        Assert.assertEquals(exception.getMessage(), "the param of script errors.");
    }

    /**
     * 用例场景：更新时检查资源信息
     * 前置条件：无
     * 检查点：判断检查的信息是否保存在extend info中
     */
    @Test
    public void resource_update_check() {
        ProtectedEnvironment environment = TestConfHelper.mockInstance(true);
        environment.setName(null);
        List<ProtectedResource> toDeletedHosts = new ArrayList<>();
        ProtectedResource host1 = new ProtectedResource();
        host1.setUuid("11");
        toDeletedHosts.add(host1);
        environment.getDependencies().put("-" + GeneralDbConstant.DEPENDENCY_HOST_KEY, toDeletedHosts);

        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(new ArrayList<>());
        PageListResponse<ProtectedResource> response2 = new PageListResponse<>();
        response2.setRecords(
            TestConfHelper.mockHost().stream().map(e -> (ProtectedResource) e).collect(Collectors.toList()));
        Mockito.when(resourceService.query(Mockito.any())).thenReturn(response2).thenReturn(response);

        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY, TestConfHelper.SAP_HANA);
        Mockito.when(resourceService.getResourceById(Mockito.any())).thenReturn(Optional.of(protectedResource));

        AgentDetailDto agentDetailDto = new AgentDetailDto();
        agentDetailDto.setResourceList(new ArrayList<>());
        AppResource appResource = new AppResource();
        appResource.setExtendInfo(environment.getExtendInfo());
        appResource.getExtendInfo().put(GeneralDbConstant.EXTEND_VERSION_KEY, "1.1");
        agentDetailDto.getResourceList().add(appResource);
        Mockito.when(agentUnifiedService.getDetail(Mockito.any(), Mockito.any(), Mockito.any(), Mockito.any()))
            .thenReturn(agentDetailDto);

        generalDbResourceProvider.updateCheck(environment);

        Assert.assertNotNull(environment.getExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_CONF));
        Assert.assertNotNull(environment.getExtendInfoByKey(GeneralDbConstant.EXTEND_RELATED_HOST_IPS));
        Assert.assertNotNull(environment.getExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY));
        Assert.assertEquals(environment.getLinkStatus(), LinkStatusEnum.ONLINE.getStatus().toString());
        Assert.assertEquals(environment.getVersion(), "1.1");
        Assert.assertTrue(environment.getDependencies().containsKey("-" + GeneralDbConstant.DEPENDENCY_HOST_KEY));
    }
}
