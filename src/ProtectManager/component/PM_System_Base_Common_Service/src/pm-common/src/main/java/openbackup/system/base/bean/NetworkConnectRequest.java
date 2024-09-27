package openbackup.system.base.bean;

import openbackup.system.base.common.enums.NetworkTypeEnum;

import lombok.Getter;
import lombok.Setter;

/**
 * 检查网络连通性请求
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/6/3
 */
@Getter
@Setter
public class NetworkConnectRequest {
    private String ip;
    private int port;
    private NetworkTypeEnum networkType;
    private String netPlaneType;
    private String nodeId;
}
