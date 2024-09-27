/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.protection.handler.v2.live.mount;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.enums.DmcJobStatus;
import openbackup.data.access.framework.protection.handler.v2.live.mount.UnifiedLiveMountTaskCompleteHandler;
import openbackup.data.access.framework.protection.handler.v2.live.mount.UnifiedUnmountTaskCompleteHandler;
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
 * 统一即时挂载卸载任务完成处理器测试类
 *
 * @author twx1009756
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-01-18
 */
@SpringBootTest
@RunWith(SpringRunner.class)
@ContextConfiguration(classes = {UnifiedUnmountTaskCompleteHandler.class})
public class UnifiedUnmountTaskCompleteHandlerTest {
    @MockBean
    private UnifiedLiveMountTaskCompleteHandler liveMountTaskCompleteHandler;

    @MockBean
    private NotifyManager notifyManager;

    @Autowired
    private UnifiedUnmountTaskCompleteHandler handler;

    /**
     * 用例场景：测试是否能够进入处理卸载即时挂载任务成功场景
     * 前置条件：封装拦截层方法参数
     * 检查点：验证是否能够进入卸载即时挂载任务成功处理器
     */
    @Test
    public void test_handle_backup_success_message() {
        TaskCompleteMessageBo taskMessage = new TaskCompleteMessageBo();
        taskMessage.setJobStatus(DmcJobStatus.SUCCESS.getStatus());
        Map<String, String> context = new HashMap<>();
        context.put(ContextConstants.JOB_TYPE, JobTypeEnum.UNMOUNT.getValue());
        context.put("live_mount", "live_mount");
        context.put("request_id", "request_id");
        context.put("job_id", "job_id");
        context.put("clone_copy", "clone_copy");
        context.put("source_copy", "source_copy");
        taskMessage.setContext(context);
        handler.onTaskCompleteSuccess(taskMessage);
        Mockito.verify(liveMountTaskCompleteHandler, Mockito.times(1)).sendLiveMountDoneMessage(any());
    }

    /**
     * 用例场景：测试是否能够进入处理卸载即时挂载任务失败场景
     * 前置条件：封装拦截层方法参数
     * 检查点：验证是否能够进入卸载即时挂载任务失败处理器
     */
    @Test
    public void test_handle_backup_failed_message() {
        TaskCompleteMessageBo taskMessage = new TaskCompleteMessageBo();
        taskMessage.setJobStatus(DmcJobStatus.FAIL.getStatus());
        HashMap<String, String> context = new HashMap<>();
        context.put(ContextConstants.JOB_TYPE, JobTypeEnum.UNMOUNT.getValue());
        context.put("live_mount", "live_mount");
        context.put("request_id", "request_id");
        context.put("job_id", "job_id");
        context.put("clone_copy", "clone_copy");
        context.put("source_copy", "source_copy");
        taskMessage.setContext(context);
        handler.onTaskCompleteFailed(taskMessage);
        Mockito.verify(liveMountTaskCompleteHandler, Mockito.times(1)).sendLiveMountDoneMessage(any());
    }

    /**
     * 用例场景：测试创建即时挂载请求拦截层
     * 前置条件：封装拦截层方法参数
     * 检查点：验证是否能够进入创建挂载拦截层
     */
    @Test
    public void test_live_mount_intercept() {
        Assert.assertFalse(handler.applicable("live_mount-v2"));
        Assert.assertTrue(handler.applicable("unmount-v2"));
    }
}
