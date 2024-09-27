/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.openstack.protection.access.provider;

import static openbackup.openstack.protection.access.provider.OpenstackRestoreProvider.SERVER_EXTEND_INFO;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.protection.access.provider.sdk.anti.ransomware.CopyRansomwareService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.MessageTemplate;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.ArgumentMatchers;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述: OpenstackRestoreProviderTest
 *
 * @author c30016231
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-18
 */
public class OpenstackRestoreProviderTest {
    private OpenstackRestoreProvider openstackRestoreProvider;
    private static final ResourceService resourceService = PowerMockito.mock(ResourceService.class);
    private static final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);
    private static final MessageTemplate<String> messageTemplate = PowerMockito.mock(MessageTemplate.class);

    private CopyRansomwareService copyRansomwareService = PowerMockito.mock(CopyRansomwareService.class);

    @Before
    public void init() {
        openstackRestoreProvider = new OpenstackRestoreProvider(resourceService, copyRestApi, messageTemplate);
        openstackRestoreProvider.setCopyRansomwareService(copyRansomwareService);
    }

    /**
     * 用例场景：Openstack恢复拦截补充正确 <br/>
     * 前置条件：Openstack恢复对象参数正确 <br/>
     * 检查点：返回恢复任务对象参数补充正确
     */
    @Test
    public void test_restore_intercept_success() {
        RestoreTask restoreTask = mockRestoreTask();
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        ProtectedResource domain = MockFactory.mockProtectedResource();
        domain.setAuth(new Authentication());
        Mockito.when(resourceService.getResourceById(ArgumentMatchers.anyBoolean(), ArgumentMatchers.any()))
            .thenReturn(Optional.of(domain));

        restoreTask = openstackRestoreProvider.initialize(restoreTask);
        TaskResource targetObject = restoreTask.getTargetObject();
        Map<String, String> extendInfo = targetObject.getExtendInfo();
        Assert.assertEquals("1", extendInfo.get(OpenstackConstant.POWER_STATE));
        Assert.assertEquals("/targetLocation", targetObject.getTargetLocation());
        Map<String, String> advanceParams = restoreTask.getAdvanceParams();
        Assert.assertEquals("1", advanceParams.get(OpenstackConstant.RESTORE_LEVEL));
        Assert.assertEquals(domain.getAuth(), restoreTask.getTargetObject().getAuth());
    }

    /**
     * 用例场景：从磁带归档恢复时，Openstack恢复拦截补充正确 <br/>
     * 前置条件：Openstack恢复对象参数正确 <br/>
     * 检查点：返回恢复任务对象参数补充正确
     */
    @Test
    public void test_restore_intercept_success_when_copy_generated_by_tape_archive() {
        RestoreTask restoreTask = mockRestoreTask();
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        JSONObject jsonObject = new JSONObject();
        jsonObject.put(SERVER_EXTEND_INFO, "test");
        copy.setProperties(jsonObject.toString());
        restoreTask = openstackRestoreProvider.initialize(restoreTask);
        Assert.assertEquals(RestoreModeEnum.DOWNLOAD_RESTORE.getMode(), restoreTask.getRestoreMode());
    }

    /**
     * 用例场景：测试获取Openstack锁资源成功 <br/>
     * 前置条件：Openstack恢复对象参数正确 <br/>
     * 检查点：返回锁对象非空，且id准确
     */
    @Test
    public void test_get_lock_resources_success() {
        RestoreTask restoreTask = mockRestoreTask();
        List<LockResourceBo> lockResources = openstackRestoreProvider.getLockResources(restoreTask);
        Assert.assertNotNull(lockResources);
        Assert.assertEquals(restoreTask.getTargetObject().getUuid(), lockResources.get(0).getId());
    }


    /**
     * 用例场景：Openstack恢复后置处理成功 <br/>
     * 前置条件：参数正确 <br/>
     * 检查点：流程无异常
     */
    @Test
    public void test_post_process_success() {
        ProtectedResource project = new ProtectedResource();
        project.setUuid(UUIDGenerator.getUUID());
        PowerMockito.when(resourceService.getResourceById(anyString()))
            .thenReturn(Optional.of(project));
        PowerMockito.when(messageTemplate.send(anyString(), any(JSONObject.class))).thenReturn(null);
        openstackRestoreProvider.postProcess(mockRestoreTask(), ProviderJobStatusEnum.SUCCESS);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：Openstack恢复插件类型判断正确 <br/>
     * 前置条件：流程正常 <br/>
     * 检查点：返回结果为True
     */
    @Test
    public void test_applicable_success() {
        boolean isOpenstackCloudHost = openstackRestoreProvider.applicable(
            ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType());
        Assert.assertTrue(isOpenstackCloudHost);
    }

    /**
     * 用例场景：Openstack恢复插件类型判断正确 <br/>
     * 前置条件：流程正常 <br/>
     * 检查点：返回结果为True
     */
    @Test
    public void test_restore_task_creation_pre_check(){
        ProtectedResource parentResource = MockFactory.mockProtectedResource();
        parentResource.setPath("parentName");
        Mockito.when(resourceService.getBasicResourceById(ArgumentMatchers.any()))
            .thenReturn(Optional.of(parentResource));

        Map<String, String> advanceParams = new HashMap();
        advanceParams.put(OpenstackConstant.RESTORE_LEVEL, OpenstackConstant.VM_RESTORE);
        TaskResource taskResource = new TaskResource();
        taskResource.setName("{\"name\":\"t1\"}");
        RestoreTask task = new RestoreTask();
        task.addParameters(advanceParams);
        task.setTargetObject(taskResource);
        openstackRestoreProvider.restoreTaskCreationPreCheck(task);
        Assert.assertEquals("parentName/t1", task.getTargetObject().getTargetLocation());
    }

    private RestoreTask mockRestoreTask() {
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setTaskId(UUIDGenerator.getUUID());
        TaskResource protectObject = new TaskResource();
        protectObject.setUuid(UUIDGenerator.getUUID());
        protectObject.setParentUuid(UUIDGenerator.getUUID());
        Map<String, String> extendInfo = new HashMap<>();
        protectObject.setExtendInfo(extendInfo);
        restoreTask.setTargetObject(protectObject);

        Map<String, String> advanceParams = new HashMap<>();
        advanceParams.put(OpenstackConstant.RESTORE_LOCATION, "/targetLocation");
        restoreTask.setAdvanceParams(advanceParams);
        ProtectedEnvironment environment = MockFactory.mockEnvironment();
        TaskEnvironment taskEnvironment = BeanTools.copy(environment, TaskEnvironment::new);
        restoreTask.setTargetEnv(taskEnvironment);
        TaskResource subObject = new TaskResource();
        subObject.setUuid("subObject_id");
        restoreTask.setSubObjects(Collections.singletonList(subObject));
        return restoreTask;
    }
}