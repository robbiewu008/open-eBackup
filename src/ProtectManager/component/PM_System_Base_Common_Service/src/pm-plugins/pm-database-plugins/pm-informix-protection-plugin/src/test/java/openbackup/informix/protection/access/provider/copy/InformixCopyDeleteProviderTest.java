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
package openbackup.informix.protection.access.provider.copy;

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
 * InformixProviderTest
 *
 * @author hwx1164326
 * @version [DataBackup 1.5.0]
 * @since 2023-08-03
 */
public class InformixCopyDeleteProviderTest {
    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);
    private final InformixCopyDeleteInterceptor copyDeleteInterceptor =
            new InformixCopyDeleteInterceptor(copyRestApi, resourceService);

    /**
     * 用例场景：检查是否需要下发agent
     * 前置条件：无
     * 检查点：不需要下发
     */
    @Test
    public void check_should_supply_agent() {
        DeleteCopyTask task = new DeleteCopyTask();
        CopyInfoBo copy = new CopyInfoBo();
        boolean isSupplyAgent = copyDeleteInterceptor.shouldSupplyAgent(task, copy);
        Assert.assertFalse(isSupplyAgent);
    }

    /**
     * 用例场景：检查informix单实例是否满足条件
     * 前置条件：无
     * 检查点：informix单实例满足条件
     */
    @Test
    public void check_applicable() {
        boolean isApplicable =
                copyDeleteInterceptor.applicable(ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType());
        Assert.assertTrue(isApplicable);
    }

    /**
     * 用例场景：检查informix单实例是否满足条件
     * 前置条件：无
     * 检查点：informix单实例满足条件
     */
    @Test
    public void getCopiesCopyTypeIsCumulativeIncrement() {
        List<Copy> copyList = generateBackupCopies();
        List<String> deleteList =
                copyDeleteInterceptor.getCopiesCopyTypeIsCumulativeIncrement(copyList, copyList.get(1), null);
        Assert.assertEquals(deleteList.size(), 3);
    }

    private List<Copy> generateBackupCopies() {
        // 副本顺序：全差增日日全
        Copy fullCopy1 = generateResourceCopy();
        fullCopy1.setGn(1);
        fullCopy1.setUuid("full_01");
        fullCopy1.setBackupType(BackupTypeConstants.FULL.getAbBackupType());
        fullCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy cumulativeCopy1 = generateResourceCopy();
        cumulativeCopy1.setGn(2);
        cumulativeCopy1.setUuid("cumulative_01");
        cumulativeCopy1.setBackupType(BackupTypeConstants.CUMULATIVE_INCREMENT.getAbBackupType());
        cumulativeCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy differenceCopy1 = generateResourceCopy();
        differenceCopy1.setGn(3);
        differenceCopy1.setUuid("difference_01");
        differenceCopy1.setBackupType(BackupTypeConstants.DIFFERENCE_INCREMENT.getAbBackupType());
        differenceCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy logCopy1 = generateResourceCopy();
        logCopy1.setGn(5);
        logCopy1.setUuid("log_01");
        logCopy1.setBackupType(BackupTypeConstants.LOG.getAbBackupType());
        logCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy logCopy2 = BeanTools.copy(logCopy1, Copy::new);
        logCopy2.setGn(6);
        logCopy2.setUuid("log_02");
        Copy fullCopy2 = BeanTools.copy(fullCopy1, Copy::new);
        logCopy2.setGn(7);
        logCopy2.setUuid("full_02");
        return Arrays.asList(fullCopy1, cumulativeCopy1, differenceCopy1, logCopy1, logCopy2, fullCopy2);
    }

    private Copy generateResourceCopy() {
        Copy copy = new Copy();
        copy.setResourceId("this_resource_id");
        return copy;
    }
}
