/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.restore.handler.v2;

import openbackup.data.access.framework.copy.verify.service.CopyVerifyTaskManager;
import openbackup.data.access.framework.protection.mocks.RestoreTaskMocker;

import openbackup.data.access.framework.restore.service.RestoreTaskManager;
import openbackup.data.access.framework.restore.service.RestoreTaskService;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import org.junit.Assert;
import org.junit.Test;

import java.util.UUID;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

/**
 * 统一恢复任务完成处理器测试集合
 *
 * @author y00559272
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/8/31
 **/
public class UnifiedRestoreTaskCompleteHandlerTest {
    private final RestoreTaskManager restoreTaskManager = mock(RestoreTaskManager.class);
    private final RestoreTaskService restoreTaskService = mock(RestoreTaskService.class);
    private final CopyVerifyTaskManager copyVerifyTaskManager = mock(CopyVerifyTaskManager.class);

    private UnifiedRestoreTaskCompleteHandler taskCompleteHandler = new UnifiedRestoreTaskCompleteHandler(
        restoreTaskManager, restoreTaskService, copyVerifyTaskManager);

    /**
     * 用例场景：当任务是恢复主任务时，执行恢复完成逻辑 <br>
     * 前置条件：参数正确 <br>
     * 检查点：消验证恢复完成分支逻辑执行一次 <br>
     */
    @Test
    public void should_execute_restore_complete_when_success_given_task_is_restore_main_task() {
        // given
        String givenRequestId = UUID.randomUUID().toString();
        int status = ProviderJobStatusEnum.SUCCESS.getStatus();
        TaskCompleteMessageBo givenMessage = buildMessage(givenRequestId, givenRequestId, status);
        RestoreTask givenTask = RestoreTaskMocker.mockRestoreTask(givenRequestId);
        givenTask.setTaskId(givenRequestId);
        given(restoreTaskService.getRestoreTaskFromJob(givenRequestId)).willReturn(givenTask);
        // when
        taskCompleteHandler.onTaskCompleteSuccess(givenMessage);
        // then
        verify(restoreTaskManager, times(1)).complete(any(), any());
    }


    /**
     * 用例场景：当任务是副本校验子任务时，执行副本校验完成逻辑 <br>
     * 前置条件：参数正确 <br>
     * 检查点：消验证副本校验完成分支逻辑执行一次 <br>
     */
    @Test
    public void should_execute_verify_complete_when_success_given_task_is_verify_sub_task() {
        // given
        String givenRequestId = UUID.randomUUID().toString();
        String givenTaskId = UUID.randomUUID().toString();
        int status = ProviderJobStatusEnum.SUCCESS.getStatus();
        TaskCompleteMessageBo givenMessage = buildMessage(givenRequestId, givenTaskId, status);
        RestoreTask givenTask = RestoreTaskMocker.mockRestoreTask(givenRequestId);
        givenTask.setTaskId(givenRequestId);
        given(restoreTaskService.getRestoreTaskFromJob(givenRequestId)).willReturn(givenTask);
        // when
        taskCompleteHandler.onTaskCompleteSuccess(givenMessage);
        // then
        verify(copyVerifyTaskManager, times(1)).complete(any(), any());
    }

    /**
     * 用例场景：当任务是恢复主任务时，执行恢复完成逻辑 <br>
     * 前置条件：参数正确 <br>
     * 检查点：消验证恢复完成分支逻辑执行一次 <br>
     */
    @Test
    public void should_execute_restore_complete_when_failed_given_task_is_restore_main_task() {
        // given
        String givenRequestId = UUID.randomUUID().toString();
        int status = ProviderJobStatusEnum.FAIL.getStatus();
        TaskCompleteMessageBo givenMessage = buildMessage(givenRequestId, givenRequestId, status);
        RestoreTask givenTask = RestoreTaskMocker.mockRestoreTask(givenRequestId);
        givenTask.setTaskId(givenRequestId);
        given(restoreTaskService.getRestoreTaskFromJob(givenRequestId)).willReturn(givenTask);
        // when
        taskCompleteHandler.onTaskCompleteFailed(givenMessage);
        // then
        verify(restoreTaskManager, times(1)).complete(any(), any());
    }

    /**
     * 用例场景：当任务是副本校验子任务时，执行副本校验完成逻辑 <br>
     * 前置条件：参数正确 <br>
     * 检查点：消验证副本校验完成分支逻辑执行一次 <br>
     */
    @Test
    public void should_execute_verify_complete_when_failed_given_task_is_verify_sub_task() {
        // given
        String givenRequestId = UUID.randomUUID().toString();
        String givenTaskId = UUID.randomUUID().toString();
        int status = ProviderJobStatusEnum.FAIL.getStatus();
        TaskCompleteMessageBo givenMessage = buildMessage(givenRequestId, givenTaskId, status);
        RestoreTask givenTask = RestoreTaskMocker.mockRestoreTask(givenRequestId);
        givenTask.setTaskId(givenRequestId);
        given(restoreTaskService.getRestoreTaskFromJob(givenRequestId)).willReturn(givenTask);
        // when
        taskCompleteHandler.onTaskCompleteFailed(givenMessage);
        // then
        verify(copyVerifyTaskManager, times(1)).complete(any(), any());
    }

    /**
     * 用例场景：验证可以正确匹配该恢完成处理器 <br>
     * 前置条件：参数正确 <br>
     * 检查点：返回值为true <br>
     */
    @Test
    public void should_return_true_when_applicable() {
        Assert.assertTrue(taskCompleteHandler.applicable("RESTORE-v2"));
    }


    private static TaskCompleteMessageBo buildMessage(String givenRequestId, String givenTaskId,
        int status) {
        TaskCompleteMessageBo givenMessage = new TaskCompleteMessageBo();
        givenMessage.setJobRequestId(givenRequestId);
        givenMessage.setTaskId(givenTaskId);
        givenMessage.setJobStatus(status);
        return givenMessage;
    }
}
