package openbackup.gaussdbt.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.enums.MountTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * GaussDBT副本删除Provider测试类
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-01
 */
public class GaussDBTCopyDeleteInterceptorProviderTest {
    private final ProtectedEnvironmentService environmentService = PowerMockito.mock(ProtectedEnvironmentService.class);
    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);
    private final GaussDBTCopyDeleteInterceptorProvider provider = new GaussDBTCopyDeleteInterceptorProvider(
        environmentService, copyRestApi, resourceService);

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入GaussDBT宏
     * 检查点：是否返回true
     */
    @Test
    public void applicable_restore_interceptor_provider_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.GAUSSDBT.getType()));
    }

    /**
     * 用例场景：GaussDBT副本删除需要添加protectEnv和非全路径挂载参数
     * 前置条件：副本资源存在，副本数据存在
     * 检查点：是否添加成功
     */
    @Test
    public void handle_copy_delete_task_success() {
        DeleteCopyTask task = new DeleteCopyTask();
        CopyInfoBo copyInfoBo = new CopyInfoBo();
        ProtectedEnvironment environment = new ProtectedEnvironment();
        String nodeString = "[{\"uuid\":\"123456\"}]";
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(GaussDBTConstant.NODES_KEY, nodeString);
        environment.setExtendInfo(extendInfo);
        PowerMockito.when(environmentService.getEnvironmentById(any())).thenReturn(environment);
        provider.handleTask(task, copyInfoBo);
        Map<String, String> advanceParams = task.getAdvanceParams();
        Assert.assertEquals(MountTypeEnum.FULL_PATH_MOUNT.getMountType(),
            advanceParams.get(GaussDBTConstant.MOUNT_TYPE_KEY));
        List<TaskEnvironment> nodes = task.getProtectEnv().getNodes();
        Assert.assertEquals("123456", nodes.get(0).getUuid());
    }

    /**
     * 用例场景：GaussDBT副本删除handTask,资源不存在的时候
     * 前置条件：副本资源存在，副本数据存在
     * 检查点：是否返回成功
     */
    @Test
    public void handle_copy_delete_task_success_when_resource_is_not_exist() {
        DeleteCopyTask task = new DeleteCopyTask();
        CopyInfoBo copyInfoBo = new CopyInfoBo();
        PowerMockito.doThrow(new LegoCheckedException("")).when(environmentService).getEnvironmentById(any());
        provider.handleTask(task, copyInfoBo);
        Map<String, String> advanceParams = task.getAdvanceParams();
        Assert.assertEquals(MountTypeEnum.FULL_PATH_MOUNT.getMountType(),
            advanceParams.get(GaussDBTConstant.MOUNT_TYPE_KEY));
    }

    /**
     * 用例场景：GaussDBT副本级联删除(删除差异副本时返回依赖副本）
     * 前置条件：副本资源存在，副本数据存在
     * 检查点：是否添加成功需要级联删除的副本
     */
    @Test
    public void get_copies_copy_type_is_cumulative_increment_success()  {
        List<Copy> copies = generateBackupCopies();
        Copy thisCopy = copies.stream().filter(copy -> copy.getGn() == 7).findFirst().get();
        Copy nextFullCopy = copies.stream().filter(copy -> copy.getGn() == 10).findFirst().get();
        List<String> processedCopies = provider.getCopiesCopyTypeIsCumulativeIncrement(copies, thisCopy, nextFullCopy);
        Assert.assertEquals(2, processedCopies.size());
    }

    /**
     * 用例场景：GaussDBT副本级联删除(删除增量副本时返回依赖副本）
     * 前置条件：副本资源存在，副本数据存在
     * 检查点：是否添加成功需要级联删除的副本
     */
    @Test
    public void get_copies_copy_type_is_difference_increment_success()  {
        List<Copy> copies = generateBackupCopies();
        Copy thisCopy = copies.stream().filter(copy -> copy.getGn() == 2).findFirst().get();
        Copy nextFullCopy = copies.stream().filter(copy -> copy.getGn() == 6).findFirst().get();
        List<String> processedCopies = provider.getCopiesCopyTypeIsDifferenceIncrement(copies, thisCopy, nextFullCopy);
        Assert.assertEquals(1, processedCopies.size());
    }

    private List<Copy> generateBackupCopies() {
        // 全增日增日全差差日全
        Copy fullCopy1 = generateResourceCopy();
        fullCopy1.setGn(1);
        fullCopy1.setUuid("full_01");
        fullCopy1.setBackupType(BackupTypeConstants.FULL.getAbBackupType());
        fullCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy differenceCopy1 = generateResourceCopy();
        differenceCopy1.setGn(2);
        differenceCopy1.setUuid("difference_01");
        differenceCopy1.setBackupType(BackupTypeConstants.DIFFERENCE_INCREMENT.getAbBackupType());
        differenceCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy logCopy1 = generateResourceCopy();
        logCopy1.setGn(3);
        logCopy1.setUuid("log_01");
        logCopy1.setBackupType(BackupTypeConstants.LOG.getAbBackupType());
        logCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy differenceCopy2 = BeanTools.copy(differenceCopy1, Copy::new);
        differenceCopy2.setGn(4);
        differenceCopy2.setUuid("difference_02");
        Copy logCopy2 = BeanTools.copy(logCopy1, Copy::new);
        logCopy2.setGn(5);
        logCopy2.setUuid("log_02");
        Copy fullCopy2 = BeanTools.copy(fullCopy1, Copy::new);
        fullCopy2.setGn(6);
        fullCopy2.setUuid("full_02");
        Copy cumulativeCopy1 = generateResourceCopy();
        cumulativeCopy1.setGn(7);
        cumulativeCopy1.setUuid("cumulative_01");
        cumulativeCopy1.setBackupType(BackupTypeConstants.CUMULATIVE_INCREMENT.getAbBackupType());
        cumulativeCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy cumulativeCopy2 = BeanTools.copy(cumulativeCopy1, Copy::new);
        cumulativeCopy2.setGn(8);
        cumulativeCopy2.setUuid("cumulative_02");
        Copy logCopy3 = BeanTools.copy(logCopy1, Copy::new);
        logCopy3.setGn(9);
        logCopy3.setUuid("log_03");
        Copy fullCopy3 = BeanTools.copy(fullCopy1, Copy::new);
        fullCopy3.setGn(10);
        fullCopy3.setUuid("full_03");
        return Arrays.asList(fullCopy1, fullCopy2, fullCopy3, differenceCopy1, differenceCopy2, logCopy1,
            cumulativeCopy1, cumulativeCopy2, logCopy2, logCopy3);
    }

    private Copy generateResourceCopy() {
        Copy copy = new Copy();
        copy.setResourceId("this_resource_id");
        return copy;
    }
}
