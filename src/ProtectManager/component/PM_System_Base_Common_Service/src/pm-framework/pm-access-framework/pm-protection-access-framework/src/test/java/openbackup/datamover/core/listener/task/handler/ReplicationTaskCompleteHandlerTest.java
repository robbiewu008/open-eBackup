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
package openbackup.datamover.core.listener.task.handler;

import static com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants.EXTERNAL_SYSTEM_ID;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.powermock.api.mockito.PowerMockito.when;

import com.huawei.oceanprotect.base.cluster.sdk.entity.TargetCluster;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.MultiClusterAuthenticationService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;

import com.fasterxml.jackson.databind.JsonNode;

import openbackup.data.access.framework.core.dao.CopyMapper;
import openbackup.data.access.framework.protection.handler.v1.replication.ReplicationCopyProcessor;
import openbackup.data.access.framework.protection.handler.v1.replication.ReplicationTaskCompleteHandler;
import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.bean.CopiesEntity;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.JobSpeedConverter;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.cluster.model.TokenResponse;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.ScheduleBo;
import openbackup.system.base.util.MessageTemplate;
import openbackup.system.base.util.ProviderRegistry;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RList;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.test.util.ReflectionTestUtils;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.atomic.AtomicStampedReference;

/**
 * ReplicationTaskCompleteHandler LLT
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(ReplicationTaskCompleteHandler.class)
@AutoConfigureMockMvc
public class ReplicationTaskCompleteHandlerTest {
    private final EncryptorService encryptorService = Mockito.mock(EncryptorService.class);
    @Mock
    private RedissonClient redissonClient;

    @Mock
    private ProviderRegistry registry;

    @Mock
    private MessageTemplate<?> messageTemplate;

    @Mock
    private JobCenterRestApi jobCenterRestApi;

    @InjectMocks
    private ReplicationTaskCompleteHandler replicationTaskCompleteHandler;

    @Mock
    private SlaQueryService slaQueryService;

    @Mock
    private ClusterService clusterService;

    @Mock
    private MultiClusterAuthenticationService multiClusterAuthenticationService;

    @Mock
    private TaskCompleteMessageBo taskCompleteMessage;

    @Mock
    private ProtectedObject protectedObject;

    @Mock
    private PolicyBo policyBo;

    @Mock
    private PolicyDto policyDto;

    @Mock
    private ScheduleBo schedule;

    @Mock
    private CopyMapper copyMapper;

    @Mock
    private TargetCluster targetCluster;

    @Mock
    private RMap<String, String> rMap;

    @Mock
    private RList rList;

    @Mock
    private JsonNode extParameters;

    private String policyStr = "{\"uuid\": \"74f79253-c6b6-4f96-9ae4-73336f8c6bd5\", \"name\": \"\\u590d\\u523601\", \"sla_id\": \"cf5e2ac7-34d9-48d1-ac7a-a462451f33e2\", \"type\": \"replication\", \"action\": \"replication\", \"worm_validity_type\": null, \"retention\": {\"retention_type\": 2, \"retention_duration\": 3, \"duration_unit\": \"d\", \"worm_retention_duration\": null, \"worm_duration_unit\": null, \"retention_quantity\": null, \"daily_copies\": null, \"weekly_copies\": null, \"monthly_copies\": null, \"yearly_copies\": null}, \"schedule\": {\"trigger\": 2, \"interval\": null, \"interval_unit\": null, \"start_time\": \"2025-02-13T20:36:20\", \"end_time\": null, \"window_start\": null, \"window_end\": null, \"days_of_week\": null, \"days_of_month\": null, \"days_of_year\": null, \"trigger_action\": null}, \"ext_parameters\": {\"replication_target_type\": 1, \"local_storage_type\": \"OceanProtectX\", \"remote_storage_type\": \"OceanProtectX\", \"qos_id\": \"\", \"replication_storage_type\": 2, \"external_system_id\": \"6\", \"link_deduplication\": true, \"link_compression\": true, \"alarm_after_failure\": true, \"start_replicate_time\": \"2025-02-13 20:36:02\", \"replication_target_mode\": 1, \"storage_info\": {\"storage_id\": \"bc46d0a5-3a92-4027-9d45-edee4ec3ddfa\", \"storage_type\": \"storage_unit\"}, \"user_info\": {\"user_id\": \"88a94c476f12a21e016f12a246e50009\", \"username\": \"sysadmin\", \"userType\": \"COMMON\", \"password\":\"11\"}, \"has_upgrade\": true}, \"active\": true, \"is_active\": true}";

    @Before
    public void setUp() {
        // 初始化mock对象
        PolicyBo policyBo1 = JSONObject.fromObject(policyStr).toBean(PolicyBo.class);
        JsonNode extParameters = policyBo1.getExtParameters();
        when(policyBo.getUuid()).thenReturn("74f79253-c6b6-4f96-9ae4-73336f8c6bd5");
        when(policyDto.getExtParameters()).thenReturn(extParameters);
        when(clusterService.getClusterByClusterId(anyInt())).thenReturn(targetCluster);
        when(slaQueryService.queryPolicyById(any())).thenReturn(policyDto);
        when(encryptorService.decrypt(any())).thenReturn("");
        TokenResponse tokenResponse = new TokenResponse();
        when(multiClusterAuthenticationService.getTokenByTargetClusterUsername(any())).thenReturn(tokenResponse);
        List<CopiesEntity> copiesEntities = new ArrayList<>();
        copiesEntities.add(CopiesEntity.builder().build());
        when(copyMapper.selectList(any())).thenReturn(copiesEntities);
        when(redissonClient.getList(anyString())).thenReturn(rList);
    }

    /**
     * 用例场景：policy参数齐全时，方法全部被调用
     * 前置条件：policy参数齐全
     * 检查点: 方法至少调用1次
     */
    @Test
    public void testWormRepCopyDeal_WhenPolicyIsNotEmpty_ShouldRun()
        throws NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        // 准备测试数据
        String requestId = "requestId";
        when(rMap.get("policy")).thenReturn(policyStr);
        when(rList.addAll(any())).thenReturn(true);

        Method processJobIdListMethod = ReplicationTaskCompleteHandler.class.getDeclaredMethod("wormRepCopyDeal",
            TaskCompleteMessageBo.class, String.class, RMap.class, ProtectedObject.class);
        processJobIdListMethod.setAccessible(true);
        processJobIdListMethod.invoke(replicationTaskCompleteHandler, taskCompleteMessage, requestId, rMap, protectedObject);
        // 执行方法
        // 验证
        Mockito.verify(slaQueryService, Mockito.atLeastOnce()).queryPolicyById(Mockito.any());
        Mockito.verify(clusterService, Mockito.atLeastOnce()).getClusterByClusterId(Mockito.anyInt());
        Mockito.verify(multiClusterAuthenticationService, Mockito.atLeastOnce()).isExistWormPolicyByResourceId(Mockito.any(), Mockito.any(), Mockito.any());
    }

    /**
     * 用例场景：policy为空时，代码直接返回，不执行逻辑
     * 前置条件：复制任务完成
     * 检查点: 后续方法未被调用
     */
    @Test
    public void testWormRepCopyDeal_WhenPolicyIsNotEmpty_ShouldLogAndReturn1()
        throws NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        // 准备测试数据
        String requestId = "requestId";
        when(rMap.get("policy")).thenReturn("");
        when(rList.addAll(any())).thenReturn(true);

        Method processJobIdListMethod = ReplicationTaskCompleteHandler.class.getDeclaredMethod("wormRepCopyDeal",
            TaskCompleteMessageBo.class, String.class, RMap.class, ProtectedObject.class);
        processJobIdListMethod.setAccessible(true);
        processJobIdListMethod.invoke(replicationTaskCompleteHandler, taskCompleteMessage, requestId, rMap, protectedObject);
        // 执行方法
        // 验证
        Mockito.verify(slaQueryService, Mockito.never()).queryPolicyById(Mockito.any());
        Mockito.verify(clusterService, Mockito.never()).getClusterByClusterId(Mockito.anyInt());
        Mockito.verify(multiClusterAuthenticationService, Mockito.never()).isExistWormPolicyByResourceId(Mockito.any(), Mockito.any(), Mockito.any());
    }

    /**
     * 用例场景：不是备份完立刻复制，代码直接返回，不执行逻辑
     * 前置条件：不是备份完立刻复制
     * 检查点: isExistWormPolicyByResourceId方法未被调用
     */
    @Test
    public void testWormRepCopyDeal_WhenRemoteHasWormPolicy_ShouldLogAndReturn()
        throws InvocationTargetException, IllegalAccessException, NoSuchMethodException {
        // 准备测试数据
        String policy = "{\"schedule\":{\"trigger\":\"1\"},\"ext_parameters\": {\"external_system_id\": 1}}";
        String requestId = "requestId";
        when(rMap.get("policy")).thenReturn(policy);

        when(policyBo.getSchedule()).thenReturn(schedule);
        when(policyBo.getExtParameters()).thenReturn(extParameters);

        when(extParameters.get(EXTERNAL_SYSTEM_ID)).thenReturn(extParameters);
        when(extParameters.asInt()).thenReturn(2);
        when(policyDto.getExtParameters()).thenReturn(extParameters);
        when(multiClusterAuthenticationService.isExistWormPolicyByResourceId(any(), any(), any())).thenReturn(true);

        Method processJobIdListMethod = ReplicationTaskCompleteHandler.class.getDeclaredMethod("wormRepCopyDeal",
            TaskCompleteMessageBo.class, String.class, RMap.class, ProtectedObject.class);
        processJobIdListMethod.setAccessible(true);
        processJobIdListMethod.invoke(replicationTaskCompleteHandler, taskCompleteMessage, requestId, rMap, protectedObject);

        // 验证
        Mockito.verify(multiClusterAuthenticationService, Mockito.never()).isExistWormPolicyByResourceId(any(), any(), any());
    }

    @Test
    public void testOnTaskCompleteSuccess() {
        TaskCompleteMessageBo taskCompleteMessageBo = new TaskCompleteMessageBo();
        taskCompleteMessageBo.setJobRequestId(UUID.randomUUID().toString());
        taskCompleteMessageBo.setJobId(UUID.randomUUID().toString().replace("-", ""));
        taskCompleteMessageBo.setJobProgress(100);
        taskCompleteMessageBo.setJobStatus(3);

        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();

        RMap map = PowerMockito.mock(RMap.class);
        when(redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE))).thenReturn(map);
        when(map.put(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);
        TargetClusterVo targetClusterVo = new TargetClusterVo();
        targetClusterVo.setClusterId("1");
        when(map.get(ArgumentMatchers.eq("target_cluster"))).thenReturn(targetClusterVo);
        ReplicationCopyProcessor processor = PowerMockito.mock(ReplicationCopyProcessor.class);
        when(registry.findProvider(ArgumentMatchers.any(),ArgumentMatchers.any(),ArgumentMatchers.any())).thenReturn(processor);
        AtomicStampedReference<Boolean> stampedReference = new AtomicStampedReference<Boolean>(true,0);
        when(processor.process(ArgumentMatchers.any())).thenReturn(stampedReference);
        replicationTaskCompleteHandler.onTaskCompleteSuccess(taskCompleteMessageBo);
        replicationTaskCompleteHandler.onTaskCompleteFailed(taskCompleteMessageBo);
    }

    @Test
    public void testApplicable() {
        boolean applicable = replicationTaskCompleteHandler.applicable("copy_replication");
        Assert.assertTrue(applicable);
        applicable = replicationTaskCompleteHandler.applicable("copy_replication-v2");
        Assert.assertTrue(applicable);
    }

    /**
     * 用例场景：复制任务完成参数组成成功
     * 前置条件：复制任务完成
     * 检查点: 参数组装成功，速度正确
     */
    @Test
    public void test_recordeReplicatedCopyNumber() {
        String jobId = "78309289-f90e-4b94-9905-d1e01ebc6765";
        RMap map = PowerMockito.mock(RMap.class);
        when(redissonClient.getMap(jobId, StringCodec.INSTANCE)).thenReturn(map);
        when(map.get("job_status")).thenReturn("SUCCESS");
        when(map.get("job_id")).thenReturn(jobId);
        TaskCompleteMessageBo completeMessageBo = new TaskCompleteMessageBo();
        completeMessageBo.setSpeed(1025L);
        completeMessageBo.setTaskId(jobId);
        completeMessageBo.setJobStatus(3);
        completeMessageBo.setJobProgress(95);
        completeMessageBo.setJobRequestId(jobId);
        ReflectionTestUtils.invokeMethod(replicationTaskCompleteHandler, "recordeReplicatedCopyNumber",
            completeMessageBo, 1);
        ArgumentCaptor<UpdateJobRequest> argumentCaptor = ArgumentCaptor.forClass(UpdateJobRequest.class);
        Mockito.verify(jobCenterRestApi, Mockito.times(1)).updateJob(anyString(), argumentCaptor.capture());
        UpdateJobRequest value = argumentCaptor.getValue();
        Assert.assertEquals(value.getSpeed(),
            JobSpeedConverter.convertJobSpeed(String.valueOf(completeMessageBo.getSpeed())));
    }
}
