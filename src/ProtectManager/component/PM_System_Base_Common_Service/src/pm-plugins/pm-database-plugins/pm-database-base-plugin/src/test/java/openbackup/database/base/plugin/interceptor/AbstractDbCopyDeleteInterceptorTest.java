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
package openbackup.database.base.plugin.interceptor;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.util.BeanTools;

import io.jsonwebtoken.lang.Collections;

import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.reflect.Whitebox;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * AbstractDbCopyDeleteInterceptorTest
 *
 */
public class AbstractDbCopyDeleteInterceptorTest {
    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final AbstractDbCopyDeleteInterceptor interceptor = new AbstractDbCopyDeleteInterceptor(copyRestApi,
        resourceService) {
        @Override
        public boolean applicable(String object) {
            return false;
        }
    };

    /**
     * 用例场景：删除时是否需要下发agent，备份副本需要下发
     * 前置条件：副本资源存在，副本数据存在
     * 检查点：返回true
     */
    @Test
    public void should_supply_agent_success() {
        Copy copy = new Copy();
        copy.setUuid("this_copy_id");
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        DeleteCopyTask task = new DeleteCopyTask();
        task.setProtectEnv(new TaskEnvironment());
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("test");
        task.setProtectObject(taskResource);
        ProtectedResource resource = new ProtectedResource();
        resource.setRootUuid("root_uuid");
        PowerMockito.when(resourceService.getBasicResourceById("test")).thenReturn(Optional.of(resource));
        PowerMockito.when(resourceService.getBasicResourceById("root_uuid")).thenReturn(Optional.of(resource));
        Assert.assertTrue(interceptor.shouldSupplyAgent(task, new CopyInfoBo()));
    }

    /**
     * 用例场景：删除时是否需要下发agent，资源不存在的时候不需要
     * 前置条件：副本资源不存在，副本数据存在
     * 检查点：返回false
     */
    @Test
    public void should_not_supply_agent_when_resource_is_not_exist() {
        Copy copy = new Copy();
        copy.setUuid("this_copy_id");
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        DeleteCopyTask task = new DeleteCopyTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("test");
        task.setProtectObject(taskResource);
        ProtectedResource resource = new ProtectedResource();
        resource.setRootUuid("root_uuid");
        PowerMockito.when(resourceService.getResourceById("test")).thenReturn(Optional.empty());
        Assert.assertFalse(interceptor.shouldSupplyAgent(task, new CopyInfoBo()));
    }

    @Test
    public void testAddResourcesToSetNextFullFull() {
        Copy copy = new Copy();
        copy.setUuid("this_copy_id");
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setBackupType(1);
        PowerMockito.when(copyRestApi.queryLatestBackupCopy(any(), any(), any())).thenReturn(copy);
        Assert.assertFalse(Collections.isEmpty(interceptor.addResourcesToSetNextFull(copy, "requestId")));
    }

    @Test
    public void testAddResourcesToSetNextFullCumulativeIncrement() {
        Copy copy = new Copy();
        copy.setUuid("this_copy_id");
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setBackupType(3);
        PowerMockito.when(copyRestApi.queryLatestBackupCopy(any(), any(), any())).thenReturn(copy);
        Assert.assertFalse(Collections.isEmpty(interceptor.addResourcesToSetNextFull(copy, "requestId")));
    }

    @Test
    public void testAddResourcesToSetNextFullDifferenceIncrement() {
        Copy copy = new Copy();
        copy.setUuid("this_copy_id");
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setBackupType(2);
        PowerMockito.when(copyRestApi.queryLatestBackupCopy(any(), any(), any())).thenReturn(copy);
        Assert.assertTrue(Collections.isEmpty(interceptor.addResourcesToSetNextFull(copy, "requestId")));
    }

    @Test
    public void testIsEnvironmentOffline() {
        DeleteCopyTask deleteCopyTask = new DeleteCopyTask();
        TaskEnvironment protectEnv = new TaskEnvironment();
        protectEnv.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
        deleteCopyTask.setProtectEnv(protectEnv);
        Assert.assertTrue(interceptor.isEnvironmentOffline(deleteCopyTask));
    }

    @Test
    public void testCollectNeedDeleteAssociatedCopy() throws NoSuchFieldException, IllegalAccessException {
        DeleteCopyTask deleteCopyTask = new DeleteCopyTask();
        TaskEnvironment protectEnv = new TaskEnvironment();
        protectEnv.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
        deleteCopyTask.setProtectEnv(protectEnv);
        Class<?> classType = interceptor.getClass().getSuperclass().getSuperclass();
        Field copyRestApiField = classType.getDeclaredField("copyRestApi");
        copyRestApiField.setAccessible(true);
        copyRestApiField.set(interceptor, copyRestApi);
        Assert.assertTrue(Collections.isEmpty(interceptor.getAssociatedCopy("deleteCopyTask")));
    }

    @Test
    public void testGetCopiesCopyTypeIsFull() {
        Copy copy = new Copy();
        copy.setUuid("this_copy_id");
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setBackupType(1);
        Assert.assertTrue(Collections.isEmpty(interceptor.getCopiesCopyTypeIsFull(new ArrayList<>(), copy, copy)));
    }

    @Test
    public void testGetCopiesCopyTypeIsDifferenceIncrement() {
        Copy copy = new Copy();
        copy.setUuid("this_copy_id");
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setBackupType(1);
        Assert.assertTrue(
            Collections.isEmpty(interceptor.getCopiesCopyTypeIsDifferenceIncrement(new ArrayList<>(), copy, copy)));
    }

    @Test
    public void testGetCopiesCopyTypeIsCumulativeIncrement() {
        Copy copy = new Copy();
        copy.setUuid("this_copy_id");
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setBackupType(1);
        Assert.assertTrue(
            Collections.isEmpty(interceptor.getCopiesCopyTypeIsCumulativeIncrement(new ArrayList<>(), copy, copy)));
    }

    @Test
    public void testGetCopiesCopyTypeIsLog() {
        Copy copy = new Copy();
        copy.setUuid("this_copy_id");
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setBackupType(1);
        Assert.assertTrue(Collections.isEmpty(interceptor.getCopiesCopyTypeIsLog(new ArrayList<>(), copy, copy)));
    }

    /**
     * 用例场景：删除全量副本时返回依赖副本（公共逻辑）
     * 前置条件：副本资源存在，副本数据存在
     * 检查点：是否添加成功需要级联删除的副本
     */
    @Test
    @Ignore
    public void collect_need_delete_associated_copy_success_when_this_copy_is_full_copy() throws Exception {
        DeleteCopyTask task = new DeleteCopyTask();
        CopyInfoBo copyInfoBo = generateCopyInfoBo();
        Copy copy = generateResourceCopy();
        copy.setGn(1);
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setBackupType(BackupTypeConstants.FULL.getAbBackupType());
        baseMock(copy);
        List<String> copyUuids = Whitebox.invokeMethod(interceptor, "collectNeedDeleteAssociatedCopy", "1");
        List<String> expectResult = Arrays.asList("difference_01", "difference_02", "difference_03");
        Assert.assertTrue(copyUuids.containsAll(expectResult));
        copyUuids.removeAll(expectResult);
        Assert.assertEquals(0, copyUuids.size());
    }

    /**
     * 用例场景：删除增量副本时默认为空，各应用实现
     * 前置条件：副本资源存在，副本数据存在
     * 检查点：返回空
     */
    @Test
    @Ignore
    public void collect_need_delete_associated_copy_success_when_this_copy_is_difference_copy() throws Exception {
        DeleteCopyTask task = new DeleteCopyTask();
        CopyInfoBo copyInfoBo = generateCopyInfoBo();
        Copy copy = generateResourceCopy();
        copy.setGn(2);
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setBackupType(BackupTypeConstants.DIFFERENCE_INCREMENT.getAbBackupType());
        baseMock(copy);
        List<String> copyUuids = Whitebox.invokeMethod(interceptor, "collectNeedDeleteAssociatedCopy", "1");
        Assert.assertEquals(0, copyUuids.size());
    }

    /**
     * 用例场景：删除日志副本时默认为空，各应用实现
     * 前置条件：副本资源存在，副本数据存在
     * 检查点：返回空
     */
    @Test
    @Ignore
    public void collect_need_delete_associated_copy_success_when_this_copy_is_log_copy() throws Exception {
        DeleteCopyTask task = new DeleteCopyTask();
        CopyInfoBo copyInfoBo = generateCopyInfoBo();
        Copy copy = generateResourceCopy();
        copy.setGn(8);
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setBackupType(BackupTypeConstants.LOG.getAbBackupType());
        baseMock(copy);
        List<String> copyUuids = Whitebox.invokeMethod(interceptor, "collectNeedDeleteAssociatedCopy", "1");
        Assert.assertEquals(0, copyUuids.size());
    }

    /**
     * 用例场景：副本级联删除(非备份副本）
     * 前置条件：非备份副本
     * 检查点：返回空
     */
    @Test
    @Ignore
    public void collect_need_delete_associated_copy_empty_when_this_copy_not_generated_by_backup() throws Exception {
        DeleteCopyTask task = new DeleteCopyTask();
        CopyInfoBo copyInfoBo = generateCopyInfoBo();
        Copy copy = generateResourceCopy();
        copy.setGn(8);
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value());
        copy.setBackupType(BackupTypeConstants.FULL.getAbBackupType());
        baseMock(copy);
        List<String> copyUuids = Whitebox.invokeMethod(interceptor, "collectNeedDeleteAssociatedCopy", "1");
        Assert.assertEquals(0, copyUuids.size());
    }

    private void baseMock(Copy copy) {
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        List<Copy> copies = generateBackupCopies().stream()
            .filter(item -> item.getGn() > copy.getGn())
            .collect(Collectors.toList());
        PowerMockito.when(copyRestApi.queryLaterBackupCopies(copy.getResourceId(), copy.getGn() + 1))
            .thenReturn(copies);
    }

    private List<Copy> generateBackupCopies() {
        // 副本顺序：全增增增全增增全
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
        return Arrays.asList(fullCopy1, fullCopy2, differenceCopy1, differenceCopy2, differenceCopy3);
    }

    private Copy generateResourceCopy() {
        Copy copy = new Copy();
        copy.setResourceId("this_resource_id");
        return copy;
    }

    private CopyInfoBo generateCopyInfoBo() {
        CopyInfoBo copyInfoBo = new CopyInfoBo();
        copyInfoBo.setUuid("this_copy_id");
        copyInfoBo.setResourceId("this_resource_id");
        return copyInfoBo;
    }
}
