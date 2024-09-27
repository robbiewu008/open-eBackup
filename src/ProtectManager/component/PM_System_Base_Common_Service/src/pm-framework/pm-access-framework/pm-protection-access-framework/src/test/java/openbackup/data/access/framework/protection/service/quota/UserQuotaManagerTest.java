/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.framework.protection.service.quota;

import com.huawei.oceanprotect.base.cluster.sdk.service.MultiClusterSummaryService;

import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;
import com.huawei.oceanprotect.system.base.quota.enums.QuotaTaskTypeEnum;
import com.huawei.oceanprotect.system.base.quota.enums.UpdateQuotaType;
import com.huawei.oceanprotect.system.base.quota.service.UserQuotaService;
import openbackup.system.base.sdk.copy.model.Copy;
import org.apache.logging.log4j.util.Strings;
import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.Optional;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

/**
 * {@link UserQuotaManager} 测试类
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-12
 */
public class UserQuotaManagerTest {
    private final UserQuotaService userQuotaService = Mockito.mock(UserQuotaService.class);

    private final MultiClusterSummaryService multiClusterSummaryService = Mockito.mock(
        MultiClusterSummaryService.class);
    private final ResourceService resourceService = Mockito.mock(
        ResourceService.class);

    private final UserQuotaManager userQuotaManager = new UserQuotaManager(userQuotaService,
        multiClusterSummaryService, resourceService);

    /**
     * 用例场景：用户备份配额足够，则校验成功
     * 前置条件：用户备份配额足够
     * 检查点： 用户备份配额足够，则校验成功
     */
    @Test
    public void should_checkSuccess_when_checkBackupQuota_given_userHasEnoughQuota() {
        String userId = UUIDGenerator.getUUID();
        String resourceId = UUIDGenerator.getUUID();
        String jobId = UUIDGenerator.getUUID();
        Mockito.doNothing()
            .when(userQuotaService)
            .checkUserQuotaInSrc(userId, resourceId, QuotaTaskTypeEnum.TASK_BACKUP, false);
        userQuotaManager.checkBackupQuota(userId, resourceId);
        Mockito.verify(userQuotaService, Mockito.times(1))
                .checkUserQuotaInSrc(userId, resourceId, QuotaTaskTypeEnum.TASK_BACKUP, false);
    }

    /**
     * 用例场景：用户备份配额不足，则抛出异常
     * 前置条件：用户备份配额不足
     * 检查点： 用户备份配额不足，则校验失败
     */
    @Test
    public void should_throwLegoCheckedException_when_checkBackupQuota_given_userHasNotEnoughQuota() {
        String userId = UUIDGenerator.getUUID();
        String resourceId = UUIDGenerator.getUUID();
        String jobId = UUIDGenerator.getUUID();
        Mockito.doThrow(new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "User has no quota to backup."))
            .when(userQuotaService)
            .checkUserQuotaInSrc(userId, resourceId, QuotaTaskTypeEnum.TASK_BACKUP, false);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> userQuotaManager.checkBackupQuota(userId, resourceId));
        assertThat(exception.getMessage()).isEqualTo("User has no quota to backup.");
    }

    /**
     * 用例场景：用户归档配额足够，则校验成功
     * 前置条件：用户归档配额足够
     * 检查点： 用户归档配额足够，则校验成功
     */
    @Test
    public void should_checkSuccess_when_checkCloudArchiveQuota_given_userHasEnoughQuota() {
        String userId = UUIDGenerator.getUUID();
        String resourceId = UUIDGenerator.getUUID();
        String jobId = UUIDGenerator.getUUID();
        Mockito.doNothing()
            .when(userQuotaService)
            .checkUserQuotaInSrc(userId, resourceId, QuotaTaskTypeEnum.TASK_ARCHIVE, false);
        userQuotaManager.checkCloudArchiveQuota(userId, resourceId);
        Mockito.verify(userQuotaService, Mockito.times(1))
                .checkUserQuotaInSrc(userId, resourceId, QuotaTaskTypeEnum.TASK_ARCHIVE, false);
    }

    /**
     * 用例场景：用户归档配额不足，则抛出异常
     * 前置条件：用户归档配额不足
     * 检查点： 用户归档配额不足，则校验失败
     */
    @Test
    public void should_throwLegoCheckedException_when_checkCloudArchiveQuota_given_userHasNotEnoughQuota() {
        String userId = UUIDGenerator.getUUID();
        String resourceId = UUIDGenerator.getUUID();
        String jobId = UUIDGenerator.getUUID();
        Mockito.doThrow(new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "User has no quota to archive."))
            .when(userQuotaService)
            .checkUserQuotaInSrc(userId, resourceId, QuotaTaskTypeEnum.TASK_ARCHIVE, false);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> userQuotaManager.checkCloudArchiveQuota(userId, resourceId));
        assertThat(exception.getMessage()).isEqualTo("User has no quota to archive.");
    }

    /**
     * 用例场景：SAML用户目标集群备份配额足够，则校验成功
     * 前置条件：SAML用户目标集群备份配额
     * 检查点： SAML用户目标集群备份配额，则校验成功
     */
    @Test
    public void should_check_success_when_check_backup__quota_in_target_cluster_given_userHasEnoughQuota() {
        Integer clusterId = 1;
        String userId = UUIDGenerator.getUUID();
        String resourceId = UUIDGenerator.getUUID();
        String jobId = UUIDGenerator.getUUID();
        Mockito.doNothing()
            .when(multiClusterSummaryService)
            .checkTargetQuota(clusterId, userId, Strings.EMPTY, QuotaTaskTypeEnum.TASK_BACKUP.value());
        userQuotaManager.checkCloudArchiveQuota(userId, resourceId);
        Mockito.verify(userQuotaService, Mockito.times(1))
                .checkUserQuotaInSrc(userId, resourceId, QuotaTaskTypeEnum.TASK_ARCHIVE, false);
    }

    /**
     * 用例场景：HCS用户目标集群复制配额足够，则校验成功
     * 前置条件：HCS用户目标集群复制配额
     * 检查点： HCS用户目标集群复制配额，则校验成功
     */
    @Test
    public void should_check_success_when_check_replcate_quota_in_target_cluster_given_userHasEnoughQuota() {
        Integer clusterId = 1;
        String userId = UUIDGenerator.getUUID();
        String resourceId = UUIDGenerator.getUUID();
        String jobId = UUIDGenerator.getUUID();
        Mockito.doNothing()
            .when(multiClusterSummaryService)
            .checkTargetQuota(clusterId, userId, Strings.EMPTY, QuotaTaskTypeEnum.TASK_REPLICATED.value());
        userQuotaManager.checkHcsUserReplicationQuota(clusterId, userId);
        Assert.assertTrue(true);
    }


    /**
     * 用例场景：测试如果副本为空，则不执行增加配额操作
     * 前置条件：副本不存在
     * 检查点：副本不存在的情况下，不增加用户配额
     */
    @Test
    public void test_shouldUpdateUserUsedQuotaZeroTime_when_increaseQuota_given_emptyCopy() {
        userQuotaManager.increaseUsedQuota(UUIDGenerator.getUUID(), null);
        Mockito.verify(userQuotaService, Mockito.times(0))
            .updateUserUsedQuota(anyString(), anyString(), any(), any(), anyString());
    }

    /**
     * 用例场景：测试如果副本存在，则执行增加配额操作
     * 前置条件：副本存在
     * 检查点：副本存在的情况下，增加用户配额
     */
    @Test
    public void test_shouldUpdateUserUsedQuotaOneTime_when_increaseQuota_given_copy() {
        String jobId = UUIDGenerator.getUUID();
        Copy copy = new Copy();
        copy.setUserId(UUIDGenerator.getUUID());
        copy.setGeneratedBy("Backup");
        String properties
            = "{\"metaPathSuffix\":\"\",\"dataPathSuffix\":\"\",\"isAggregation\":\"false\",\"dataAfterReduction\":39,\"format\":0,\"snapshots\":[{\"id\":\"702@aa551abb-1730-4e95-9375-e3adfedda8ff\",\"parentName\":\"Storage_7707428c5e5d4c4ba26f2db956bfb09f\"}],\"verifyStatus\":\"3\",\"repositories\":[{\"type\":2,\"protocol\":5,\"role\":0,\"remotePath\":[{\"type\":1,\"path\":\"/Storage_CacheDataRepository/7707428c5e5d4c4ba26f2db956bfb09f\",\"id\":\"39\"}],\"extendInfo\":{\"esn\":\"2102354DEY10MA000001\"}},{\"type\":1,\"protocol\":6,\"role\":0,\"remotePath\":[{\"type\":0,\"path\":\"/Storage_7707428c5e5d4c4ba26f2db956bfb09f/source_policy_7707428c5e5d4c4ba26f2db956bfb09f_Context_Global_MD\",\"id\":\"702\"},{\"type\":1,\"path\":\"/Storage_7707428c5e5d4c4ba26f2db956bfb09f/source_policy_7707428c5e5d4c4ba26f2db956bfb09f_Context\",\"id\":\"702\"}],\"extendInfo\":{\"esn\":\"2102354DEY10MA000001\",\"securityStyle\":\"2\",\"copy_format\":0}}],\"dataBeforeReduction\":616,\"maxSizeAfterAggregate\":\"0\",\"multiFileSystem\":\"false\",\"maxSizeToAggregate\":\"0\"}";
        JSONObject propertiesJson = JSONObject.fromObject(properties);
        propertiesJson.put("size", "240");
        copy.setProperties(properties);
        userQuotaManager.increaseUsedQuota(jobId, copy);
        Mockito.verify(userQuotaService, Mockito.times(1))
            .updateUserUsedQuota(copy.getUserId(), null, copy.getGeneratedBy(), UpdateQuotaType.INCREASE, "0");
    }

    /**
     * 用例场景：测试如果副本为空，则不执行减少配额操作
     * 前置条件：副本不存在
     * 检查点：副本不存在的情况下，不减少用户配额
     */
    @Test
    public void test_shouldUpdateUserUsedQuotaZeroTime_when_decreaseQuota_given_emptyCopy() {
        userQuotaManager.decreaseUsedQuota(UUIDGenerator.getUUID(), null);
        Mockito.verify(userQuotaService, Mockito.times(0))
            .updateUserUsedQuota(anyString(), anyString(), any(), any(), anyString());
    }

    /**
     * 用例场景：测试如果副本存在，则执行减少配额操作
     * 前置条件：副本存在
     * 检查点：副本存在的情况下，减少用户配额
     */
    @Test
    public void test_shouldUpdateUserUsedQuotaOneTime_when_decreaseQuota_given_copy() {
        String jobId = UUIDGenerator.getUUID();
        Copy copy = new Copy();
        copy.setUserId(UUIDGenerator.getUUID());
        copy.setGeneratedBy("Backup");
        userQuotaManager.decreaseUsedQuota(jobId, copy);
        Mockito.verify(userQuotaService, Mockito.times(1))
            .updateUserUsedQuota(copy.getUserId(), null, copy.getGeneratedBy(), UpdateQuotaType.REDUCE, "0");
    }

    /**
     * 用例场景：获取资源所属用户id
     * 前置条件：副本不存在
     * 检查点：副本不存在的情况下，不减少用户配额
     */
    @Test
    public void test_get_user_id_by_resource_or_root_resource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUserId("userId");
        protectedResource.setName("userName");
        Optional<ProtectedResource> protectedResource1 = Optional.of(protectedResource);
        PowerMockito.when(resourceService.getResourceById(Mockito.anyBoolean(),Mockito.any())).thenReturn(protectedResource1);
        String userId = userQuotaManager.getUserId("", "rootUserId");
        Assert.assertEquals(userId, "userId");
    }
}
