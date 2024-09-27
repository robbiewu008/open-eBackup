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
package openbackup.opengauss.resources.access.interceptor;

import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
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
import java.util.List;

/**
 * OpenGauss副本删除测试类
 *
 * @author lWX1100347
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-17
 */
public class OpenGaussCopyDeleteInterceptorTest {
    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);
    private final OpenGaussCopyDeleteInterceptor copyDeleteInterceptor = new OpenGaussCopyDeleteInterceptor(
        copyRestApi, resourceService);

    /**
     * 用例场景：OpenGauss副本删除策略适配
     * 前置条件：类型判断
     * 检查点：是否匹配
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(copyDeleteInterceptor.applicable(ResourceSubTypeEnum.OPENGAUSS_INSTANCE.getType()));
        Assert.assertTrue(copyDeleteInterceptor.applicable(ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType()));
    }

    /**
     * 用例场景：OpenGauss副本级联删除(删除全量副本时返回依赖副本）
     * 前置条件：副本资源存在，副本数据存在
     * 检查点：是否添加成功需要级联删除的副本
     */
    @Test
    public void get_copies_copy_type_is_difference_increment_success() {
        List<Copy> copies = generateBackupCopies();
        Copy thisCopy = copies.stream().filter(copy -> copy.getGn() == 2).findFirst().get();
        Copy nextFullCopy = copies.stream().filter(copy -> copy.getGn() == 5).findFirst().get();
        List<String> processedCopies = copyDeleteInterceptor.getCopiesCopyTypeIsDifferenceIncrement(copies, thisCopy, nextFullCopy);
        Assert.assertEquals(2, processedCopies.size());
    }

    /**
     * 用例场景：OpenGauss设置是否强制删除
     * 前置条件：无
     * 检查点：设置强制删除成功
     */
    @Test
    public void handle_task_success() {
        DeleteCopyTask task = new DeleteCopyTask();
        CopyInfoBo copyInfoBo = generateCopyInfoBo();
        copyDeleteInterceptor.handleTask(task, copyInfoBo);
        Assert.assertTrue(task.getIsForceDeleted());
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
        Copy differenceCopy4 = BeanTools.copy(differenceCopy1, Copy::new);
        differenceCopy4.setGn(6);
        differenceCopy4.setUuid("difference_04");
        Copy differenceCopy5 = BeanTools.copy(differenceCopy1, Copy::new);
        differenceCopy5.setGn(7);
        differenceCopy5.setUuid("difference_05");
        Copy fullCopy3 = BeanTools.copy(fullCopy1, Copy::new);
        fullCopy3.setGn(8);
        fullCopy3.setUuid("full_03");
        return Arrays.asList(fullCopy1, fullCopy2, fullCopy3, differenceCopy1, differenceCopy2, differenceCopy3,
            differenceCopy4, differenceCopy5);
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