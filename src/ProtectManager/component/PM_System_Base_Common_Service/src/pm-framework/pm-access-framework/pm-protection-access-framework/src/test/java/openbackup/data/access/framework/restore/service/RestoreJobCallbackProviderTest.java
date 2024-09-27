/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.restore.service;

import openbackup.data.access.framework.restore.service.RestoreJobCallbackProvider;
import openbackup.data.access.framework.restore.service.RestoreTaskService;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * RestoreJobCallbackProvider 类测试
 *
 * @since 2022-09-12
 */
@RunWith(PowerMockRunner.class)
public class RestoreJobCallbackProviderTest {
    @Mock
    private RestoreTaskService restoreTaskService;

    @InjectMocks
    private RestoreJobCallbackProvider restoreJobCallbackProvider;


    /**
     * 用例场景：测试applicable接口成功
     * 前置条件：输入不同的jobType
     * 检查点：除RESTORE/RESTORE-v2/INSTANT_RESTORE/INSTANT_RESTORE-v2之外的值返回false。
     */
    @Test
    public void applicable_restore_callback_success() {
        Assert.assertTrue(restoreJobCallbackProvider.applicable("INSTANT_RESTORE"));
        Assert.assertTrue(restoreJobCallbackProvider.applicable("RESTORE"));
        Assert.assertTrue(restoreJobCallbackProvider.applicable("RESTORE-v2"));
        Assert.assertTrue(restoreJobCallbackProvider.applicable("INSTANT_RESTORE-v2"));
        Assert.assertFalse(restoreJobCallbackProvider.applicable("livemount"));
    }

    /**
     * 用例场景：测试callback接口成功。
     * 前置条件：输入正确的jobBo
     * 检查点：成功调用接口，不抛出异常。
     */
    @Test
    public void restore_callback_success() {
        JobBo jobBo = new JobBo();
        restoreJobCallbackProvider.doCallback(jobBo);
        jobBo.setCopyId("123");
        restoreJobCallbackProvider.doCallback(jobBo);
        Mockito.verify(restoreTaskService, Mockito.times(1))
                .updateCopyStatus(jobBo.getCopyId(), CopyStatus.NORMAL);
    }
}
