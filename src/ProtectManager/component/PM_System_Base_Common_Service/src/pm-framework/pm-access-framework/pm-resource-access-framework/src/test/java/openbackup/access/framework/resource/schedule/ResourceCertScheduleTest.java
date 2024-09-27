package openbackup.access.framework.resource.schedule;

import openbackup.access.framework.resource.schedule.DefaultResourceCertCheckProvider;
import openbackup.access.framework.resource.schedule.ResourceCertSchedule;
import openbackup.access.framework.resource.schedule.VmwareResourceCertCheckProvider;
import openbackup.access.framework.resource.util.ResourceCertUtil;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.pack.lock.Lock;
import openbackup.system.base.pack.lock.LockService;
import openbackup.system.base.sdk.alarm.CommonAlarmService;
import openbackup.system.base.service.DeployTypeService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Arrays;
import java.util.HashMap;
import java.util.concurrent.TimeUnit;

import static org.mockito.ArgumentMatchers.any;

/**
 * ResourceCertSchedule Test
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/10/14
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({ResourceCertUtil.class})
public class ResourceCertScheduleTest {
    private final ProviderManager providerManager = PowerMockito.mock(ProviderManager.class);

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final CommonAlarmService commonAlarmService = PowerMockito.mock(CommonAlarmService.class);

    private final EncryptorService encryptorService = PowerMockito.mock(EncryptorService.class);

    private final LockService lockService = PowerMockito.mock(LockService.class);

    private final DefaultResourceCertCheckProvider defaultResourceCertCheckProvider = PowerMockito.mock(DefaultResourceCertCheckProvider.class);

    private final ResourceCertSchedule resourceCertSchedule = new ResourceCertSchedule(resourceService, commonAlarmService, encryptorService, providerManager, defaultResourceCertCheckProvider);

    /**
     * 用例名称：定时检验资源的证书和吊销列表。
     * 前置条件：无。
     * check点：不报错且正常时不发送告警。
     */
    @Test
    public void execute_success() {
        resourceCertSchedule.setLockService(lockService);
        Lock lock = PowerMockito.mock(Lock.class);
        PowerMockito.mockStatic(ResourceCertUtil.class);
        PowerMockito.when(ResourceCertUtil.checkCertificateIsValid(any(), any())).thenReturn(false);
        PowerMockito.when(ResourceCertUtil.checkCrlIsValid(any(), any())).thenReturn(false);
        PowerMockito.when(lockService.createDistributeLock(any())).thenReturn(lock);
        PowerMockito.when(lockService.createSQLDistributeLock(any())).thenReturn(lock);
        PowerMockito.when(lock.tryLock(1, TimeUnit.SECONDS)).thenReturn(true);

        DeployTypeService deployTypeService = Mockito.mock(DeployTypeService.class);
        resourceCertSchedule.setDeployTypeService(deployTypeService);
        Mockito.when(deployTypeService.isCyberEngine()).thenReturn(false);

        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        ProtectedResource protectedResource = new ProtectedResource();
        final HashMap<String, String> map = new HashMap<>();
        map.put(Constants.CERT_KEY, "111");
        map.put(Constants.REVOCATION_LIST_VMWARE, "222");
        protectedResource.setExtendInfo(map);
        response.setRecords(Arrays.asList(protectedResource));
        response.setTotalCount(1);
        PowerMockito.when(resourceService.query(any())).thenReturn(response);

        VmwareResourceCertCheckProvider vmwareResourceCertCheckProvider = new VmwareResourceCertCheckProvider();
        PowerMockito.when(providerManager.findProvider(any(), any(), any())).thenReturn(vmwareResourceCertCheckProvider);
        resourceCertSchedule.execute();
        Assert.assertTrue(true);
    }
}
