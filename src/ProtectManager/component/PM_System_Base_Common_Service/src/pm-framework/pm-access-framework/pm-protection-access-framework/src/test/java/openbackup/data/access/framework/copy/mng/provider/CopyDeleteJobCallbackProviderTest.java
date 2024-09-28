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
package openbackup.data.access.framework.copy.mng.provider;

import openbackup.data.access.framework.copy.mng.provider.CopyDeleteJobCallbackProvider;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;

import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyString;

/**
 * CopyDeleteJobCallbackProvider测试
 *
 */
public class CopyDeleteJobCallbackProviderTest {
    private CopyRestApi copyRestApi;

    private CopyDeleteJobCallbackProvider copyDeleteJobCallbackProvider;

    @Before
    public void init() {
        copyRestApi = Mockito.mock(CopyRestApi.class);
        copyDeleteJobCallbackProvider = new CopyDeleteJobCallbackProvider(copyRestApi);
    }

    /**
     * 用例场景：副本删除任务超时失败后，回退副本状态
     * 前置条件：无
     * 检查点：副本状态回退正常
     */
    @Test
    public void copy_status_should_update_success() {
        JobBo jobBo = new JobBo();
        jobBo.setJobId("jobId");
        jobBo.setCopyId("copyId");
        PowerMockito.when(copyRestApi.queryCopyByID(anyString(),anyBoolean())).thenReturn(new Copy());
        copyDeleteJobCallbackProvider.doCallback(jobBo);
        Mockito.verify(copyRestApi, Mockito.times(1)).updateCopyStatus(Mockito.eq(jobBo.getCopyId()), any());
    }
}
