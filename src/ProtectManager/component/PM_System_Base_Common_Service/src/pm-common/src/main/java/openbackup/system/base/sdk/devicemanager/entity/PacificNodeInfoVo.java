package openbackup.system.base.sdk.devicemanager.entity;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * pacific节点基本信息
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-02-01
 */
@Getter
@Setter
public class PacificNodeInfoVo {
    // 节点名称
    private String name;

    // 节点管理ip
    private String manageIp;

    // 节点状态
    private Integer status;

    // 节点角色
    private List<String> role;
}
