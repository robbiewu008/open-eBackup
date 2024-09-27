package openbackup.data.access.framework.protection.service.job;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.core.dao.ProtectedObjectMapper;
import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.data.access.framework.protection.service.job.UnifiedBackupJobProvider;
import openbackup.data.protection.access.provider.sdk.job.JobProvider;
import com.huawei.oceanprotect.functionswitch.template.service.FunctionSwitchService;
import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import com.huawei.oceanprotect.sla.sdk.dto.SlaDto;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import com.baomidou.mybatisplus.core.MybatisConfiguration;
import com.baomidou.mybatisplus.core.metadata.TableInfoHelper;

import org.apache.ibatis.builder.MapperBuilderAssistant;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

/**
 * UnifiedBackupJobProviderTest
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-05-08
 */
@RunWith(PowerMockRunner.class)
public class UnifiedBackupJobProviderTest {
    private UnifiedBackupJobProvider unifiedBackupJobProvider;

    @Mock
    private SlaQueryService slaQueryService;

    @Mock
    private FunctionSwitchService functionSwitchService;

    @Mock
    private JobProvider unifiedJobProvider;

    @Mock
    private ProtectedObjectMapper protectedObjectMapper;

    @Before
    public void init() {
        TableInfoHelper.initTableInfo(new MapperBuilderAssistant(new MybatisConfiguration(), ""), ProtectedObjectPo.class);
        unifiedBackupJobProvider = new UnifiedBackupJobProvider();
        Whitebox.setInternalState(unifiedBackupJobProvider, "slaQueryService", slaQueryService);
        Whitebox.setInternalState(unifiedBackupJobProvider, "functionSwitchService", functionSwitchService);
        Whitebox.setInternalState(unifiedBackupJobProvider, "unifiedJobProvider", unifiedJobProvider);
        Whitebox.setInternalState(unifiedBackupJobProvider, "protectedObjectMapper", protectedObjectMapper);
    }

    @Test
    public void test_stop_job_success() {
        unifiedBackupJobProvider.stopJob("123456");
        Mockito.verify(unifiedJobProvider).stopJob(anyString());
    }

    @Test
    public void test_fill_job_info_success() {
        ProtectedObjectPo protectedObjectPo = new ProtectedObjectPo();
        protectedObjectPo.setExtParameters("{}");
        PowerMockito.when(protectedObjectMapper.selectOne(any())).thenReturn(protectedObjectPo);
        Job insertJob = new Job();
        insertJob.setExtendStr("{\"slaId\":\"123456\"}");
        PowerMockito.when(slaQueryService.querySlaById(anyString())).thenReturn(new SlaDto());
        unifiedBackupJobProvider.fillJobInfo(insertJob);
        Assert.assertTrue(insertJob.getExtendStr().contains(JobExtendInfoKeys.TRIGGER_POLICY));
    }

    @Test
    public void test_applicable_success() {
        Assert.assertTrue(unifiedBackupJobProvider.applicable(JobTypeEnum.BACKUP.getValue()));
    }
}