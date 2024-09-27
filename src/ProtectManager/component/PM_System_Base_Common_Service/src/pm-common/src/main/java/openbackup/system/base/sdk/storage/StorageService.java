package openbackup.system.base.sdk.storage;

import openbackup.system.base.common.model.repository.StoragePool;
import openbackup.system.base.common.model.storage.StorageResponse;
import openbackup.system.base.common.model.storage.StorageSession;

import java.util.List;

/**
 * 存储服务
 *
 * @author y00413474
 * @author w00493811
 * @since 2020-07-02
 */
public interface StorageService {
    /**
     * 查询存储信息
     *
     * @return 存储信息
     */
    StorageResponse<StorageSession> getStorageSession();

    /**
     * 查询存储池
     *
     * @return 存储池
     */
    StorageResponse<List<StoragePool>> getStoragePools();

    /**
     * 设置存储池告警阈值
     *
     * @param storagePoolId 存储池编号
     * @param userConsumedCapacityThreshold 容量告警阈值(%)
     * @return 操作结果
     */
    StorageResponse<Object> setStoragePoolAlarmThreshold(String storagePoolId, int userConsumedCapacityThreshold);
}
