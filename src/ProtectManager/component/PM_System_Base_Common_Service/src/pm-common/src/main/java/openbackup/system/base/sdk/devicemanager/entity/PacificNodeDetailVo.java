package openbackup.system.base.sdk.devicemanager.entity;

import lombok.Getter;
import lombok.Setter;

/**
 * pacific节点详情
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-02-01
 */
@Getter
@Setter
public class PacificNodeDetailVo {
    // 基本信息
    private PacificNodeInfoVo pacificNodeInfoVo;

    // 已配置的业务网络信息
    private PacificNodeNetworkInfo usedNetworkInfo;

    // 所有业务网络信息
    private PacificNodeNetworkInfo allNetworkInfo;
}
