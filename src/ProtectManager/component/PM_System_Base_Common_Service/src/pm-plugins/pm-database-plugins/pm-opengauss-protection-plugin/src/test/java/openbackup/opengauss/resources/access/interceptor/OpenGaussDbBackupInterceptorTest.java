package openbackup.opengauss.resources.access.interceptor;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;

import openbackup.opengauss.resources.access.provider.OpenGaussAgentProvider;
import openbackup.opengauss.resources.access.provider.OpenGaussMockData;
import openbackup.opengauss.resources.access.service.OpenGaussAgentService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;

public class OpenGaussDbBackupInterceptorTest {
    private final OpenGaussAgentService openGaussAgentService = PowerMockito.mock(OpenGaussAgentService.class);
    private final OpenGaussAgentProvider agentProvider = PowerMockito.mock(OpenGaussAgentProvider.class);
    private final OpenGaussDbBackupInterceptor provider = new OpenGaussDbBackupInterceptor(openGaussAgentService,
        agentProvider);

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入资源类型
     * 检查点：是否返回true
     */
    @Test
    public void applicable_open_gauss_backup_interceptor_provider_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.OPENGAUSS_INSTANCE.getType()));
        Assert.assertFalse(provider.applicable(ResourceSubTypeEnum.GAUSSDBT.getType()));
    }

    /**
     * 用例场景：备份环境设置更新存储仓
     * 前置条件：保护对象为OpenGauss实例
     * 检查点：1、是否设置data仓和cache仓；检查Repositories的类型
     *       2、检查备份速率是否设置为true
     */
    @Test
    public void should_return_repository_is_two_if_open_gauss_database() {
        BackupTask backupTask = OpenGaussMockData.mockBackupTask(ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType());
        BackupTask returnTask = provider.supplyBackupTask(backupTask);
        Assert.assertEquals(returnTask.getRepositories().size(), 2);
        List<Integer> repositories = returnTask.getRepositories()
            .stream()
            .map(StorageRepository::getType)
            .collect(Collectors.toList());
        Assert.assertEquals(repositories, Arrays.asList(1, 2));
        Assert.assertEquals(SpeedStatisticsEnum.UBC.getType(),
            backupTask.getAdvanceParams().get(TaskUtil.SPEED_STATISTICS));
    }

    /**
     * 用例场景：备份任务副本格式为1，为非原生
     * 前置条件：保护对象为OpenGauss实例
     * 检查点：备份任务副本格式为1，为非原生
     */
    @Test
    public void should_return_non_native_copy_format_when_supply_backup_task_if_open_gauss_database() {
        BackupTask backupTask = OpenGaussMockData.mockBackupTask(ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType());
        BackupTask returnTask = provider.supplyBackupTask(backupTask);
        Assert.assertEquals(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat(), returnTask.getCopyFormat());
    }

    /**
     * 用例场景：备份环境查询环境中的agent信息
     * 前置条件：保护对象为OpenGauss实例
     * 检查点：查询环境资源相关的agent信息
     */
    @Test
    public void should_return_agent_endpoint_when_supply_agent_if_open_gauss_database() {
        BackupTask backupTask = OpenGaussMockData.mockBackupTask(ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType());
        Endpoint endpoint = new Endpoint();
        endpoint.setIp("8.3.3.6");
        endpoint.setPort(8963);
        PowerMockito.when(openGaussAgentService.getAgentEndpoint(anyString()))
            .thenReturn(Collections.singletonList(endpoint));
        provider.supplyAgent(backupTask);
        Assert.assertEquals(1, backupTask.getAgents().size());
        Assert.assertEquals(8963, backupTask.getAgents().get(0).getPort());
    }
}