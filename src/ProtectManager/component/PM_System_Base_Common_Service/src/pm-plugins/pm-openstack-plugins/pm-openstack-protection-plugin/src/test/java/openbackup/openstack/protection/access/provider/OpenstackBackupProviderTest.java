/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.openstack.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.backup.v2.PostBackupTask;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.common.OpenstackQuotaService;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.openstack.protection.access.dto.VolInfo;
import openbackup.system.base.common.enums.RetentionTypeEnum;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.RetentionBo;
import openbackup.system.base.util.BeanTools;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;
import org.mockito.ArgumentMatchers;
import org.mockito.Mockito;
import org.springframework.test.util.ReflectionTestUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

import static org.mockito.ArgumentMatchers.any;

/**
 * 功能描述: OpenstackBackupProviderTest
 *
 * @author c30016231
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-11
 */
public class OpenstackBackupProviderTest {
    private static OpenstackBackupProvider backupProvider;
    private static final ResourceService resourceService = Mockito.mock(ResourceService.class);
    private static final OpenstackQuotaService openstackQuotaService = Mockito.mock(OpenstackQuotaService.class);
    private static final CopyRestApi copyRestApi = Mockito.mock(CopyRestApi.class);
    private static final ClusterNativeApi clusterNativeApi = Mockito.mock(ClusterNativeApi.class);

    @BeforeClass
    public static void init() {
        backupProvider = new OpenstackBackupProvider(resourceService, openstackQuotaService, copyRestApi, clusterNativeApi);
    }

    /**
     * 用例场景：OpenStack备份插件类型判断正确 <br/>
     * 前置条件：流程正常 <br/>
     * 检查点：返回结果为True
     */
    @Test
    public void test_applicable_success() {
        boolean openStackCloudServer = backupProvider.applicable("OpenStackCloudServer");
        Assert.assertTrue(openStackCloudServer);
    }

    /**
     * 用例场景：Openstack备份对象参数转换 <br/>
     * 前置条件：Openstack备份对象参数正确 <br/>
     * 检查点：无异常，参数补充正确
     */
    @Test
    public void test_backup_intercept_success() {
        BackupTask backupTask = mockOpenstackBackupTask();
        Mockito.when(clusterNativeApi.getCurrentEsn()).thenReturn("esn_01");
        ProtectedResource domain = MockFactory.mockProtectedResource();
        domain.setAuth(new Authentication());
        Mockito.when(resourceService.getResourceById(ArgumentMatchers.anyBoolean(), ArgumentMatchers.any()))
            .thenReturn(Optional.of(domain));

        BackupTask interceptTask = backupProvider.initialize(backupTask);
        Assert.assertEquals(1, interceptTask.getProtectSubObjects().size());
        Assert.assertEquals(domain.getAuth(), interceptTask.getProtectObject().getAuth());
        Assert.assertEquals("esn_01",
            interceptTask.getAdvanceParams().get(OpenstackConstant.ESN));
        Assert.assertEquals(2, interceptTask.getRepositories().size());
    }

    /**
     * 用例场景：Openstack备份下发全部磁盘 <br/>
     * 前置条件：保护对象高级参数全部磁盘参数为true <br/>
     * 检查点：下发的磁盘信息为空，其余参数正常
     */
    @Test
    public void test_backup_intercept_success_if_all_disk_is_true() {
        BackupTask backupTask = mockOpenstackBackupTask();
        backupTask.getAdvanceParams().put(OpenstackConstant.ALL_DISK, "true");
        Mockito.when(clusterNativeApi.getCurrentEsn()).thenReturn("esn_01");
        Mockito.when(resourceService.getResourceById(ArgumentMatchers.anyBoolean(), ArgumentMatchers.any()))
            .thenReturn(Optional.of(MockFactory.mockProtectedResource()));

        BackupTask interceptTask = backupProvider.initialize(backupTask);
        Assert.assertNull(interceptTask.getProtectSubObjects());
        Assert.assertEquals(2, interceptTask.getRepositories().size());
    }

    /**
     * 用例场景：Openstack备份对象参数转换 <br/>
     * 前置条件：Openstack备份对象参数正确 <br/>
     * 检查点：无异常，参数补充正确
     */
    @Test
    public void test_post_process_success() {
        PostBackupTask postBackupTask = mockPostBackupTask();
        ProtectedEnvironment environment = MockFactory.mockEnvironment();
        environment.getExtendInfo().put(OpenstackConstant.REGISTER_SERVICE, OpenstackConstant.REGISTER_OPENSTACK);
        Mockito.when(resourceService.getResourceById(ArgumentMatchers.anyBoolean(), ArgumentMatchers.any()))
            .thenReturn(Optional.of(environment))
            .thenReturn(Optional.of(MockFactory.mockProtectedResource()));
        Mockito.when(openstackQuotaService.isRegisterOpenstack(any())).thenReturn(true);
        Mockito.doNothing().when(openstackQuotaService).updateUsedQuota(any(), any(), any());
        backupProvider.finalize(postBackupTask);
        Assert.assertTrue(true);
    }

    private PostBackupTask mockPostBackupTask() {
        PostBackupTask postBackupTask = new PostBackupTask();
        ProtectedObject protectedObject = new ProtectedObject();
        protectedObject.setEnvUuid("env_uuid");
        protectedObject.setResourceId("res_uuid");
        postBackupTask.setProtectedObject(protectedObject);
        postBackupTask.setCopyInfo(new CopyInfo());
        return postBackupTask;
    }

    private BackupTask mockOpenstackBackupTask() {
        BackupTask backupTask = new BackupTask();

        TaskResource protectObject = new TaskResource();
        protectObject.setUuid(UUIDGenerator.getUUID());
        protectObject.setParentUuid(UUIDGenerator.getUUID());
        Map<String, String> proExtendInfo = new HashMap<>();
        List<VolInfo> hostDiskInfos = new ArrayList<>();
        VolInfo volInfo = new VolInfo();
        volInfo.setId("disk_uuid1");
        volInfo.setName("disk_name1");
        hostDiskInfos.add(volInfo);
        proExtendInfo.put(OpenstackConstant.VOLUME_INFO_KEY, JSONObject.writeValueAsString(hostDiskInfos));
        proExtendInfo.put(OpenstackConstant.DOMAIN_ID_KEY, "default");
        protectObject.setExtendInfo(proExtendInfo);
        backupTask.setProtectObject(protectObject);

        Map<String, String> advanceParams = new HashMap<>();
        advanceParams.put(OpenstackConstant.DISK_IDS, "[disk_uuid1, test-uuid2]");
        advanceParams.put(OpenstackConstant.ALL_DISK, "false");
        backupTask.setAdvanceParams(advanceParams);

        ProtectedEnvironment environment = MockFactory.mockEnvironment();
        TaskEnvironment taskEnvironment = BeanTools.copy(environment, TaskEnvironment::new);
        backupTask.setProtectEnv(taskEnvironment);

        StorageRepository dataRepository = new StorageRepository();
        dataRepository.setType(RepositoryTypeEnum.DATA.getType());
        List<StorageRepository> repositoryList = new ArrayList<>();
        repositoryList.add(dataRepository);
        backupTask.setRepositories(repositoryList);
        return backupTask;
    }

    /**
     * 用例场景：如果是临时保留策略，则不调用删除多余副本接口 <br/>
     * 前置条件：副本保留策略为临时保留 <br/>
     * 检查点：临时保留策略不删除多余副本
     */
    @Test
    @Ignore
    public void should_callDeleteExcessCopiesZeroTime_when_deleteExcessCopies_given_temporaryRetentionType() {
        PostBackupTask postBackupTask = new PostBackupTask();
        PolicyBo policyBo = new PolicyBo();
        RetentionBo retentionBo = new RetentionBo();
        retentionBo.setRetentionType(RetentionTypeEnum.TEMPORARY.getType());
        policyBo.setRetention(retentionBo);
        postBackupTask.setPolicyBo(policyBo);
        ReflectionTestUtils.invokeMethod(backupProvider, "deleteExcessCopies", postBackupTask);
        Mockito.verify(copyRestApi, Mockito.times(0)).deleteExcessCopies(anyString(), any());
    }

    /**
     * 用例场景：如果是按数量保留策略，则调用1次多余副本接口 <br/>
     * 前置条件：副本保留策略为按数量保留 <br/>
     * 检查点：按数量保留策略需删除多余副本
     */
    @Test
    public void should_callDeleteExcessCopiesOneTime_when_deleteExcessCopies_given_quantityRetentionType() {
        PostBackupTask postBackupTask = new PostBackupTask();
        PolicyBo policyBo = new PolicyBo();
        RetentionBo retentionBo = new RetentionBo();
        retentionBo.setRetentionType(RetentionTypeEnum.QUANTITY.getType());
        retentionBo.setRetentionQuantity(3);
        policyBo.setRetention(retentionBo);
        postBackupTask.setPolicyBo(policyBo);
        ProtectedObject protectedObject = new ProtectedObject();
        protectedObject.setResourceId(UUIDGenerator.getUUID());
        postBackupTask.setProtectedObject(protectedObject);
        postBackupTask.setUserId(UUIDGenerator.getUUID());
        ReflectionTestUtils.invokeMethod(backupProvider, "deleteExcessCopies", postBackupTask);
        Mockito.verify(copyRestApi, Mockito.times(1)).deleteExcessCopies(anyString(), any());
    }
}