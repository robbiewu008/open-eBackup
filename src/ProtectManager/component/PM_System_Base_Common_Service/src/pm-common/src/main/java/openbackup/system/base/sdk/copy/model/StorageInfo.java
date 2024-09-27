package openbackup.system.base.sdk.copy.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 存储设备信息
 *
 * @author c30058517
 * @since 2024-08-12
 */
@Data
public class StorageInfo {
    /**
     * 存储池
     */
    @JsonProperty("storage_pool")
    private String storagePool;

    /**
     * 设备Id
     */
    @JsonProperty("device_id")
    private String deviceId;

    /**
     * 设备类型
     */
    @JsonProperty("device_type")
    private String deviceType;
}