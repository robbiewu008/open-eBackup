/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.protection.service.replication;

import static org.mockito.ArgumentMatchers.any;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterQueryService;
import openbackup.data.access.client.sdk.api.framework.dmc.DmcCopyService;
import openbackup.data.access.client.sdk.api.framework.dme.DmeCopyInfo;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.dme.replicate.DmeReplicationRestApi;
import openbackup.data.access.framework.protection.service.replication.UnifiedReplicationProvider;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import com.huawei.oceanprotect.sla.infrastructure.repository.SlaRepositoryImpl;
import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.protection.QosCommonRestApi;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import com.huawei.oceanprotect.system.sdk.service.SystemSwitchInternalService;

import org.assertj.core.util.Maps;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.Collections;

/**
 * Unified Replication Provider Test
 *
 * @author l00272247
 * @since 2022-01-30
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@MockBean( {
    DmcCopyService.class, DmeReplicationRestApi.class, ClusterInternalApi.class, QosCommonRestApi.class,
    JobCenterRestApi.class, RedissonClient.class, CopyRestApi.class, DmeUnifiedRestApi.class, SlaQueryService.class,
    TaskRepositoryManager.class, SystemSwitchInternalService.class, SlaRepositoryImpl.class, ClusterNativeApi.class,
    BackupStorageApi.class, ClusterQueryService.class, AuthNativeApi.class
})
@SpringBootTest(classes = {UnifiedReplicationProvider.class})
public class UnifiedReplicationProviderTest {
    @Autowired
    private DmeUnifiedRestApi dmeUnifiedRestApi;

    @InjectMocks
    private UnifiedReplicationProvider unifiedReplicationProvider;

    /**
     * 用例名称：验证统一框架复制副本扩展属性构建。<br/>
     * 前置条件：复制任务执行成功。<br/>
     * check点：复制副本扩展属性构建正确；
     */
    @Test
    public void test_build_copy_properties() {
        CopyInfoBo copyInfoBo = new CopyInfoBo();
        String backupId = "backup-id";
        DmeCopyInfo dmeCopyInfo = new DmeCopyInfo();
        dmeCopyInfo.setExtendInfo(Maps.newHashMap("copyVerifyFile", "true"));
        dmeCopyInfo.setSnapshots(Collections.emptyList());
        dmeCopyInfo.setSize(115L);
        dmeCopyInfo.setRepositories(Collections.emptyList());
        PowerMockito.when(dmeUnifiedRestApi.getCopyInfo(any())).thenReturn(dmeCopyInfo);
        JSONObject properties = unifiedReplicationProvider.buildCopyProperties(copyInfoBo, backupId);
        Assert.assertTrue(properties.getJSONArray("snapshots").isEmpty());
        Assert.assertTrue(properties.getJSONArray("repositories").isEmpty());
        Assert.assertFalse(unifiedReplicationProvider.applicable(""));
        Assert.assertEquals(properties.get("size"), 115L);
    }
}
