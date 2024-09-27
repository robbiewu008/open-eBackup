package openbackup.data.access.client.sdk.api.storage.model;

import lombok.Data;

import java.util.List;

/**
 * 功能描述
 *
 * @author y00413474
 * @since 2020-07-18
 */
@Data
public class StorageInfo {
    private String ip;

    private Integer port;

    private String username;

    private String password;

    private Integer type;

    /**
     * 管理ip，多个ip以，拼接
     */
    private String managementIps;

    private List<StoragePool> storagePoolList;
}
