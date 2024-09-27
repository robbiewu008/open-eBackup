package openbackup.system.base.common.model.repository;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * 功能描述
 *
 * @author y00413474
 * @since 2020-07-18
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class StorageInfo {
    private String ip;

    private int port;

    private String username;

    private String password;

    private int type;

    /**
     * 管理ip，多个ip以，拼接
     */
    private String managementIps;

    private List<StoragePool> storagePoolList;
}