package openbackup.oracle.interceptor;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.oracle.constants.OracleConstants;
import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import static org.mockito.ArgumentMatchers.any;

/**
 * oracle数据库恢复拦截器实现类 测试类
 *
 * @version [OceanProtect DataBackup 1.3.0]
 * @author c30038333
 * @since 2023-02-14
 */
public class OracleSingleRestoreProviderTest {
    private final OracleBaseService oracleBaseService = Mockito.mock(OracleBaseService.class);
    private static final OracleSingleRestoreProvider oracleSingleRestoreProvider = new OracleSingleRestoreProvider();

    /**
     * 用例场景：supplyRestoreTask
     * 前置条件：无
     * 检查点：
     */
    @Test
    public void supply_restore_task_success() {
        RestoreTask restoreTask = mockRestoreTask();
        Mockito.when(oracleBaseService.getResource(any())).thenReturn(mockResource());
        Mockito.when(oracleBaseService.getEnvironmentById(any())).thenReturn(mockProEnvironment());
        Mockito.when(oracleBaseService.getOsType(any())).thenReturn("windows");
        restoreTask.setTargetEnv(mockEnvironment());
        oracleSingleRestoreProvider.setOracleBaseService(oracleBaseService);
        oracleSingleRestoreProvider.supplySingle(restoreTask);
        Assert.assertEquals("windows", restoreTask.getAgents().get(0).getAgentOS());
    }

    private RestoreTask mockRestoreTask(){
        RestoreTask task = new RestoreTask();
        task.setTaskId("123");
        task.setAdvanceParams(new HashMap<>());

        TaskResource targetObject = new TaskResource();
        targetObject.setUuid("d79889d96c2040129ab99e8f02c807b6");
        task.setTargetObject(targetObject);
        targetObject.setSubType(ResourceSubTypeEnum.ORACLE.getType());

        task.setTargetLocation(RestoreLocationEnum.NEW);

        task.setAdvanceParams(new HashMap<>());

        List<StorageRepository> repositories = new ArrayList<>();
        repositories.add(new StorageRepository());
        task.setRepositories(repositories);
        return task;
    }

    private ProtectedResource mockResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setAuth(new Authentication());
        resource.setExtendInfo(new HashMap<>());
        return resource;
    }

    private TaskEnvironment mockEnvironment() {
        TaskEnvironment environment = new TaskEnvironment();
        environment.setExtendInfo(new HashMap<>());
        environment.setUuid("12345");
        environment.setEndpoint("192.168.111.102");
        environment.setPort(10010);
        return environment;
    }

    private ProtectedEnvironment mockProEnvironment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setExtendInfo(new HashMap<>());
        environment.setUuid("12345");
        environment.setEndpoint("192.168.111.102");
        environment.setPort(10010);
        environment.setOsType(OracleConstants.WINDOWS);
        return environment;
    }
}

