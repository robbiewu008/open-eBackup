/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.datamover.core.listener.replication;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.controller.AccessPointController;
import openbackup.data.access.framework.protection.listener.v1.replication.ProtectionReplicationListener;
import openbackup.data.access.framework.protection.listener.v1.replication.ReplicationExtParam;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.replication.ReplicationProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.functionswitch.template.service.FunctionSwitchService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.enums.UserTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.resource.ResourceRestApi;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.service.ApplicationContextService;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import com.huawei.oceanprotect.system.base.user.service.UserService;

import com.fasterxml.jackson.databind.JsonNode;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.kafka.support.Acknowledgment;

import java.util.Collections;
import java.util.Optional;

/**
 * ProtectionReplicationListener LLT
 *
 * @author m00576658
 * @since 2021-03-02
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(AccessPointController.class)
public class ProtectionReplicationListenerTest {
    @Mock
    private ProviderManager registry;

    @Mock
    private RedissonClient redissonClient;

    @Mock
    private ClusterInternalApi clusterInternalApi;

    @Mock
    private ResourceService resourceService;

    @Mock
    private ResourceRestApi resourceRestApi;

    @Mock
    private ApplicationContextService applicationContextService;

    @Mock
    private UserService userService;

    @Mock
    private FunctionSwitchService functionSwitchService;

    @Mock
    private UserQuotaManager userQuotaManager;

    @Mock
    private JobService jobService;

    @Mock
    private ClusterService targetClusterService;

    @InjectMocks
    private ProtectionReplicationListener protectionReplicationListener;

    @Test
    public void replicate_success() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
        JSONObject data = new JSONObject();
        PolicyBo policy = new PolicyBo();
        policy.setName("test");
        ReplicationExtParam replicationExtParam = new ReplicationExtParam();
        replicationExtParam.setExternalSystemId("1");
        JsonNode jsonNode = JSONObject.cast(replicationExtParam, JsonNode.class);
        policy.setExtParameters(jsonNode);
        ResourceEntity resourceEntity = getResourceEntity(data, policy);
        PowerMockito.when(resourceRestApi.queryResourceById(any(), any())).thenReturn(resourceEntity);
        JobBo jobBo = new JobBo();
        jobBo.setStatus("READY");
        commonMock(jobBo);

        // mock replicationPreCheck
        UserInnerResponse userInnerResponse = new UserInnerResponse();
        userInnerResponse.setUserType(UserTypeEnum.SAML.getValue());
        PowerMockito.when(userService.getUserInfoByUserId(Mockito.any())).thenReturn(userInnerResponse);
        protectionReplicationListener.replicate(data.toString(), acknowledgment);
        PowerMockito.doThrow(new LegoCheckedException("User not exit!")).when(userService).getUserInfoByUserId(Mockito.any());
        protectionReplicationListener.replicate(data.toString(), acknowledgment);
        jobBo.setStatus("SUCCESS");
        protectionReplicationListener.replicate(data.toString(), acknowledgment);
        Mockito.verify(redissonClient, Mockito.times(3)).getMap("request_id", StringCodec.INSTANCE);
    }

    private void commonMock(JobBo jobBo) {
        PowerMockito.when(jobService.queryJob(any())).thenReturn(jobBo);
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(
                redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
            .thenReturn(map);
        PowerMockito.when(map.put(any(), any())).thenReturn(null);
        ReplicationProvider replicationProvider = PowerMockito.mock(ReplicationProvider.class);
        PowerMockito.when(registry.findProviderOrDefault(any(), any(), any())).thenReturn(replicationProvider);
        TargetClusterVo targetClusterVo = new TargetClusterVo();
        targetClusterVo.setClusterId("1");
        targetClusterVo.setMgrIpList(Collections.singletonList("127.0.0.1"));
        PowerMockito.when(clusterInternalApi.queryTargetClusterDetailsByClusterId(anyInt()))
            .thenReturn(targetClusterVo);
        PowerMockito.when(resourceService.getResourceById(any()))
            .thenReturn(Optional.ofNullable(new ProtectedResource()));
        PowerMockito.when(userQuotaManager.getUserId(Mockito.any(), Mockito.any())).thenReturn("userId");
    }

    private ResourceEntity getResourceEntity(JSONObject data, PolicyBo policy) {
        ResourceEntity resourceEntity = new ResourceEntity();
        resourceEntity.setName("test");
        resourceEntity.setUserId("userId");
        ProtectedObject protectedObject = new ProtectedObject();
        protectedObject.setName("test");
        data.set("request_id", "request_id");
        data.set("policy", JSONObject.fromObject(policy).toString());
        data.set("resource_obj", JSONObject.fromObject(resourceEntity).toString());
        data.set("protected_object", JSONObject.fromObject(protectedObject).toString());
        return resourceEntity;
    }

    /**
     * 用例名称：hcs复制校验
     * 前置条件：无
     * check点：1.没有发生异常
     */
    @Test
    public void replicate_success_hcs() {
        String policyJson = "{\n" + "  \"uuid\": \"fb32cfaa-76fb-4194-aedb-88ce224db7d9\",\n"
            + "  \"name\": \"复制01\",\n" + "  \"type\": \"replication\",\n" + "  \"action\": \"replication\",\n"
            + "  \"retention\": {\n" + "    \"retention_type\": 2,\n" + "    \"retention_duration\": 10,\n"
            + "    \"duration_unit\": \"d\",\n" + "    \"retention_quantity\": null,\n"
            + "    \"daily_copies\": null,\n" + "    \"weekly_copies\": null,\n" + "    \"monthly_copies\": null,\n"
            + "    \"yearly_copies\": null\n" + "  },\n" + "  \"schedule\": {\n" + "    \"trigger\": 2,\n"
            + "    \"interval\": null,\n" + "    \"interval_unit\": null,\n"
            + "    \"start_time\": \"2023-09-16T23:05:59\",\n" + "    \"end_time\": null,\n"
            + "    \"window_start\": null,\n" + "    \"window_end\": null,\n" + "    \"days_of_week\": null,\n"
            + "    \"days_of_month\": null,\n" + "    \"days_of_year\": null,\n" + "    \"trigger_action\": null\n"
            + "  },\n" + "  \"ext_parameters\": {\n" + "    \"replication_target_type\": 1,\n"
            + "    \"qos_id\": \"\",\n" + "    \"link_deduplication\": false,\n" + "    \"link_compression\": false,\n"
            + "    \"is_worm\": false,\n" + "    \"alarm_after_failure\": false,\n"
            + "    \"start_replicate_time\": \"2023-09-07 23:05:50\",\n" + "    \"replication_target_mode\": 1,\n"
            + "    \"region_code\": \"sa-fb-1\",\n" + "    \"project_id\": \"1fe2e8669dd94a4ab1a975171cb68e42\",\n"
            + "    \"external_system_id\": \"5\",\n" + "    \"has_upgrade\": true\n" + "  },\n"
            + "  \"active\": true,\n" + "  \"is_active\": true\n" + "}";
        PolicyBo policyBo = JSONObject.fromObject(policyJson).toBean(PolicyBo.class);
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
        JSONObject data = new JSONObject();
        ResourceEntity resourceEntity = getResourceEntity(data, policyBo);
        PowerMockito.when(resourceRestApi.queryResourceById(any(), any())).thenReturn(resourceEntity);
        JobBo jobBo = new JobBo();
        jobBo.setStatus("READY");
        commonMock(jobBo);
        UserInnerResponse userInnerResponse2 = new UserInnerResponse();
        userInnerResponse2.setUserType(UserTypeEnum.HCS.getValue());
        PowerMockito.when(userService.getUserInfoByUserId(Mockito.any())).thenReturn(userInnerResponse2);
        protectionReplicationListener.replicate(data.toString(), acknowledgment);
        Mockito.verify(redissonClient, Mockito.times(1)).getMap("request_id", StringCodec.INSTANCE);
    }

    @Test
    public void test_query_target_cluster() {
        PowerMockito.when(targetClusterService.getTargetClusterToken(anyInt()))
            .thenThrow(new LegoCheckedException(1677930013L, "Auth need init"));
        Assert.assertThrows(LegoCheckedException.class,
            () -> Whitebox.invokeMethod(protectionReplicationListener, "queryTargetCluster", "1"));
    }
}
