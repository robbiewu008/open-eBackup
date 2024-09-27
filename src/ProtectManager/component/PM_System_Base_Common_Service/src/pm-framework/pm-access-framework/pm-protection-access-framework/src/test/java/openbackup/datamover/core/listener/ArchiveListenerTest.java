/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.datamover.core.listener;

import com.github.jknack.handlebars.internal.text.StringEscapeUtils;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.listener.v1.archive.ArchiveListener;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.archive.ArchiveProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import com.huawei.oceanprotect.functionswitch.template.service.FunctionSwitchService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.Codec;
import org.redisson.client.codec.StringCodec;
import org.springframework.boot.SpringBootConfiguration;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.UUID;

/**
 * CopyListener LLT
 *
 * @author m00576658
 * @since 2021-03-05
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@SpringBootConfiguration
public class ArchiveListenerTest {
    @Mock
    private ProviderManager registry;

    @Mock
    private RedissonClient redissonClient;

    @Mock
    private FunctionSwitchService functionSwitchService;

    @Mock
    private UserQuotaManager userQuotaManager;

    @Mock
    private CopyRestApi copyRestApi;

    @InjectMocks
    private ArchiveListener archiveListener;

    @Test
    public void testArchiving() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
        JSONObject data = new JSONObject();

        data.set("request_id", UUID.randomUUID().toString());
        RMap map = PowerMockito.mock(RMap.class);
        String policy = StringEscapeUtils.unescapeJava(
            "{\\\"uuid\\\":\\\"73638461-f61b-40e4-a003-147e3e8c6e54\\\",\\\"name\\\":\\\"策略0\\\",\\\"type\\\":\\\"archiving\\\",\\\"action\\\":\\\"archiving\\\",\\\"retention\\\":{\\\"retention_type\\\":2,\\\"retention_duration\\\":10,\\\"duration_unit\\\":\\\"d\\\",\\\"retention_quantity\\\":null,\\\"daily_copies\\\":null,\\\"weekly_copies\\\":null,\\\"monthly_copies\\\":null,\\\"yearly_copies\\\":null},\\\"schedule\\\":{\\\"trigger\\\":2,\\\"interval\\\":null,\\\"interval_unit\\\":null,\\\"start_time\\\":null,\\\"end_time\\\":null,\\\"window_start\\\":null,\\\"window_end\\\":null,\\\"days_of_week\\\":null,\\\"days_of_month\\\":null,\\\"days_of_year\\\":null,\\\"trigger_action\\\":null},\\\"ext_parameters\\\":{\\\"qos_id\\\":\\\"\\\",\\\"protocol\\\":2,\\\"auto_index\\\":false,\\\"storage_id\\\":\\\"ca9018af2b414c22998c3f5b2455e651\\\",\\\"archiving_scope\\\":\\\"latest\\\",\\\"network_access\\\":false,\\\"auto_retry\\\":false,\\\"archive_target_type\\\":1,\\\"alarm_after_failure\\\":false},\\\"active\\\":true,\\\"is_active\\\":true}");
        PowerMockito.when(map.get("policy")).thenReturn(policy);
        PowerMockito.when(redissonClient.getMap(Mockito.any(), (Codec) Mockito.any())).thenReturn(map);
        PowerMockito.when(map.put(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);
        ArchiveProvider provider = PowerMockito.mock(ArchiveProvider.class);
        PowerMockito.when(registry.findProvider(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(provider);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUserId("userId");
        Copy copy = new Copy();
        copy.setUuid("copyId1");
        copy.setUserId("userId1");
        PowerMockito.when(copyRestApi.queryCopyByID(Mockito.any())).thenReturn(copy);
        PowerMockito.when(userQuotaManager.getUserId(Mockito.any(),Mockito.any())).thenReturn("userId");
        archiveListener.archiving(data.toString(), acknowledgment);
        Mockito.verify(redissonClient, Mockito.times(1)).getMap((String) data.get("request_id"), StringCodec.INSTANCE);
    }
}
