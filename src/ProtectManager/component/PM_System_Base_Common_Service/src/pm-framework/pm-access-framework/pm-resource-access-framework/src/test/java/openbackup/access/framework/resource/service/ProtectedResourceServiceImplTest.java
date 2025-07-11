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

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import openbackup.access.framework.resource.persistence.dao.ProtectedResourceExtendInfoMapper;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceMapper;
import openbackup.access.framework.resource.persistence.model.ProtectedEnvironmentPo;
import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupChangeCauseEnum;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupModifyReq;
import openbackup.data.protection.access.provider.sdk.backup.ResourceExtendInfoConstants;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.NextBackupParams;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceGroupResult;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.VstoreResourceQueryParam;
import openbackup.data.protection.access.provider.sdk.resource.model.AgentTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.model.ResourceExtendInfoKeyConstants;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.enums.UserTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.quota.QuotaService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.sdk.resource.model.UpdateCopyUserObjectReq;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.service.hostagent.AgentQueryService;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;
import openbackup.system.base.util.MessageTemplate;

import com.huawei.oceanprotect.system.base.user.service.ResourceSetResourceService;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetService;
import com.huawei.oceanprotect.system.base.user.service.UserService;
import com.huawei.oceanprotect.system.sdk.dto.SystemSwitchDto;
import com.huawei.oceanprotect.system.sdk.enums.SwitchStatusEnum;
import com.huawei.oceanprotect.system.sdk.service.SystemSwitchInternalService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.test.util.ReflectionTestUtils;

import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Timer;
import java.util.stream.Collectors;

/**
 * 测试类
 *
 */
@RunWith(PowerMockRunner.class)
public class ProtectedResourceServiceImplTest {
    private static final String RESOURCE_ID = "266ea41d-adf5-480b-af50-15b940c2b846";
    ProtectedResourceServiceImpl protectedResourceService;
    @Mock
    ProtectedResourceRepository repository;

    @Mock
    MessageTemplate messageTemplate;

    @Mock
    ProtectedResourceMonitorService protectedResourceMonitorService;

    @Mock
    EncryptorService encryptorService;

    @Mock
    ProtectedResourceMapper protectedResourceMapper;

    @Mock
    ProtectedResourceDecryptService decryptService;

    @Mock
    JobScheduleService jobScheduleService;
    @Mock
    DeployTypeService deployTypeService;

    @Mock
    private SystemSwitchInternalService systemSwitchInternalService;
    @Mock
    private ResourceScanService resourceScanService;

    @Mock
    private SessionService sessionService;

    @Mock
    private AgentQueryService agentQueryService;
    @Mock
    private ResourceSetApi resourceSetApi;

    @Mock
    private CopyRestApi copyRestApi;

    @Mock
    private UserService userService;

    @Mock
    private QuotaService quotaService;

    @Mock
    private CopyManagerService copyManagerService;

    @Mock
    private ResourceSetService resourceSetService;

    @Mock
    private ResourceSetResourceService resourceSetResourceService;

    @Before
    public void prepare() throws Exception {
        protectedResourceService.setDeployTypeService(deployTypeService);
        protectedResourceService.setJobScheduleService(jobScheduleService);
        protectedResourceService.setResourceScanService(resourceScanService);
        systemSwitchInternalService = Mockito.mock(SystemSwitchInternalService.class);
        protectedResourceService.setSystemSwitchInternalService(systemSwitchInternalService);
        protectedResourceService.setAgentQueryService(agentQueryService);
        protectedResourceService.setSessionService(sessionService);
        protectedResourceService.setResourceSetApi(resourceSetApi);
        ReflectionTestUtils.setField(protectedResourceService, "maxResourceNum", 20000);
        SystemSwitchDto systemSwitchDto = new SystemSwitchDto();
        systemSwitchDto.setStatus(SwitchStatusEnum.OFF);
        Mockito.when(systemSwitchInternalService.queryByName(Mockito.any())).thenReturn(systemSwitchDto);

        PowerMockito.when(repository.groupQuery(ArgumentMatchers.any()))
                .thenReturn(mock_paged_protected_resource_group_result());
        PowerMockito.when(repository.groupQuery(ArgumentMatchers.any()))
                .thenReturn(mock_paged_protected_resource_group_result());
    }

    /**
     * 用例场景：创建资源
     * 前置条件：无
     * 检查点： 创建资源时，执行ResourceProvider.check方法。
     */
    @Test
    public void create_resource_success() {
        BasePage<ProtectedResourcePo> page = new BasePage<>();
        ProtectedResourcePo resource = new ProtectedResourcePo();
        resource.setUuid("11");
        resource.setCreatedTime(new Timestamp(System.currentTimeMillis()));
        page.setItems(Collections.singletonList(resource));
        Mockito.when(repository.query(any())).thenReturn(page);

        String uuid = "11";
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(uuid);
        protectedResource.setRootUuid("rootUuid");

        ResourceProvider resourceProvider = Mockito.mock(ResourceProvider.class);
        ProviderManager providerManager = Mockito.mock(ProviderManager.class);
        protectedResourceService.setProviderManager(providerManager);
        Mockito.when(providerManager.findProvider(Mockito.eq(ResourceProvider.class), Mockito.any(), Mockito.eq(null)))
            .thenReturn(resourceProvider);
        Mockito.when(protectedResourceMonitorService.invoke(Mockito.any(), Mockito.any(), Mockito.any()))
            .thenReturn(uuid);
        Mockito.when(agentQueryService.querySharedAgentIds()).thenReturn(new ArrayList<>());
        Mockito.when(sessionService.getCurrentUser()).thenReturn(null);
        String[] ids = protectedResourceService.create(new ProtectedResource[] {protectedResource}, false);

        Assert.assertEquals(ids[0], uuid);
        Mockito.verify(resourceProvider, Mockito.times(1)).check(protectedResource);
    }

    /**
     * 用例场景：删除资源
     * 前置条件：无
     * 检查点： 删除资源时，同时删除ResourceProvider中preHandleDelete中的资源。
     */
    @Test
    public void delete_resource_success_when_pre_handle_delete() {
        BasePage<ProtectedResourcePo> page = new BasePage<>();
        ProtectedResourcePo resource = new ProtectedResourcePo();
        resource.setUuid("11");
        resource.setCreatedTime(new Timestamp(System.currentTimeMillis()));
        page.setItems(Collections.singletonList(resource));
        Mockito.when(repository.query(any())).thenReturn(page);

        ProviderManager providerManager = Mockito.mock(ProviderManager.class);
        protectedResourceService.setProviderManager(providerManager);
        ResourceProvider resourceProvider = Mockito.mock(ResourceProvider.class);
        Mockito.when(providerManager.findProvider(Mockito.eq(ResourceProvider.class), Mockito.any(), Mockito.eq(null)))
            .thenReturn(resourceProvider);
        ResourceDeleteContext resourceDeleteContext = new ResourceDeleteContext();
        ResourceDeleteContext.ResourceDeleteDependency resourceDeleteDependency
                = new ResourceDeleteContext.ResourceDeleteDependency();
        resourceDeleteDependency.setDeleteIds(Arrays.asList("22", "33"));
        resourceDeleteContext.setResourceDeleteDependencyList(Collections.singletonList(resourceDeleteDependency));
        Mockito.when(resourceProvider.preHandleDelete(Mockito.any())).thenReturn(resourceDeleteContext);

        ProtectedResourceExtendInfoMapper protectedResourceExtendInfoMapper = Mockito.mock(
                ProtectedResourceExtendInfoMapper.class);
        protectedResourceService.setResourceExtendInfoMapper(protectedResourceExtendInfoMapper);
        Mockito.when(protectedResourceExtendInfoMapper.selectCount(Mockito.any())).thenReturn(0L);

        protectedResourceService.delete(new String[] {"11"});

        ResourceDeleteParams params = new ResourceDeleteParams();
        params.setForce(false);
        params.setShouldDeleteRegister(true);
        params.setResources(new String[] {"22", "33", "11"});
        Mockito.verify(repository, Mockito.times(1)).delete(params);
    }

    /**
     * 用例场景：分页查询resource
     * 前置条件：查询第1页的resource
     * 检查点： 确定返回的resource中确定填充了environment字段。
     */
    @Test
    public void query_resource_success() {
        PowerMockito.when(repository.query(any()))
                .thenReturn(mock_paged_resources()).thenReturn(mock_paged_environments());
        PageListResponse<ProtectedResource> pagedResources =
                protectedResourceService.query(false, 1, 20, new HashMap<>());
        Assert.assertNotNull(pagedResources.getRecords().get(0).getEnvironment());
    }


    /**
     * 用例场景：根据key分组分页查询resource
     * 前置条件：查询第1页的resource
     * 检查点： 确定返回的列表符合按照key属性对应的value值分组，确定填充了key关联的资源
     */
    @Test
    public void group_query_resource_success() {
        VstoreResourceQueryParam param = VstoreResourceQueryParam.builder()
            .page(1)
            .size(20)
            .isSearchProtectObject(false)
            .key("tenantId")
            .order("-")
            .build();
        PageListResponse<ProtectedResourceGroupResult> pagedResources =
                protectedResourceService.groupQueryByExtendInfo(param);
        Assert.assertNotNull(pagedResources.getRecords().get(0).getResources());
    }


    private BasePage<ProtectedResourcePo> mock_paged_resources() {
        List<ProtectedResourcePo> resources = new ArrayList<>();

        for (int i = 0; i < 20; i++) {
            resources.add(buildOneResource());
        }

        return BasePage.create(resources);
    }

    private ProtectedResourcePo buildOneResource() {
        ProtectedResourcePo ret = ProtectedResourcePo.fromProtectedResource(new ProtectedResource());
        ret.setUuid("123");
        ret.setRootUuid("456");
        ret.setCreatedTime(new Timestamp(123456));
        return ret;
    }

    private BasePage<ProtectedResourcePo> mock_paged_environments() {
        List<ProtectedResourcePo> environmentPos = new ArrayList<>();

        for (int i = 0; i < 1; i++) {
            environmentPos.add(buildOneEnv());
        }

        return BasePage.create(environmentPos);
    }

    private ProtectedEnvironmentPo buildOneEnv() {
        ProtectedEnvironmentPo ret = new ProtectedEnvironmentPo();
        ret.setUuid("456");
        ret.setRootUuid("456");
        ret.setCreatedTime(new Timestamp(123456));
        return ret;
    }

    private BasePage<ProtectedResourceGroupResult> mock_paged_protected_resource_group_result() {
        List<ProtectedResourceGroupResult> protectedResourceGroupResults = new ArrayList<>();

        for (int i = 0; i < 2; i++) {
            protectedResourceGroupResults.add(buildOneGrop());
        }

        return BasePage.create(protectedResourceGroupResults);
    }

    private ProtectedResourceGroupResult buildOneGrop() {
        ProtectedResourceGroupResult groupResult = new ProtectedResourceGroupResult();
        groupResult.setKey("tenantId");
        groupResult.setValue("001");
        groupResult.setResources(buildResourceList());
        return groupResult;
    }

    private List<ProtectedResource> buildResourceList() {
        List<ProtectedResourcePo> protectedResourceList = new ArrayList<>();

        for (int i = 0; i < 3; i++) {
            protectedResourceList.add(buildOneResource());
        }
        return protectedResourceList.stream().map(ProtectedResourcePo::toProtectedResource).collect(Collectors.toList());
    }

    @Test
    public void test_update_source_type() {
        protectedResourceService.setDeployTypeService(deployTypeService);
        protectedResourceService.setJobScheduleService(jobScheduleService);
        protectedResourceService.setResourceScanService(resourceScanService);

        protectedResourceService.updateSourceType(Collections.emptyList(), "autoscan");
        verify(protectedResourceMapper, times(0)).update(any(), any());
        protectedResourceService.updateSourceType(Collections.singletonList("uuid"), "autoscan");
        verify(protectedResourceMapper, times(1)).update(any(), any());
    }

    @Test
    public void test_update_sub_source() {
        protectedResourceService.setDeployTypeService(deployTypeService);
        protectedResourceService.setJobScheduleService(jobScheduleService);
        protectedResourceService.setResourceScanService(resourceScanService);
        protectedResourceService.updateSubResource(Collections.emptyList(), Collections.emptyMap());
        verify(protectedResourceMapper, times(0)).update(any(), any());
        Map<String, Object> updateKv = new HashMap<>();
        updateKv.put("PARENT_NAME", "testName");
        protectedResourceService.updateSubResource(Collections.singletonList("uuid"), updateKv);
        verify(protectedResourceMapper, times(1)).update(any(), any());
    }

    /**
     * 用例场景：资源扫描provider出错时，框架抛出异常
     * 前置条件：无
     * 检查点： 资源扫描provider出错时，框架抛出异常
     */
    @Test
    public void scan_resource_throw_exception_when_provider_occur_error() {
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("11");
        BasePage<ProtectedResourcePo> basePage = new BasePage<>();
        ProtectedResourcePo protectedResourcePo = new ProtectedResourcePo();
        basePage.setItems(Collections.singletonList(protectedResourcePo));
        protectedResourcePo.setCreatedTime(new Timestamp(System.currentTimeMillis()));

        PowerMockito.when(repository.query(any())).thenReturn(basePage);

        ResourceProvider resourceProvider = Mockito.mock(ResourceProvider.class);
        ProviderManager providerManager = Mockito.mock(ProviderManager.class);
        protectedResourceService.setProviderManager(providerManager);
        Mockito.when(providerManager.findProvider(Mockito.eq(ResourceProvider.class), Mockito.any(), Mockito.eq(null)))
            .thenReturn(resourceProvider);
        Mockito.when(resourceProvider.scan(resource)).thenThrow(new LegoCheckedException(123, "error"));

        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> protectedResourceService.executeScanProtectedResource(resource));
        Assert.assertEquals(legoCheckedException.getErrorCode(), 123);
    }

    /**
     * 用例场景：检查资源是否受信
     * 前置条件：无
     * 检查点： 通用代理资源扩展属性为true，或者为内置代理时检查通过，否则异常
     */
    @Test
    public void check_trust_host() {
        SystemSwitchDto systemSwitchDto = new SystemSwitchDto();
        systemSwitchDto.setStatus(SwitchStatusEnum.ON);
        Mockito.when(systemSwitchInternalService.queryByName(Mockito.any())).thenReturn(systemSwitchDto);

        List<ProtectedEnvironment> resources = new ArrayList<>();
        // resource1 其他类型资源
        ProtectedEnvironment resource1 = new ProtectedEnvironment();
        resource1.setType("testType");
        resource1.setSubType("testSubType");
        resources.add(resource1);
        protectedResourceService.checkHostIfBeTrusted(resources);

        // resource1 resource2  UBackupAgent扩展字段无值
        ProtectedEnvironment resource2 = new ProtectedEnvironment();
        resource2.setType(ResourceTypeEnum.HOST.getType());
        resource2.setSubType(ResourceSubTypeEnum.U_BACKUP_AGENT.getType());
        resources.add(resource2);
        LegoCheckedException exception1 = Assert.assertThrows(LegoCheckedException.class,
            () -> protectedResourceService.checkHostIfBeTrusted(resources));
        Assert.assertEquals(exception1.getErrorCode(), CommonErrorCode.RESOURCE_NOT_TRUST);

        // resource1 resource3  UBackupAgent 内置代理
        resources.remove(resource2);
        ProtectedEnvironment resource3 = new ProtectedEnvironment();
        resource3.setType(ResourceTypeEnum.HOST.getType());
        resource3.setSubType(ResourceSubTypeEnum.U_BACKUP_AGENT.getType());
        Map<String, String> extendInfo3 = new HashMap<>();
        extendInfo3.put(ResourceExtendInfoKeyConstants.EXT_INFO_SCENARIO, AgentTypeEnum.INTERNAL_AGENT.getValue());
        resource3.setExtendInfo(extendInfo3);
        resources.add(resource3);
        protectedResourceService.checkHostIfBeTrusted(resources);

        // resource1 resource4  UBackupAgent 扩展字段为true
        resources.remove(resource3);
        ProtectedEnvironment resource4 = new ProtectedEnvironment();
        resource4.setType(ResourceTypeEnum.HOST.getType());
        resource4.setSubType(ResourceSubTypeEnum.U_BACKUP_AGENT.getType());
        Map<String, String> extendInfo4 = new HashMap<>();
        extendInfo4.put(ResourceExtendInfoKeyConstants.TRUSTWORTHINESS, "true");
        resource4.setExtendInfo(extendInfo4);
        resources.add(resource4);
        protectedResourceService.checkHostIfBeTrusted(resources);
    }


    /**
     * 用例名称：验证资源注册时，资源总数是否超出系统最大规格。<br/>
     * 前置条件：无。<br/>
     * check点：超过抛异常<br/>
     */
    @Test
    public void test_whether_resource_exceeds_upper_limit() {
        BasePage<ProtectedResourcePo> page = new BasePage<>();
        ProtectedResourcePo resource = new ProtectedResourcePo();
        resource.setUuid("11");
        resource.setCreatedTime(new Timestamp(System.currentTimeMillis()));
        page.setItems(Collections.singletonList(resource));
        Mockito.when(repository.query(any())).thenReturn(page);

        String uuid = "11";
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(uuid);
        protectedResource.setRootUuid("rootUuid");

        ResourceProvider resourceProvider = Mockito.mock(ResourceProvider.class);
        ProviderManager providerManager = Mockito.mock(ProviderManager.class);
        protectedResourceService.setProviderManager(providerManager);
        Mockito.when(providerManager.findProvider(Mockito.eq(ResourceProvider.class), Mockito.any(), Mockito.eq(null)))
                .thenReturn(resourceProvider);
        Mockito.when(protectedResourceMonitorService.invoke(Mockito.any(), Mockito.any(), Mockito.any()))
                .thenReturn(uuid);
        Mockito.when(protectedResourceMapper.selectCount(Mockito.any())).thenReturn(20002L);

        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> protectedResourceService.create(new ProtectedResource[]{protectedResource}, false));
        Assert.assertEquals(exception.getErrorCode(), CommonErrorCode.RESOURCE_NUM_EXCEED_LIMIT);
        Assert.assertEquals(exception.getMessage(),
                "Do not add resources, the number of resources exceeds the upper limit.");
    }

    /**
     * 用例场景：修改下次备份类型和原因
     * 前置条件：下次备份类型和原因参数设置正确
     * 检查点：无异常抛出
     */
    @Test
    public void test_modify_next_backup_success() {
        NextBackupModifyReq nextBackupModifyReq = NextBackupModifyReq.build(UUIDGenerator.getUUID(),
                NextBackupChangeCauseEnum.RESTORE_SUCCESS_TO_FULL);
        protectedResourceService.modifyNextBackup(nextBackupModifyReq, false);
        Mockito.verify(repository, Mockito.times(1)).update(any(ProtectedResource.class), anyBoolean());
    }

    /**
     * 用例场景：清除下次备份类型和原因
     * 前置条件：资源id正常
     * 检查点：无异常抛出
     */
    @Test
    public void test_clean_next_backup_success() {
        protectedResourceService.cleanNextBackup(RESOURCE_ID);
        Mockito.verify(repository, Mockito.times(1)).update(any(ProtectedResource.class), anyBoolean());
    }

    /**
     * 用例场景：查询下次备份类型和原因
     * 前置条件：资源id传参
     * 检查点：返回参数成功
     */
    @Test
    public void test_query_next_backup_type_and_cause_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(RESOURCE_ID);
        Map<String, String> extParams = new HashMap<>();
        extParams.put(ResourceExtendInfoConstants.NEXT_BACKUP_CHANGE_CAUSE_EXT_KEY,
                NextBackupChangeCauseEnum.RESTORE_SUCCESS_TO_FULL.getLabel());
        extParams.put(ResourceExtendInfoConstants.NEXT_BACKUP_TYPE_EXT_KEY, BackupTypeEnum.FULL.lower());
        protectedResource.setExtendInfo(extParams);
        ProtectedResourceServiceImpl spy = Mockito.spy(protectedResourceService);
        Mockito.when(spy.getBasicResourceById(ArgumentMatchers.any())).thenReturn(Optional.of(protectedResource));
        NextBackupParams nextBackupParams = spy.queryNextBackupTypeAndCause(RESOURCE_ID);
        Assert.assertEquals(nextBackupParams.getNextBackupChangeCause(),
                NextBackupChangeCauseEnum.RESTORE_SUCCESS_TO_FULL.getLabel());
        Assert.assertEquals(nextBackupParams.getNextBackupType(),
                BackupTypeEnum.FULL.lower());
    }

    @Test
    public void test_updatePluginResourceUserId() {
        Mockito.doNothing().when(protectedResourceMapper).updatePluginResourceUserId(anyString());
        protectedResourceService.updatePluginResourceUserId("");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：创建资源集关联成功
     * 前置条件：无
     * 检查点：执行资源集关联方法一次
     */
    @Test
    public void test_create_resource_set_relation() {
        ProtectedResource protectedResource = new ProtectedResource();
        ReflectionTestUtils.invokeMethod(protectedResourceService, "createResourceSetRelation", protectedResource);
        verify(resourceSetApi, times(1)).addResourceSetRelation(any());
    }

    /**
     * 用例场景：创建timer对象成功
     * 前置条件：无
     * 检查点：计时器创建正常
     */
    @Test
    public void test_create_timer() throws Exception {
        List<JobBo> jobBos = new ArrayList<>();
        JobBo jobBo = new JobBo();
        jobBo.setJobId("12");
        jobBos.add(jobBo);
        PowerMockito.when(resourceScanService.queryManualScanRunningJobByResId("21D123")).thenReturn(jobBos);
        Timer timer = Whitebox.invokeMethod(protectedResourceService, "setTimer", "21D123");
        verify(resourceScanService, times(1)).queryManualScanRunningJobByResId("21D123");
    }

    /**
     * 用例场景：切换用户成功
     * 前置条件：无
     * 检查点：无报错
     */
    @Test
    public void test_update_copy_user() {
        UpdateCopyUserObjectReq updateCopyUserObjectReq = new UpdateCopyUserObjectReq();
        updateCopyUserObjectReq.setResourceId("1212");
        updateCopyUserObjectReq.setUserId("12");
        UserInnerResponse userInfo = new UserInnerResponse();
        userInfo.setUserType(UserTypeEnum.HCS.getValue());
        userInfo.setUserId("121");
        Copy copy1 = new Copy();
        copy1.setUuid("11212");
        copy1.setProperties(
            "\"{\\\"snapshots\\\":[{\\\"id\\\":\\\"1200@Snapshot_37bc5e1f_7d77_48a0_a46d_f0490c02387c\\\",\\\""
                + "parentName\\\":\\\"Rep_131163_Huaage_Fileset_d1cb57ad584949e192634c43da5758bb_su0_0_0\\\"}],"
                + "\\\"dataAfterReduction\\\":194536,\\\"dataBeforeReduction\\\":3855364,\\\"dataPathSuffix\\\":"
                + "\\\"\\\",\\\"fsRelations\\\":{\\\"relations\\\":[{\\\"newEsn\\\":\\\"2102353GTJ10M1000002\\\",\\"
                + "\"newFsId\\\":\\\"1200\\\",\\\"newFsName\\\":\\\"Rep_131163_Huaage_Fileset_d1cb57ad584949e192634"
                + "c43da5758bb_su0_0_0\\\",\\\"oldEsn\\\":\\\"2102353GTJ10M1000002\\\",\\\"oldFsId\\\":\\\"1196\\\","
                + "\\\"oldFsName\\\":\\\"Fileset_d1cb57ad584949e192634c43da5758bb_su0\\\",\\\"role\\\":0}]},\\\"isAgg"
                + "regation\\\":\\\"false\\\",\\\"isSanClient\\\":\\\"false\\\",\\\"maxSizeAfterAggregate\\\":\\\"0\\\""
                + ",\\\"maxSizeToAggregate\\\":\\\"0\\\",\\\"metaPathSuffix\\\":\\\"\\\",\\\"multiFileSystem\\\":\\\"fa"
                + "lse\\\",\\\"originCopyTimeStamp\\\":1732951109,\\\"repositories\\\":[{\\\"id\\\":\\\"2102353GTJ10M1"
                + "000002\\\",\\\"type\\\":2,\\\"protocol\\\":5,\\\"role\\\":0,\\\"remotePath\\\":[{\\\"type\\\":1,\\"
                + "\"path\\\":\\\"/Fileset_CacheDataRepository/d1cb57ad584949e192634c43da5758bb\\\",\\\"id\\\":\\\""
                + "996\\\",\\\"parentId\\\":\\\"\\\"}],\\\"extendInfo\\\":{\\\"capacityAvailable\\\":true,\\\"esn\\"
                + "\":\\\"2102353GTJ10M1000002\\\",\\\"storage_info\\\":{\\\"storage_device\\\":\\\"2102353GTJ10M1000"
                + "002\\\",\\\"storage_pool\\\":\\\"0\\\"}}},{\\\"id\\\":\\\"2102353GTJ10M1000002\\\",\\\"type\\\":1"
                + ",\\\"protocol\\\":5,\\\"role\\\":0,\\\"remotePath\\\":[{\\\"type\\\":0,\\\"path\\\":\\\"/Rep_1311"
                + "63_Huaage_Fileset_d1cb57ad584949e192634c43da5758bb_su0_0_0/source_policy_d1cb57ad584949e192634c43d"
                + "a5758bb_Context_Global_MD\\\",\\\"id\\\":\\\"1200\\\",\\\"parentId\\\":\\\"\\\"},{\\\"type\\\":1,"
                + "\\\"path\\\":\\\"/Rep_131163_Huaage_Fileset_d1cb57ad584949e192634c43da5758bb_su0_0_0/source_policy"
                + "_d1cb57ad584949e192634c43da5758bb_Context\\\",\\\"id\\\":\\\"1200\\\",\\\"parentId\\\":\\\"\\\"}]"
                + ",\\\"extendInfo\\\":{\\\"capacityAvailable\\\":true,\\\"copy_format\\\":0,\\\"esn\\\":\\\"2102353G"
                + "TJ10M1000002\\\",\\\"fsId\\\":\\\"1196\\\",\\\"storage_info\\\":{\\\"storage_device\\\":\\\""
                + "\\\",\\\"storage_pool\\\":\\\"0\\\"}}}],\\\"extendInfo\\\":{\\\"dataAfterReduction\\\":194536,\\\""
                + "dataBeforeReduction\\\":3855364,\\\"dataPathSuffix\\\":\\\"\\\",\\\"fsRelations\\\":{\\\"relation"
                + "s\\\":[{\\\"newEsn\\\":\\\"2102353GTJ10M1000002\\\",\\\"newFsId\\\":\\\"1200\\\",\\\"newFsName\\\""
                + ":\\\"Rep_131163_Huaage_Fileset_d1cb57ad584949e192634c43da5758bb_su0_0_0\\\",\\\"oldEsn\\\":\\\"2"
                + "102353GTJ10M1000002\\\",\\\"oldFsId\\\":\\\"1196\\\",\\\"oldFsName\\\":\\\"Fileset_d1cb57ad584949"
                + "e192634c43da5758bb_su0\\\",\\\"role\\\":0}]},\\\"isAggregation\\\":\\\"false\\\",\\\"isSanClient\\"
                + "\":\\\"false\\\",\\\"maxSizeAfterAggregate\\\":\\\"0\\\",\\\"maxSizeToAggregate\\\":\\\"0\\\",\\\""
                + "metaPathSuffix\\\":\\\"\\\",\\\"multiFileSystem\\\":\\\"false\\\",\\\"originCopyTimeStamp\\\":173"
                + "2951109},\\\"format\\\":0,\\\"verifyStatus\\\":\\\"3\\\",\\\"size\\\":976562,\\\"backup_id\\\":\\\""
                + "46154c63-40b6-4d54-8703-0b6cd121d3ba\\\",\\\"replicate_count\\\":2,\\\"labelList\\\":[]}\",\n");
        copy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());

        Copy copy2 = new Copy();
        copy1.setUuid("222211212");
        copy1.setProperties(
            "\"{\\\"snapshots\\\":[{\\\"id\\\":\\\"1200@Snapshot_37bc5e1f_7d77_48a0_a46d_f0490c02387c\\\",\\\""
                + "parentName\\\":\\\"Rep_131163_Huaage_Fileset_d1cb57ad584949e192634c43da5758bb_su0_0_0\\\"}],"
                + "\\\"dataAfterReduction\\\":194536,\\\"dataBeforeReduction\\\":3855364,\\\"dataPathSuffix\\\":"
                + "\\\"\\\",\\\"fsRelations\\\":{\\\"relations\\\":[{\\\"newEsn\\\":\\\"2102353GTJ10M1000002\\\",\\"
                + "\"newFsId\\\":\\\"1200\\\",\\\"newFsName\\\":\\\"Rep_131163_Huaage_Fileset_d1cb57ad584949e192634"
                + "c43da5758bb_su0_0_0\\\",\\\"oldEsn\\\":\\\"2102353GTJ10M1000002\\\",\\\"oldFsId\\\":\\\"1196\\\","
                + "\\\"oldFsName\\\":\\\"Fileset_d1cb57ad584949e192634c43da5758bb_su0\\\",\\\"role\\\":0}]},\\\"isAgg"
                + "regation\\\":\\\"false\\\",\\\"isSanClient\\\":\\\"false\\\",\\\"maxSizeAfterAggregate\\\":\\\"0\\\""
                + ",\\\"maxSizeToAggregate\\\":\\\"0\\\",\\\"metaPathSuffix\\\":\\\"\\\",\\\"multiFileSystem\\\":\\\"fa"
                + "lse\\\",\\\"originCopyTimeStamp\\\":1732951109,\\\"repositories\\\":[{\\\"id\\\":\\\"2102353GTJ10M1"
                + "000002\\\",\\\"type\\\":2,\\\"protocol\\\":5,\\\"role\\\":0,\\\"remotePath\\\":[{\\\"type\\\":1,\\"
                + "\"path\\\":\\\"/Fileset_CacheDataRepository/d1cb57ad584949e192634c43da5758bb\\\",\\\"id\\\":\\\""
                + "996\\\",\\\"parentId\\\":\\\"\\\"}],\\\"extendInfo\\\":{\\\"capacityAvailable\\\":true,\\\"esn\\"
                + "\":\\\"2102353GTJ10M1000002\\\",\\\"storage_info\\\":{\\\"storage_device\\\":\\\"2102353GTJ10M1000"
                + "002\\\",\\\"storage_pool\\\":\\\"0\\\"}}},{\\\"id\\\":\\\"2102353GTJ10M1000002\\\",\\\"type\\\":1"
                + ",\\\"protocol\\\":5,\\\"role\\\":0,\\\"remotePath\\\":[{\\\"type\\\":0,\\\"path\\\":\\\"/Rep_1311"
                + "63_Huaage_Fileset_d1cb57ad584949e192634c43da5758bb_su0_0_0/source_policy_d1cb57ad584949e192634c43d"
                + "a5758bb_Context_Global_MD\\\",\\\"id\\\":\\\"1200\\\",\\\"parentId\\\":\\\"\\\"},{\\\"type\\\":1,"
                + "\\\"path\\\":\\\"/Rep_131163_Huaage_Fileset_d1cb57ad584949e192634c43da5758bb_su0_0_0/source_policy"
                + "_d1cb57ad584949e192634c43da5758bb_Context\\\",\\\"id\\\":\\\"1200\\\",\\\"parentId\\\":\\\"\\\"}]"
                + ",\\\"extendInfo\\\":{\\\"capacityAvailable\\\":true,\\\"copy_format\\\":0,\\\"esn\\\":\\\"2102353G"
                + "TJ10M1000002\\\",\\\"fsId\\\":\\\"1196\\\",\\\"storage_info\\\":{\\\"storage_device\\\":\\\""
                + "\\\",\\\"storage_pool\\\":\\\"0\\\"}}}],\\\"extendInfo\\\":{\\\"dataAfterReduction\\\":194536,\\\""
                + "dataBeforeReduction\\\":3855364,\\\"dataPathSuffix\\\":\\\"\\\",\\\"fsRelations\\\":{\\\"relation"
                + "s\\\":[{\\\"newEsn\\\":\\\"2102353GTJ10M1000002\\\",\\\"newFsId\\\":\\\"1200\\\",\\\"newFsName\\\""
                + ":\\\"Rep_131163_Huaage_Fileset_d1cb57ad584949e192634c43da5758bb_su0_0_0\\\",\\\"oldEsn\\\":\\\"2"
                + "102353GTJ10M1000002\\\",\\\"oldFsId\\\":\\\"1196\\\",\\\"oldFsName\\\":\\\"Fileset_d1cb57ad584949"
                + "e192634c43da5758bb_su0\\\",\\\"role\\\":0}]},\\\"isAggregation\\\":\\\"false\\\",\\\"isSanClient\\"
                + "\":\\\"false\\\",\\\"maxSizeAfterAggregate\\\":\\\"0\\\",\\\"maxSizeToAggregate\\\":\\\"0\\\",\\\""
                + "metaPathSuffix\\\":\\\"\\\",\\\"multiFileSystem\\\":\\\"false\\\",\\\"originCopyTimeStamp\\\":173"
                + "2951109},\\\"format\\\":0,\\\"verifyStatus\\\":\\\"3\\\",\\\"size\\\":976562,\\\"backup_id\\\":\\\""
                + "46154c63-40b6-4d54-8703-0b6cd121d3ba\\\",\\\"replicate_count\\\":2,\\\"labelList\\\":[]}\",\n");
        copy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        List<Copy> copies = Arrays.asList(copy1, copy2);
        Mockito.when(userService.getUserInfoByUserId(Mockito.anyString())).thenReturn(userInfo);
        Mockito.when(userService.isDPAdminAndCustomizedUser(Mockito.anyString())).thenReturn(true);
        Mockito.when(copyRestApi.queryCopiesByResourceIdAndGeneratedBy(Mockito.anyString(), Mockito.any()))
            .thenReturn(copies);
        Mockito.when(
                quotaService.checkIsUserHasEnoughQuota(Mockito.anyString(), Mockito.anyList(), Mockito.anyString()))
            .thenReturn(true);

        protectedResourceService.updateCopyUser(updateCopyUserObjectReq);
    }
}
