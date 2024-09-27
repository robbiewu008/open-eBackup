package openbackup.system.base.sdk.accesspoint.model;

import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.SourceClustersParams;
import openbackup.system.base.sdk.cluster.model.StorageSystemInfo;

import lombok.Data;

import java.util.List;

/**
 * Dme Local Device
 *
 * @author l00272247
 * @since 2020-12-16
 */
@Data
public class DmeLocalDevice {
    private int port;

    private String userName;

    private String password;

    private String cert;

    private List<String> mgrIp;

    private String esn;

    private String localStorageType;

    /**
     * build dme local device by ClusterInternalApi
     *
     * @param clusterInternalApi clusterInternalApi
     * @return DmeLocalDevice
     */
    public static DmeLocalDevice build(ClusterInternalApi clusterInternalApi) {
        ClusterDetailInfo clusterDetail = clusterInternalApi.queryClusterDetails();
        DmeLocalDevice localDevice = new DmeLocalDevice();
        SourceClustersParams sourceClustersParams = clusterDetail.getSourceClusters();
        localDevice.setMgrIp(sourceClustersParams.getMgrIpList());
        StorageSystemInfo storage = clusterDetail.getStorageSystem();
        localDevice.setUserName(storage.getUsername());
        localDevice.setPassword(storage.getPassword());
        localDevice.setPort(storage.getStoragePort());
        localDevice.setEsn(storage.getStorageEsn());
        return localDevice;
    }
}
