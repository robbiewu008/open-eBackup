package openbackup.data.access.framework.copy.mng.service;

import java.util.HashMap;
import java.util.Map;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.Codec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import openbackup.data.access.client.sdk.api.framework.dee.DeeFsSnapshotRestApi;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.copy.mng.service.impl.FsSnapshotServiceImpl;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.CopyConstants;
import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.repository.tapelibrary.common.util.JsonUtil;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.RetentionBo;
import openbackup.system.base.sdk.protection.model.SlaBo;

import static org.mockito.ArgumentMatchers.any;

/**
 * 安全一体机创建快照单元测试
 *
 * @author q00564609
 * @since 2024-06-14
 * @version OceanCyber 300 1.2.0
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {FsSnapshotServiceImpl.class})
public class FsSnapshotServiceImplTest {
    @Autowired
    private FsSnapshotServiceImpl fsSnapshotService;

    @MockBean
    private MemberClusterService memberClusterService;

    @MockBean
    private RedissonClient redissonClient;

    @MockBean
    private DeeFsSnapshotRestApi deeFsSnapshotRestApi;

    @MockBean
    private JobService jobService;

    /**
     * 用例名称：执行创建快照
     * 前置条件：无
     * check点：调用创建快照，不出错
     */
    @Test
    public void create_fs_snapshot_success() {
        BackupObject backupObject = buildBackupObject();
        ProtectedResource protectedResource = buildProtectedResource();

        RMap rMap = Mockito.mock(RMap.class);
        PolicyBo policyBo = new PolicyBo();
        RetentionBo retention = new RetentionBo();
        retention.setDurationUnit("y");
        retention.setRetentionDuration(1);
        retention.setRetentionType(6);
        policyBo.setRetention(retention);
        rMap.put(CopyConstants.POLICY, JsonUtil.toJsonString(policyBo));
        rMap.put(ContextConstants.COPY_FORMAT, "0");
        rMap.put(CopyPropertiesKeyConstant.KEY_BACKUP_CHAIN_ID, "chainId");
        SlaBo slaBo = new SlaBo();
        slaBo.setName("test");
        slaBo.setUuid("uuid1");
        slaBo.setUserId("userId1");
        slaBo.setType("1");
        rMap.put(CopyConstants.SLA, JsonUtil.toJsonString(slaBo));
        rMap.put(CopyConstants.COPY_NAME, "test");
        Mockito.when(redissonClient.getMap(any(), any(Codec.class))).thenReturn(rMap);

        PowerMockito.when(memberClusterService.getCurrentClusterEsn()).thenReturn("esn1");
        PowerMockito.doNothing().when(deeFsSnapshotRestApi).createFsSnapshot(ArgumentMatchers.any());
        PowerMockito.doNothing().when(jobService).updateJob(ArgumentMatchers.any(), ArgumentMatchers.any());

        fsSnapshotService.oceanCyberDetectBackup(backupObject, protectedResource);
        Mockito.verify(jobService, Mockito.times(1)).updateJob(Mockito.any(), Mockito.any());
    }

    private BackupObject buildBackupObject() {
        BackupObject backupObject = new BackupObject();
        backupObject.setRequestId("taskId1");
        backupObject.setTaskId("taskId1");
        backupObject.setBackupType("backup");
        return backupObject;
    }

    private ProtectedResource buildProtectedResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setRootUuid("rootUuid1");
        protectedResource.setName("test");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("filesystemId", "42");
        extendInfo.put("filesystemName", "test");
        extendInfo.put("snapshotId", "snap1");
        extendInfo.put("snapshotName", "snap1");
        extendInfo.put("multiFileSystem", "false");
        extendInfo.put("isSanClient", "false");
        extendInfo.put("tenantName", "System");
        protectedResource.setExtendInfo(extendInfo);

        return protectedResource;
    }
}
