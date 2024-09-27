package openbackup.oracle.interceptor;

import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.oracle.constants.OracleConstants;
import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Test;
import org.mockito.InjectMocks;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

import static org.mockito.ArgumentMatchers.any;

public class OracleClusterRestoreProviderTest {

    private final OracleBaseService oracleBaseService = Mockito.mock(OracleBaseService.class);

    @InjectMocks
    private OracleClusterRestoreProvider oracleClusterRestoreProvider = new OracleClusterRestoreProvider(oracleBaseService);


    @Test
    public void supply_restore_task_success() {
        RestoreTask task = mockTask(ResourceSubTypeEnum.ORACLE_CLUSTER);
        ProtectedResource resource = mockGetSingleRes(task);
        ProtectedEnvironment cluster = mockEnvironment();

        Mockito.when(oracleBaseService.getEnvironmentById(any())).thenReturn(cluster);
        Mockito.when(oracleBaseService.getResource(any())).thenReturn(resource);
        oracleClusterRestoreProvider.supplyCluster(task,new Copy());
        Mockito.verify(oracleBaseService, Mockito.times(1)).getEnvironmentById(any());
    }

    private RestoreTask mockTask(ResourceSubTypeEnum subTypeEnum) {
        RestoreTask task = new RestoreTask();
        task.setTargetLocation(RestoreLocationEnum.NEW);
        task.setAdvanceParams(new HashMap<>());
        List<StorageRepository> repositories = new ArrayList<>();
        StorageRepository storageRepository = new StorageRepository();
        repositories.add(storageRepository);
        task.setRepositories(repositories);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setNodes(new ArrayList<>());
        Map<String, String> taskEnvironmentExtendInfo = new HashMap<>();
        taskEnvironment.setExtendInfo(taskEnvironmentExtendInfo);
        task.setTargetEnv(taskEnvironment);
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid(UUID.randomUUID().toString());
        taskResource.setParentUuid(UUID.randomUUID().toString());
        taskResource.setSubType(subTypeEnum.getType());
        final Map<String, String> extendInfo = new HashMap<>();
        taskResource.setExtendInfo(extendInfo);
        task.setTargetObject(taskResource);
        task.setAgents(new ArrayList<>());
        return task;
    }

    private ProtectedEnvironment mockEnvironment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();

        List<ProtectedResource> agents = new ArrayList<>();
        ProtectedEnvironment agentA = new ProtectedEnvironment();
        agentA.setEndpoint("192.168.111.180");
        agentA.setPort(10010);
        ProtectedEnvironment agentB = new ProtectedEnvironment();
        agentB.setEndpoint("192.168.111.181");
        agentB.setPort(10010);
        agents.add(agentA);
        agents.add(agentB);

        environment.setDependencies(new HashMap<>());
        environment.getDependencies().put(DatabaseConstants.AGENTS, agents);

        return environment;
    }
    private ProtectedResource mockGetSingleRes(RestoreTask task) {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(UUID.randomUUID().toString());
        protectedResource.setVersion("111");
        HashMap<String, String> extendInfo = new HashMap<>();
        extendInfo.put(OracleConstants.INSTANCES, "1234");
        protectedResource.setExtendInfo(extendInfo);
        PowerMockito.when(oracleBaseService.getSingleInstanceByClusterInstance(task.getTargetObject().getUuid()))
                .thenReturn(Collections.singletonList(protectedResource));
        return protectedResource;
    }
}