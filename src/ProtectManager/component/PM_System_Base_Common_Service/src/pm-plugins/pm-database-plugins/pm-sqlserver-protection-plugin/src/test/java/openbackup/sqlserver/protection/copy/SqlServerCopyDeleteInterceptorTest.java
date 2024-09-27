package openbackup.sqlserver.protection.copy;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Optional;
import java.util.UUID;

/**
 * SQL Server副本删除拦截器测试类
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-04
 */
public class SqlServerCopyDeleteInterceptorTest {
    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private final SqlServerBaseService sqlServerBaseService = PowerMockito.mock(SqlServerBaseService.class);

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final SqlServerCopyDeleteInterceptor copyDeleteInterceptor = new SqlServerCopyDeleteInterceptor(copyRestApi,
        sqlServerBaseService, resourceService);

    /**
     * 用例场景：SQL Server副本删除策略适配
     * 前置条件：类型判断
     * 检查点：是否匹配
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(copyDeleteInterceptor.applicable(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType()));
        Assert.assertTrue(copyDeleteInterceptor.applicable(ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType()));
        Assert.assertTrue(copyDeleteInterceptor.applicable(ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType()));
        Assert.assertTrue(copyDeleteInterceptor.applicable(ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType()));
    }

    @Test
    public void handle_task(){
        CopyInfoBo copyInfoBo = new CopyInfoBo();
        DeleteCopyTask task = new DeleteCopyTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("test");
        task.setProtectObject(taskResource);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setExtendInfo(new HashMap<>());
        task.setProtectEnv(taskEnvironment);
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("test");
        resource.setRootUuid("test");
        PowerMockito.when(resourceService.getBasicResourceById(anyString())).thenReturn(Optional.of(resource));
        copyDeleteInterceptor.handleTask(task, copyInfoBo);
        Assert.assertNotNull(task.getProtectEnv().getExtendInfo());
        Assert.assertNotNull(task.getProtectEnv().getNodes());
    }

    @Test
    public void execute_post_process_success() {
        PowerMockito.when(copyRestApi.queryLatestBackupCopy(any(), any(), any())).thenReturn(mockCopy());
        copyDeleteInterceptor.finalize(mockCopy(), mockTask());
        Assert.assertTrue(true);
    }

    private Copy mockCopy() {
        Copy copy = new Copy();
        copy.setUuid(UUID.randomUUID().toString());
        copy.setResourceId(UUID.randomUUID().toString());
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setBackupType(1);
        copy.setGn(1);
        return copy;
    }

    private TaskCompleteMessageBo mockTask() {
        TaskCompleteMessageBo taskMessage = new TaskCompleteMessageBo();
        taskMessage.setJobStatus(3);
        taskMessage.setJobRequestId(UUID.randomUUID().toString());
        return taskMessage;
    }

    /**
     * 用例场景：SQL Server副本级联删除(删除日志副本时返回依赖的日志副本）
     * 前置条件：副本资源存在，副本数据存在
     * 检查点：是否添加成功需要级联删除的副本
     */
    @Test
    public void get_copies_copy_type_is_log_success() {
        List<Copy> copies = generateBackupCopies();
        Copy thisCopy = copies.stream().filter(copy -> copy.getGn() == 3).findFirst().get();
        Copy nextFullCopy = copies.stream().filter(copy -> copy.getGn() == 7).findFirst().get();
        List<String> processedCopies = copyDeleteInterceptor.getCopiesCopyTypeIsLog(copies, thisCopy, nextFullCopy);
        Assert.assertEquals(2, processedCopies.size());
    }

    /**
     * 用例场景：SQL Server副本级联删除(删除差异副本后日志副本）
     * 前置条件：副本资源存在，副本数据存在
     * 检查点：是否返回空
     */
    @Test
    public void get_copies_copy_type_is_cumulative_increment_success() {
        List<Copy> copies = generateBackupCopies();
        Copy thisCopy = copies.stream().filter(copy -> copy.getGn() == 5).findFirst().get();
        Copy nextFullCopy = copies.stream().filter(copy -> copy.getGn() == 7).findFirst().get();
        List<String> processedCopies = copyDeleteInterceptor.getCopiesCopyTypeIsCumulativeIncrement(copies, thisCopy, nextFullCopy);
        Assert.assertEquals(1, processedCopies.size());
    }

    /**
     * 用例场景：SQL Server副本删除的时候下发agents
     * 前置条件：副本资源存在，副本数据存在，副本资源是可保护的资源对象
     * 检查点：是否成功回填agents
     */
    @Test
    public void supply_agents_success() {
        PowerMockito.when(sqlServerBaseService.convertNodeListToAgents(anyString())).thenReturn(new ArrayList<>());
        DeleteCopyTask task = new DeleteCopyTask();
        copyDeleteInterceptor.supplyAgent(task, generateCopyInfoBo());
        Assert.assertNotNull(task.getAgents());
    }

    private List<Copy> generateBackupCopies() {
        // 副本顺序：全差日日差日全日
        Copy fullCopy1 = generateResourceCopy();
        fullCopy1.setGn(1);
        fullCopy1.setUuid("full_01");
        fullCopy1.setBackupType(BackupTypeConstants.FULL.getAbBackupType());
        fullCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy cumulativeCopy1 = generateResourceCopy();
        cumulativeCopy1.setGn(2);
        cumulativeCopy1.setUuid("cumulative_01");
        cumulativeCopy1.setBackupType(BackupTypeConstants.CUMULATIVE_INCREMENT.getAbBackupType());
        cumulativeCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy logCopy1 = BeanTools.copy(cumulativeCopy1, Copy::new);
        logCopy1.setGn(3);
        logCopy1.setUuid("log_01");
        logCopy1.setBackupType(BackupTypeConstants.LOG.getAbBackupType());
        logCopy1.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        Copy logCopy2 = BeanTools.copy(logCopy1, Copy::new);
        logCopy2.setGn(4);
        logCopy2.setUuid("log_02");
        Copy cumulativeCopy2 = BeanTools.copy(cumulativeCopy1, Copy::new);
        cumulativeCopy2.setGn(5);
        cumulativeCopy2.setUuid("cumulative_02");
        Copy logCopy3 = BeanTools.copy(logCopy1, Copy::new);
        logCopy3.setGn(6);
        logCopy3.setUuid("log_03");
        Copy fullCopy3 = BeanTools.copy(fullCopy1, Copy::new);
        fullCopy3.setGn(7);
        fullCopy3.setUuid("full_03");
        Copy logCopy4 = BeanTools.copy(logCopy1, Copy::new);
        logCopy4.setGn(8);
        logCopy4.setUuid("log_04");
        return Arrays.asList(fullCopy1, cumulativeCopy2, fullCopy3, cumulativeCopy1, logCopy1, logCopy2, logCopy3,
            logCopy4);
    }

    private Copy generateResourceCopy() {
        Copy copy = new Copy();
        copy.setBackupType(BackupTypeConstants.FULL.getAbBackupType());
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setResourceId("this_resource_id");
        return copy;
    }

    private CopyInfoBo generateCopyInfoBo() {
        CopyInfoBo copyInfoBo = new CopyInfoBo();
        copyInfoBo.setUuid("this_copy_id");
        copyInfoBo.setResourceId("this_resource_id");
        return copyInfoBo;
    }
}