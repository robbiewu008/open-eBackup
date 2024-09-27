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
package openbackup.data.access.framework.protection.service.replication;

import openbackup.data.access.framework.protection.service.replication.UnifiedReplicationCopyProcessor;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;

import java.util.ArrayList;
import java.util.UUID;
import java.util.concurrent.atomic.AtomicStampedReference;

/**
 * Advance Replication Copy Processor Test
 *
 * @author l00272247
 * @since 2022-01-18
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(UnifiedReplicationCopyProcessor.class)
@AutoConfigureMockMvc
public class UnifiedReplicationCopyProcessorTest {
    @InjectMocks
    private UnifiedReplicationCopyProcessor unifiedReplicationCopyProcessor;

    @Test
    public void test_process() {
        TaskCompleteMessageBo taskCompleteMessageBo = new TaskCompleteMessageBo();
        ArrayList<String> list = new ArrayList<>();
        list.add(UUID.randomUUID().toString());
        JSONArray jsonArray = JSONArray.fromObject(list);
        JSONObject jsonObject = new JSONObject();
        jsonObject.set("backup_copy_list", jsonArray);
        taskCompleteMessageBo.setExtendsInfo(jsonObject);
        taskCompleteMessageBo.setJobStatus(IsmNumberConstant.THREE);
        AtomicStampedReference<Boolean> reference = unifiedReplicationCopyProcessor.process(taskCompleteMessageBo);
        int[] stampHolder = new int[1];
        Boolean isComplete = reference.get(stampHolder);
        int stamp = reference.getStamp();
        Assert.assertTrue(isComplete);
        Assert.assertEquals(1, stamp);
    }

    @Test
    public void test_applicable() {
        Assert.assertTrue(unifiedReplicationCopyProcessor.applicable(ResourceSubTypeEnum.VMWARE.getType()));
    }
}
