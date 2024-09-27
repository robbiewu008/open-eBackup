package openbackup.system.base.common.model.storage;

import lombok.Data;

/**
 * 功能描述
 *
 * @author y00413474
 * @since 2020-07-01
 */
@Data
public class StorageSessionRequest {
    private String username;

    private String password;

    private Integer scope;
}
