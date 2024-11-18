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
package openbackup.tpops.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import junit.framework.TestCase;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {TpopsGaussDbJobQueueProvider.class, ResourceService.class})
public class TpopsGaussDbJobQueueProviderTest extends TestCase {

    private TpopsGaussDbJobQueueProvider tpopsGaussDbJobQueueProvider;

    @Mock
    private ResourceService resourceService;

    @Before
    public void init() throws IllegalAccessException {
        tpopsGaussDbJobQueueProvider = new TpopsGaussDbJobQueueProvider(resourceService);
    }

    @Test
    public void testApplicableWithYes() {
        Job job = new Job();
        job.setSourceSubType(ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType());
        job.setType(JobTypeEnum.BACKUP.getValue());
        Assert.assertTrue(tpopsGaussDbJobQueueProvider.applicable(job));
    }

    @Test
    public void testApplicableWithNo() {
        Job job = new Job();
        job.setSourceSubType(ResourceSubTypeEnum.TPOPS_GAUSSDB_PROJECT.getType());
        job.setType(JobTypeEnum.BACKUP.getValue());
        Assert.assertFalse(tpopsGaussDbJobQueueProvider.applicable(job));
    }

    @Test
    public void testApplicableWithOtherResource() {
        Job job = new Job();
        job.setSourceSubType(ResourceSubTypeEnum.EXCHANGE_GROUP.getType());
        job.setType(JobTypeEnum.RESTORE.getValue());
        Assert.assertFalse(tpopsGaussDbJobQueueProvider.applicable(job));
    }
}