/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.dameng.protection.access.interceptor;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.dameng.protection.access.service.DamengService;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Optional;

/**
 * 功能描述
 *
 * @author lWX1100347
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-21
 */
public class DamengCopyDeleteInterceptorTest {
    private final DamengService damengService = PowerMockito.mock(DamengService.class);
    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);
    private final DamengCopyDeleteInterceptor damengCopyDeleteInterceptor = new DamengCopyDeleteInterceptor(
        damengService, copyRestApi, resourceService);

    /**
     * 用例场景：dameng环境检查类过滤
     * 前置条件：无
     * 检查点：过滤成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(damengCopyDeleteInterceptor.applicable(ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType()));
        Assert.assertTrue(damengCopyDeleteInterceptor.applicable(ResourceSubTypeEnum.DAMENG_CLUSTER.getType()));
        Assert.assertFalse(damengCopyDeleteInterceptor.applicable(ResourceSubTypeEnum.GAUSSDBT.getType()));
    }

    /**
     * 用例场景：dameng副本删除设置agent
     * 前置条件：
     * 检查点：设置成功
     */
    @Test
    public void supply_agent_success() {
        Copy copy = generateResourceCopy();
        copy.setUuid("this_copy_id");
        copy.setGn(1);
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setBackupType(BackupTypeConstants.FULL.getAbBackupType());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        CopyInfoBo copyInfoBo = new CopyInfoBo();
        copyInfoBo.setResourceId("this_resource_id");
        copyInfoBo.setResourceSubType(ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        Endpoint endpoint = new Endpoint();
        endpoint.setId("agentUuid");
        PowerMockito.when(damengService.getEndpointList(any())).thenReturn(Collections.singletonList(endpoint));
        DeleteCopyTask deleteCopyTask = new DeleteCopyTask();
        damengCopyDeleteInterceptor.supplyAgent(deleteCopyTask, copyInfoBo);
        Assert.assertEquals(deleteCopyTask.getAgents().get(0).getId(), endpoint.getId());
    }

    /**
     * 用例场景：dameng下发副本删除参数
     * 前置条件：副本资源存在
     * 检查点：设置成功
     */
    @Test
    public void handle_dameng_copy_delete_task_success() {
        CopyInfoBo copyInfoBo = new CopyInfoBo();
        DeleteCopyTask task = new DeleteCopyTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("test");
        task.setProtectObject(taskResource);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        task.setProtectEnv(taskEnvironment);
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("test");
        resource.setRootUuid("test");
        PowerMockito.when(resourceService.getBasicResourceById(anyString())).thenReturn(Optional.of(resource));
        damengCopyDeleteInterceptor.handleTask(task, copyInfoBo);
        Assert.assertNotNull(task.getProtectEnv().getExtendInfo());
        Assert.assertNotNull(task.getProtectEnv().getNodes());
    }

    /**
     * 用例场景：Dameng副本级联删除(删除全量副本时返回依赖副本）
     * 前置条件：副本资源存在，副本数据存在
     * 检查点：是否添加成功需要级联删除的副本
     */
    @Test
    public void get_copies_copy_type_is_difference_increment_success() {
        List<Copy> copies = generateBackupCopies();
        Copy thisCopy = copies.stream().filter(copy -> copy.getGn() == 2).findFirst().get();
        Copy nextFullCopy = copies.stream().filter(copy -> copy.getGn() == 5).findFirst().get();
        List<String> processedCopies = damengCopyDeleteInterceptor.getCopiesCopyTypeIsDifferenceIncrement(copies, thisCopy, nextFullCopy);
        Assert.assertEquals(2, processedCopies.size());
    }

    /**
     * 用例场景：Dameng副本级联删除(删除日志副本）
     * 前置条件：副本资源存在，副本数据存在
     * 检查点：是否返回空
     */
    @Test
    public void collect_need_delete_associated_copy_success_when_this_copy_is_log_copy() {
        List<Copy> copies = generateBackupCopies();
        Copy thisCopy = copies.stream().filter(copy -> copy.getGn() == 6).findFirst().get();
        Copy nextFullCopy = copies.stream().filter(copy -> copy.getGn() == 8).findFirst().get();
        List<String> processedCopies = damengCopyDeleteInterceptor.getCopiesCopyTypeIsLog(copies, thisCopy, nextFullCopy);
        Assert.assertEquals(1, processedCopies.size());
    }


    private List<Copy> generateBackupCopies() {
        // 副本顺序：全增增增全日日全
        Copy fullCopy1 = generateResourceCopy();
        fullCopy1.setGn(1);
        fullCopy1.setUuid("full_01");
        fullCopy1.setBackupType(BackupTypeConstants.FULL.getAbBackupType());
        fullCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy differenceCopy1 = generateResourceCopy();
        differenceCopy1.setGn(2);
        differenceCopy1.setUuid("difference_01");
        differenceCopy1.setBackupType(BackupTypeConstants.DIFFERENCE_INCREMENT.getAbBackupType());
        differenceCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy differenceCopy2 = generateResourceCopy();
        differenceCopy2.setGn(3);
        differenceCopy2.setUuid("difference_02");
        differenceCopy2.setBackupType(BackupTypeConstants.DIFFERENCE_INCREMENT.getAbBackupType());
        differenceCopy2.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy differenceCopy3 = BeanTools.copy(differenceCopy2, Copy::new);
        differenceCopy3.setGn(4);
        differenceCopy3.setUuid("difference_03");
        Copy fullCopy2 = BeanTools.copy(fullCopy1, Copy::new);
        fullCopy2.setGn(5);
        fullCopy2.setUuid("full_02");
        Copy logCopy1 = generateResourceCopy();
        logCopy1.setGn(6);
        logCopy1.setUuid("log_01");
        logCopy1.setBackupType(BackupTypeConstants.LOG.getAbBackupType());
        logCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy logCopy2 = BeanTools.copy(logCopy1, Copy::new);
        logCopy2.setGn(7);
        logCopy2.setUuid("log_02");
        Copy fullCopy3 = BeanTools.copy(fullCopy1, Copy::new);
        fullCopy3.setGn(8);
        fullCopy3.setUuid("full_03");
        return Arrays.asList(fullCopy1, fullCopy2, fullCopy3, differenceCopy1, differenceCopy2, differenceCopy3,
            logCopy1, logCopy2);
    }

    private Copy generateResourceCopy() {
        Copy copy = new Copy();
        copy.setResourceId("this_resource_id");
        return copy;
    }
}
