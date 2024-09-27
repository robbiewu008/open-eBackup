package openbackup.system.base.sdk.devicemanager.entity;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

import javax.annotation.Nullable;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Size;

/**
 * pacific节点业务网络信息
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-02-01
 */
@Getter
@Setter
public class PacificNodeNetworkInfo {
    // 备份业务网络信息
    @NotNull
    @Size(min = 1)
    private List<PacificIpInfo> backupIpInfoList;

    @NotNull
    // 归档业务网络信息
    private List<PacificIpInfo> archiveIpInfoList;

    @Nullable
    private List<PacificIpInfo> replicationIpInfoList;
}
