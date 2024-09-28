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
package openbackup.data.access.framework.protection.handler.v2.livemount;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.enums.DmcJobStatus;
import openbackup.data.access.framework.protection.handler.v2.live.mount.UnifiedLiveMountTaskCompleteHandler;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.HashMap;
import java.util.Map;

/**
 * 统一即时挂载任务完成处理器测试类
 *
 */
@SpringBootTest
@RunWith(SpringRunner.class)
@ContextConfiguration(classes = {UnifiedLiveMountTaskCompleteHandler.class})
public class UnifiedLiveMountTaskCompleteHandlerTest {
    @MockBean
    private NotifyManager notifyManager;

    @Autowired
    private UnifiedLiveMountTaskCompleteHandler handler;

    /**
     * 用例场景：测试是否能够进入处理live mount任务成功场景
     * 前置条件：封装拦截层方法参数
     * 检查点：验证是否能够进入创建挂载任务成功处理器
     */
    @Test
    public void test_handle_live_mount_success_message() {
        TaskCompleteMessageBo taskMessage = new TaskCompleteMessageBo();
        taskMessage.setJobStatus(DmcJobStatus.SUCCESS.getStatus());
        Map<String, String> context = new HashMap<>();
        context.put(ContextConstants.JOB_TYPE, JobTypeEnum.LIVE_MOUNT.getValue());
        context.put("live_mount", "live_mount");
        context.put("request_id", "request_id");
        context.put("job_id", "job_id");
        context.put("clone_copy", "clone_copy");
        context.put("source_copy", "source_copy");
        taskMessage.setContext(context);
        handler.onTaskCompleteSuccess(taskMessage);
        Mockito.verify(notifyManager, Mockito.times(1)).send(any(), any());
    }

    /**
     * 用例场景：测试是否能够进入处理live mount任务失败场景
     * 前置条件：封装拦截层方法参数
     * 检查点：验证是否能够进入创建挂载任务失败处理器
     */
    @Test
    public void test_handle_live_mount_failed_message() {
        TaskCompleteMessageBo taskMessage = new TaskCompleteMessageBo();
        taskMessage.setJobStatus(DmcJobStatus.FAIL.getStatus());
        HashMap<String, String> context = new HashMap<>();
        context.put(ContextConstants.JOB_TYPE, JobTypeEnum.LIVE_MOUNT.getValue());
        context.put("live_mount", "live_mount");
        context.put("request_id", "request_id");
        context.put("job_id", "job_id");
        context.put("clone_copy", "clone_copy");
        context.put("source_copy", "source_copy");
        taskMessage.setContext(context);
        handler.onTaskCompleteFailed(taskMessage);
        Mockito.verify(notifyManager, Mockito.times(1)).send(any(), any());
    }

    /**
     * 用例场景：测试创建即时挂载请求拦截层
     * 前置条件：封装拦截层方法参数
     * 检查点：验证是否能够进入创建挂载拦截层
     */
    @Test
    public void test_live_mount_intercept() {
        Assert.assertTrue(handler.applicable("live_mount-v2"));
        Assert.assertFalse(handler.applicable("unmount-v2"));
    }
}
