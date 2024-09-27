package openbackup.system.base.sdk.devicemanager.entity;

import lombok.Getter;
import lombok.Setter;

/**
 * pacific节点信息 前端存储网络和管理网络信息
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-02-04
 */
@Getter
@Setter
public class PacificHostInfo {
    // 节点前端存储网络
    private String storageIp;

    // 控制ip 配置了vlan端口时，此ip可用于关联zone中的节点ip
    private String ctrlIp;

    // 管理ip
    private String manageIp;
}