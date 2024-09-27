/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.copy.mng.handler.v2;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.copy.mng.handler.v2.UnifiedCopyDeleteCompleteHandler;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.enums.DmcJobStatus;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.copy.index.service.impl.UnifiedCopyIndexService;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

/**
 * 统一副本删除任务完成处理器测试类
 *
 * @author twx1009756
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-01-27
 */
@SpringBootTest
@RunWith(SpringRunner.class)
@ContextConfiguration(classes = {UnifiedCopyDeleteCompleteHandler.class})
public class UnifiedCopyDeleteCompleteHandlerTest {
    @MockBean
    private NotifyManager notifyManager;

    @MockBean
    private CopyRestApi copyRestApi;

    @MockBean
    private JobCenterRestApi jobCenterRestApi;

    @MockBean
    private UnifiedCopyIndexService unifiedCopyIndexService;

    @MockBean
    private JobService jobService;

    @MockBean
    private UserQuotaManager userQuotaManager;

    @Autowired
    private UnifiedCopyDeleteCompleteHandler handler;

    @MockBean
    private CopyManagerService copyManagerService;

    @MockBean
    private ProviderManager providerManager;

    /**
     * 用例场景：测试是否能够进入处理副本删除任务成功场景
     * 前置条件：封装拦截层方法参数
     * 检查点：验证是否能够进入副本删除任务成功处理器, 校验经过了任务完成处理的值是否与期望值相同
     */
    @Test
    public void test_handle_copy_delete_success_message() {
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(buildCopyInfo());
        Mockito.doNothing().when(userQuotaManager).decreaseUsedQuota(anyString(), any());
        TaskCompleteMessageBo taskMessage = buildTaskMessage();
        taskMessage.setJobStatus(DmcJobStatus.SUCCESS.getStatus());
        handler.onTaskCompleteSuccess(taskMessage);
        Assert.assertTrue(JSONObject.fromObject(taskMessage.getContext()).getBoolean("backup_damaged"));
        Assert.assertEquals(JSONObject.fromObject(taskMessage.getContext()).get("copy_id"), "123456789");
        Assert.assertEquals(JSONObject.fromObject(taskMessage.getContext()).get("request_id"), "request_id");
        Assert.assertEquals(JSONObject.fromObject(taskMessage.getContext()).get("job_type"), "COPY_DELETE");
        Assert.assertEquals(JSONObject.fromObject(taskMessage.getExtendsInfo()).get("relative_copies"),
            Arrays.asList("123", "456"));

        TaskCompleteMessageBo taskMessageNoExt = buildTaskMessageNoExt();
        taskMessageNoExt.setJobStatus(DmcJobStatus.SUCCESS.getStatus());
        handler.onTaskCompleteSuccess(taskMessageNoExt);
        Assert.assertEquals(JSONObject.fromObject(taskMessage.getContext()).get("job_type"), "COPY_DELETE");
        Assert.assertEquals(JSONObject.fromObject(taskMessage.getContext()).get("copy_id"), "123456789");
        Assert.assertEquals(JSONObject.fromObject(taskMessage.getContext()).get("request_id"), "request_id");
    }

    /**
     * 用例场景：测试是否能够进入处理副本删除任务失败场景
     * 前置条件：封装拦截层方法参数
     * 检查点：验证是否能够进入副本删除任务失败处理器, 校验经过了任务完成处理的值是否与期望值相同
     */
    @Test
    public void test_handle_copy_delete_failed_message() {
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(buildCopyInfo());
        Mockito.doNothing().when(userQuotaManager).decreaseUsedQuota(anyString(), any());
        TaskCompleteMessageBo taskMessage = buildTaskMessage();
        taskMessage.setJobStatus(DmcJobStatus.FAIL.getStatus());
        handler.onTaskCompleteFailed(taskMessage);
        Assert.assertTrue(JSONObject.fromObject(taskMessage.getContext()).getBoolean("backup_damaged"));
        Assert.assertEquals(JSONObject.fromObject(taskMessage.getContext()).get("copy_id"), "123456789");
        Assert.assertEquals(JSONObject.fromObject(taskMessage.getContext()).get("request_id"), "request_id");
        Assert.assertEquals(JSONObject.fromObject(taskMessage.getContext()).get("job_type"), "COPY_DELETE");
        Assert.assertEquals(JSONObject.fromObject(taskMessage.getExtendsInfo()).get("relative_copies"),
            Arrays.asList("123", "456"));
        TaskCompleteMessageBo taskMessageNoExt = buildTaskMessageNoExt();
        taskMessageNoExt.setJobStatus(DmcJobStatus.FAIL.getStatus());
        handler.onTaskCompleteFailed(taskMessageNoExt);
        Assert.assertEquals(JSONObject.fromObject(taskMessage.getContext()).get("job_type"), "COPY_DELETE");
        Assert.assertEquals(JSONObject.fromObject(taskMessage.getContext()).get("copy_id"), "123456789");
        Assert.assertEquals(JSONObject.fromObject(taskMessage.getContext()).get("request_id"), "request_id");
    }

    /**
     * 用例场景：测试创建副本删除请求拦截层
     * 前置条件：封装拦截层方法参数
     * 检查点：验证是否能够进入副本删除拦截层
     */
    @Test
    public void test_copy_delete_intercept() {
        Assert.assertTrue(handler.applicable("COPY_DELETE-v2"));
        Assert.assertTrue(handler.applicable("COPY_EXPIRE-v2"));
        Assert.assertFalse(handler.applicable("unmount-v2"));
    }

    private TaskCompleteMessageBo buildTaskMessage() {
        TaskCompleteMessageBo taskMessage = new TaskCompleteMessageBo();
        Map<String, String> context = new HashMap<>();
        context.put(ContextConstants.JOB_TYPE, JobTypeEnum.COPY_DELETE.getValue());
        context.put("request_id", "request_id");
        context.put("copy_id", "123456789");
        context.put("backup_damaged", "true");
        taskMessage.setContext(context);
        Map extendInfo = new HashMap();
        extendInfo.put("relative_copies", Arrays.asList("123", "456"));
        taskMessage.setExtendsInfo(extendInfo);
        return taskMessage;
    }

    private TaskCompleteMessageBo buildTaskMessageNoExt() {
        TaskCompleteMessageBo taskMessage = new TaskCompleteMessageBo();
        Map<String, String> context = new HashMap<>();
        context.put(ContextConstants.JOB_TYPE, JobTypeEnum.COPY_DELETE.getValue());
        context.put("request_id", "request_id");
        context.put("copy_id", "123456789");
        taskMessage.setContext(context);
        return taskMessage;
    }

    private Copy buildCopyInfo() {
        Copy copy = new Copy();
        copy.setUuid("123456");
        copy.setStatus("Indexed");
        return copy;
    }
}
