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
package openbackup.data.access.framework.copy.verify.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.copy.verify.provider.CopyVerifyJobCallbackProvider;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;

import org.junit.Test;
import org.mockito.Mockito;

/**
 * CopyVerifyJobCallbackProviderTest
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-07-11
 */
public class CopyVerifyJobCallbackProviderTest {
    private final CopyRestApi copyRestApi = Mockito.mock(CopyRestApi.class);

    private final CopyVerifyJobCallbackProvider provider = new CopyVerifyJobCallbackProvider(copyRestApi);

    /**
     * 用例场景：副本校验任务超时失败后，回退副本状态
     * 前置条件：无
     * 检查点：副本状态回退正常
     */
    @Test
    public void copy_status_should_update_success() {
        JobBo jobBo = new JobBo();
        jobBo.setJobId("jobId");
        jobBo.setCopyId("copyId");
        Mockito.when(copyRestApi.queryCopyByID(jobBo.getCopyId(), false)).thenReturn(new Copy());
        provider.doCallback(jobBo);
        Mockito.verify(copyRestApi, Mockito.times(1)).updateCopyStatus(any(), any());
    }
}
