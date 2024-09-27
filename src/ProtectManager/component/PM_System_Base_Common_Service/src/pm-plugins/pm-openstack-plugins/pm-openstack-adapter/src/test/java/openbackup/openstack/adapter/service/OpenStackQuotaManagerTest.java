package openbackup.openstack.adapter.service;

import static org.assertj.core.api.Assertions.assertThat;

import openbackup.openstack.adapter.constants.OpenStackErrorCodes;
import openbackup.openstack.adapter.dto.OpenStackQuotaDto;
import openbackup.openstack.adapter.exception.OpenStackException;
import openbackup.openstack.adapter.service.OpenStackQuotaManager;
import openbackup.openstack.adapter.service.OpenStackUserManager;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import com.huawei.oceanprotect.system.base.quota.enums.UserQuotaErrorCode;
import com.huawei.oceanprotect.system.base.quota.po.UserQuota;
import com.huawei.oceanprotect.system.base.quota.po.UserQuotaPo;
import com.huawei.oceanprotect.system.base.quota.service.UserQuotaService;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.Collections;
import java.util.List;

/**
 * {@link OpenStackQuotaManager} 测试类
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-28
 */
public class OpenStackQuotaManagerTest {
    private final UserQuotaService quotaService = Mockito.mock(UserQuotaService.class);
    private final OpenStackUserManager userManager = Mockito.mock(OpenStackUserManager.class);
    private final OpenStackQuotaManager quotaManager = new OpenStackQuotaManager(quotaService, userManager);

    /**
     * 用例场景：设置配额成功
     * 前置条件：无
     * 检查点： 1.设置配额接口正常调用；2.GB成功转换为B
     */
    @Test
    public void should_setQuotaSuccess_when_setQuota() {
        String userId = UUIDGenerator.getUUID();
        Mockito.when(userManager.obtainUserId()).thenReturn(userId);

        String projectId = UUIDGenerator.getUUID();
        OpenStackQuotaDto quota = new OpenStackQuotaDto();
        quota.setSize(2);

        quotaManager.setQuota(projectId, quota);
        UserQuota userQuota = new UserQuota();
        userQuota.setUserId(userId);
        userQuota.setResourceId(projectId);
        userQuota.setBackupTotalQuota(2L * 1024 * 1024 * 1024);
        userQuota.setCloudArchiveTotalQuota(-1L);
        Mockito.verify(quotaService, Mockito.times(1)).setUserQuota(userQuota);
    }

    /**
     * 用例场景：设置配额时，如果配额小于已占用空间，抛出指定异常
     * 前置条件：待设置配额小于已占用空间
     * 检查点： 待设置配额小于已占用空间时，应抛出异常
     */
    @Test
    public void should_throwOpenStackException_when_setQuota_given_userTotalQuotaNotEnoughException() {
        String userId = UUIDGenerator.getUUID();
        Mockito.when(userManager.obtainUserId()).thenReturn(userId);

        String projectId = UUIDGenerator.getUUID();
        OpenStackQuotaDto quota = new OpenStackQuotaDto();
        quota.setSize(2);

        UserQuota userQuota = new UserQuota();
        userQuota.setUserId(userId);
        userQuota.setResourceId(projectId);
        userQuota.setBackupTotalQuota(2L * 1024 * 1024 * 1024);
        userQuota.setCloudArchiveTotalQuota(-1L);
        Mockito.when(quotaService.setUserQuota(userQuota))
                .thenThrow(new LegoCheckedException(UserQuotaErrorCode.USER_TOTAL_QUOTA_NOT_ENOUGH.getCode()));
        OpenStackException exception =
                Assert.assertThrows(OpenStackException.class, () -> quotaManager.setQuota(projectId, quota));
        assertThat(exception.getErrorCode()).isEqualTo(OpenStackErrorCodes.INIT_QUOTA_LESS_THAN_USED);
        assertThat(exception.getMessage())
                .isEqualTo(
                        String.format(
                                "Set project: %s of user: %s quota: %s fail.",
                                projectId, userId, 2L * 1024 * 1024 * 1024));
    }

    /**
     * 用例场景：设置配额时，如果配额有问题
     * 前置条件：待设置配额小于已占用空间
     * 检查点： 待设置配额小于已占用空间时，应抛出异常
     */
    @Test
    public void should_throwLegoCheckedException_when_setQuota() {
        String userId = UUIDGenerator.getUUID();
        Mockito.when(userManager.obtainUserId()).thenReturn(userId);

        String projectId = UUIDGenerator.getUUID();
        OpenStackQuotaDto quota = new OpenStackQuotaDto();
        quota.setSize(2);

        UserQuota userQuota = new UserQuota();
        userQuota.setUserId(userId);
        userQuota.setResourceId(projectId);
        userQuota.setBackupTotalQuota(2L * 1024 * 1024 * 1024);
        userQuota.setCloudArchiveTotalQuota(-1L);
        Mockito.when(quotaService.setUserQuota(userQuota))
                .thenThrow(new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM));
        LegoCheckedException exception =
                Assert.assertThrows(LegoCheckedException.class, () -> quotaManager.setQuota(projectId, quota));
        assertThat(exception.getErrorCode()).isEqualTo(CommonErrorCode.ILLEGAL_PARAM);
    }

    /**
     * 用例场景：查询配额时，返回正确的配额信息
     * 前置条件：存在配额
     * 检查点： 返回配额信息正确
     */
    @Test
    public void should_returnOneOpenStackQuotaWithCorrectValue_when_queryQuota() {
        String projectId = UUIDGenerator.getUUID();
        UserQuotaPo userQuota = new UserQuotaPo();
        userQuota.setBackupTotalQuota(2L * 1024 * 1024 * 1024 + 2048);
        userQuota.setBackupUsedQuota((long) 1024 * 1024 * 1024 + 1024);

        Mockito.when(quotaService.listUserQuotaInfoByResourceId(Collections.singletonList(projectId)))
                .thenReturn(Collections.singletonList(userQuota));

        List<OpenStackQuotaDto> quota = quotaManager.getQuota(projectId);
        assertThat(quota)
                .hasSize(1)
                .element(0)
                .hasFieldOrPropertyWithValue("all", 2)
                .hasFieldOrPropertyWithValue("used", 1);
    }

    /**
     * 用例场景：查询配额时，返回正确的配额信息
     * 前置条件：存在配额
     * 检查点： 返回配额信息正确
     */
    @Test
    public void should_returnNoLimitQuota_when_queryQuota_given_userQuotaNegativeOne() {
        String projectId = UUIDGenerator.getUUID();
        UserQuotaPo userQuota = new UserQuotaPo();
        userQuota.setBackupTotalQuota(-1);
        userQuota.setBackupUsedQuota((long) 1024 * 1024 * 1024 + 1024);

        Mockito.when(quotaService.listUserQuotaInfoByResourceId(Collections.singletonList(projectId)))
                .thenReturn(Collections.singletonList(userQuota));

        List<OpenStackQuotaDto> quota = quotaManager.getQuota(projectId);
        assertThat(quota)
                .hasSize(1)
                .element(0)
                .hasFieldOrPropertyWithValue("all", -1)
                .hasFieldOrPropertyWithValue("used", 1);
    }
}
