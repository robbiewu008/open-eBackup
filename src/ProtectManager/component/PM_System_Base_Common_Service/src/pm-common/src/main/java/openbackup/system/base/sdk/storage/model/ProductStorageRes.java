package openbackup.system.base.sdk.storage.model;

import openbackup.system.base.sdk.storage.enums.StorageAuthStatus;

import lombok.Data;

/**
 * 存储信息
 *
 * @author p00511147
 * @since 2020-11-10
 */
@Data
public class ProductStorageRes {
    private String id;

    private String ip;

    private int port;

    private StorageAuthStatus status;

    private String esn;

    private String wwn;

    private String deviceName;

    private String type;

    private String createTime;

    private String userName;
}
