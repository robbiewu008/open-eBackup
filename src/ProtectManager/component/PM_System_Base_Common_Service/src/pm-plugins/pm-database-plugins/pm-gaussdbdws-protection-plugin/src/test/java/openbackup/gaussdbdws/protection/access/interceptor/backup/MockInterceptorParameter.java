package openbackup.gaussdbdws.protection.access.interceptor.backup;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.DataProtectionParams;
import openbackup.system.base.sdk.cluster.model.SourceClustersParams;
import openbackup.system.base.sdk.cluster.model.StorageSystemInfo;
import openbackup.system.base.sdk.repository.model.BackupClusterVo;
import openbackup.system.base.sdk.repository.model.BackupUnitVo;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * DWS 下发参数工具类
 *
 * @author swx1010572
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-05
 */
public class MockInterceptorParameter {
    public static BackupTask getBackupTask() {
        BackupTask backupTask = new BackupTask();
        List<StorageRepository> repositories = new ArrayList<>();
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setEndpoint(new Endpoint());
        repositories.add(storageRepository);
        backupTask.setRepositories(repositories);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid("aaaaaaaaaaaaaaaaa");
        taskEnvironment.setRootUuid("aaaaaaaaaaaaaaaaa");
        List<TaskEnvironment> nodes = new ArrayList<>();
        TaskEnvironment clusterTaskEnvironment = new TaskEnvironment();
        clusterTaskEnvironment.setUuid("bbbbbbbbbbbbb");
        TaskEnvironment hostTaskEnvironment = new TaskEnvironment();
        hostTaskEnvironment.setUuid("ccccccccccc");
        nodes.add(clusterTaskEnvironment);
        nodes.add(hostTaskEnvironment);
        taskEnvironment.setNodes(nodes);
        taskEnvironment.setExtendInfo(new HashMap<>());
        backupTask.setProtectEnv(taskEnvironment);
        backupTask.setProtectObject(new TaskResource());
        backupTask.getProtectObject().setRootUuid("cccccccc");
        backupTask.getProtectObject().setUuid("aaaaaaaaaaaaaaaaa");
        return backupTask;
    }

    public static ProtectedEnvironment getProtectedEnvironment() {
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid("aaaaaaaaaaaaaaaaa");
        resource.setLinkStatus("1");
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DwsConstant.DWS_CLUSTER_AGENT, getProtectedResources("bbbbbbbbbbbbb"));
        dependencies.put(DwsConstant.HOST_AGENT, getProtectedResources("ccccccccccc"));
        resource.setDependencies(dependencies);
        Authentication authentication = new Authentication();
        authentication.setAuthKey("omm");
        resource.setAuth(authentication);
        resource.setExtendInfo(new HashMap<>());
        resource.getExtendInfo().put(DwsConstant.EXTEND_INFO_KEY_ENV_FILE, "/opt/install");
        return resource;
    }

    private static List<ProtectedResource> getProtectedResources(String uuid) {
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(uuid);
        list.add(protectedResource);
        return list;
    }


    public static Map<ProtectedResource, List<ProtectedEnvironment>> getProtectedResourceListMap() {
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap = new HashMap<>();
        List<ProtectedEnvironment> protectedEnvironments = new ArrayList<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("hostAgent");
        protectedEnvironment.setPort(8088);
        protectedEnvironment.setEndpoint("192.168.10.10");
        protectedEnvironments.add(protectedEnvironment);
        protectedResourceMap.put(new ProtectedResource(), protectedEnvironments);
        return protectedResourceMap;
    }


    public static NasDistributionStorageDetail getNasDistributionStorageDetail() {
        NasDistributionStorageDetail nasDistributionStorageDetail = new NasDistributionStorageDetail();
        nasDistributionStorageDetail.setUnitList(new ArrayList<>());
        BackupUnitVo backupUnitVo = new BackupUnitVo();
        BackupClusterVo backupClusterVo = new BackupClusterVo();
        backupClusterVo.setIp("192.168.100.100");
        backupClusterVo.setPort(9527);
        backupClusterVo.setClusterId(10000);
        backupUnitVo.setBackupClusterVo(backupClusterVo);
        nasDistributionStorageDetail.getUnitList().add(backupUnitVo);
        return nasDistributionStorageDetail;
    }

    public static List<ClusterDetailInfo> getClusterDetailInfo() {
        List<ClusterDetailInfo> clusterDetailInfos = new ArrayList<>();
        ClusterDetailInfo clusterDetailInfo = new ClusterDetailInfo();
        StorageSystemInfo storageSystemInfo = Optional.ofNullable(clusterDetailInfo.getStorageSystem())
            .orElse(new StorageSystemInfo());
        SourceClustersParams sourceClustersParams = Optional.ofNullable(clusterDetailInfo.getSourceClusters())
            .orElse(new SourceClustersParams());
        List<DataProtectionParams> dataProtectionParams = Optional.ofNullable(
            clusterDetailInfo.getDataProtectionEngines()).orElse(new ArrayList<>());
        sourceClustersParams.setStorageDisplayIps("192.168.100.102");
        storageSystemInfo.setStorageEsn("2102354DEY10M3000002");
        storageSystemInfo.setPassword("AAAAAgAAAAAAAAAAAAAAAQAAAAk8jDbAZpwA6b28kggrrvvDVewywTfg+I5MpjL9AAAAAAAAAAAAAAAAAAAAGU/Nb0x/27hS75VsTEIfdqTfxZbrbTyGNms=");
        storageSystemInfo.setStoragePort(8088);
        storageSystemInfo.setUsername("admin");
        clusterDetailInfo.setStorageSystem(storageSystemInfo);
        clusterDetailInfo.setSourceClusters(sourceClustersParams);
        clusterDetailInfo.setDataProtectionEngines(dataProtectionParams);
        clusterDetailInfos.add(clusterDetailInfo);
        return clusterDetailInfos;
    }
}
