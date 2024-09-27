package openbackup.system.base.sdk.repository.api;

import openbackup.system.base.sdk.repository.model.BackupClusterVo;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;

/**
 * 备份存储api
 *
 * @author nwx1077006
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-10-10
 */
public interface BackupStorageApi {
    /**
     * 获得存储库详情
     *
     * @param id 存储库id
     * @return 存储库详情
     */
    NasDistributionStorageDetail getDetail(String id);

    /**
     * 查询存储容量
     *
     * @param id 存储库id
     * @param clusterId 分布式集群id
     * @return 集群容量信息
     */
    BackupClusterVo getClusterStorage(String id, int clusterId);
}
