package openbackup.database.base.plugin.provider;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.GeneralDbConstant;
import openbackup.database.base.plugin.util.TestConfHelper;
import com.huawei.oceanprotect.job.sdk.JobService;

import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.boot.autoconfigure.sql.init.SqlInitializationAutoConfiguration;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.List;

/**
 * GeneralDbCopyDeleteInterceptor测试类
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-01-30
 */
@SpringBootTest(classes = {
    GeneralDbCopyDeleteInterceptor.class, GeneralDbProtectAgentService.class, SqlInitializationAutoConfiguration.class
})
@RunWith(SpringRunner.class)
public class GeneralDbCopyDeleteInterceptorTest {
    @Autowired
    private GeneralDbCopyDeleteInterceptor generalDbCopyDeleteInterceptor;

    @MockBean
    private RedissonClient redissonClient;

    @MockBean
    private CopyRestApi copyRestApi;

    @MockBean
    private ProviderManager providerManager;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    @Qualifier("unifiedResourceConnectionChecker")
    private ProtectedResourceChecker protectedResourceChecker;

    @MockBean
    private JobService jobService;

    @MockBean
    private GeneralDbProtectAgentService generalDbProtectAgentService;

    @Before
    public void init() {
        RMap rMap = Mockito.mock(RMap.class);
        Mockito.when(rMap.get("is_associated")).thenReturn("true");
        Mockito.when(redissonClient.getMap(Mockito.any(), Mockito.any(StringCodec.class))).thenReturn(rMap);

        Mockito.when(jobService.queryJob(Mockito.any())).thenReturn(new JobBo());

        Mockito.when(copyRestApi.queryCopyByID("copyUuid",false)).thenReturn(mockCopy());

        List<Copy> copies = new ArrayList<>();
        Copy copy1 = new Copy();
        copy1.setUuid("copy1");
        copy1.setGn(6);
        copy1.setNextCopyGn(7);
        copy1.setBackupType(BackupTypeEnum.DIFFERENCE_INCREMENT.getAbbreviation());
        copies.add(copy1);
        Copy copy2 = new Copy();
        copy2.setUuid("copy2");
        copy2.setGn(7);
        copy2.setNextCopyGn(-1);
        copy2.setBackupType(BackupTypeEnum.DIFFERENCE_INCREMENT.getAbbreviation());
        copies.add(copy2);
        Mockito.when(copyRestApi.queryLaterCopiesByGeneratedBy(Mockito.any(), Mockito.anyInt(), Mockito.any())).thenReturn(copies);
    }

    /**
     * 用例场景：通用数据库副本删除时删除关联的副本
     * 前置条件：存在关联副本
     * 检查点：删除全量副本时，删除关联的增量副本
     */
    @Test
    public void associate_delete_should_success() {
        DeleteCopyTask task = new DeleteCopyTask();
        CopyInfoBo copyInfoBo = new CopyInfoBo();
        Copy copy = mockCopy();
        BeanUtils.copyProperties(copy, copyInfoBo);
        generalDbCopyDeleteInterceptor.initialize(task, copyInfoBo);

        Mockito.verify(copyRestApi, Mockito.times(2))
            .deleteCopy(Mockito.any(), Mockito.any(), Mockito.any(), Mockito.any(), Mockito.any());
    }

    private Copy mockCopy() {
        Copy copy = new Copy();
        copy.setUuid("copyUuid");
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setBackupType(BackupTypeEnum.FULL.getAbbreviation());
        copy.setGn(5);
        copy.setNextCopyGn(6);

        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_CONF, TestConfHelper.getHanaConf());
        copy.setResourceProperties(JsonUtil.json(protectedResource));

        return copy;
    }
}
