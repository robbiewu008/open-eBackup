package openbackup.system.base.sdk.storage.model;

import lombok.Data;

/**
 * dorado设备信息
 *
 * @author p00511147
 * @since 2020-11-10
 */
@Data
public class ProductStorageInfo {
    private String id;

    private String ip;

    private int port;

    private int type;

    private String userName;

    private String password;

    private String lunId;

    private String deviceId;
}
